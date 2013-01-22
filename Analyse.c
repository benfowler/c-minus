/*********************************************************************
 *  NAME:          Benjamin Fowler
 *  NUMBER:        02251132
 *  SUBJECT:       Modern Compiler Construction
 *  INSTRUCTOR:    Dr Wayne Kelly
 *********************************************************************
 *  MODULE:        Analyse.c
 *  PURPOSE:       Defines the implementation of the semantic
 *                  analyser module.
 *  DATE STARTED:  7th May, 2000.
 *  LAST EDITED:   11th May, 2000.
 *********************************************************************/

#include "Analyse.h"
#include "Globals.h"
#include "SymTab.h"
#include "Util.h"


/*********************************************************************
 *  Module-static function declarations
 */

/* draw a ruler on the screen */
static void drawRuler(FILE *output, char *string);

/* the guts of buildSymbolTable() */
static void buildSymbolTable2(TreeNode *syntaxTree);

/* flag an error from the type checker */
static void flagSemanticError(char *str);

/* generic tree traversal routine */
static void traverse(TreeNode *syntaxTree,
		     void (*preProc)(TreeNode *),
		     void (*postProc)(TreeNode *) );

/* routine to perform the actual type check on a node */
static void checkNode(TreeNode *syntaxTree);

/* dummy do-nothing procedure used to keep traversal() happy */
static void nullProc(TreeNode *syntaxTree);

/* traverse the syntax tree, marking global variables as such */
void markGlobals(TreeNode *tree);

/* declare the C-minus "built-in" input() and output() routines */
static void declarePredefines(void);

/* type-check functions' formal parameters against actual parameters */
static int checkFormalAgainstActualParms(TreeNode *formal, TreeNode *actual);


/*********************************************************************
 *  Public function definitions
 */

void buildSymbolTable(TreeNode *syntaxTree)
{
    /* Format headings */
    if (TraceAnalyse)
    {
	drawRuler(listing, "");
	fprintf(listing,
		"Scope Identifier        Line   Is a   Symbol type\n");
	fprintf(listing,
		"depth                   Decl.  parm?\n");
    }

    declarePredefines();   /* make input() and output() visible in globals */
    buildSymbolTable2(syntaxTree);

    /* set the isGlobal field in appropriate nodes: needed in code generator */
    markGlobals(syntaxTree);
    
    /* Dump the global scope, if it's asked for */
    if (TraceAnalyse)
    {
	drawRuler(listing, "GLOBALS");
	dumpCurrentScope();
	drawRuler(listing, "");
	fprintf(listing, "*** Symbol table dump complete\n");
    }
}


void typeCheck(TreeNode *syntaxTree)
{
    traverse(syntaxTree, nullProc, checkNode);
}


/*********************************************************************
 *  Static function declarations
 */

  
static void declarePredefines(void)
{
    TreeNode *input;
    TreeNode *output;
    TreeNode *temp;

    /* define "int input(void)" */
    input = newDecNode(FuncDecK);
    input->name = copyString("input");
    input->functionReturnType = Integer;
    input->expressionType = Function;
    
    /* define "void output(int)" */
    temp = newDecNode(ScalarDecK); 
    temp->name = copyString("arg"); 
    temp->variableDataType = Integer;
    temp->expressionType = Integer; 
    
    output = newDecNode(FuncDecK); 
    output->name = copyString("output"); 
    output->functionReturnType = Void; 
    output->expressionType = Function; 
    output->child[0] = temp;

    /* get input() and output() added to global scope */
    insertSymbol("input", input, 0);
    insertSymbol("output", output, 0);
}


static void buildSymbolTable2(TreeNode *syntaxTree)
{
    int         i;         /* iterate over node children */
    HashNodePtr luSymbol;  /* symbol being looked up */
    char        errorMessage[80];  

    /* used to decorate RETURN nodes with enclosing procedure */
    static TreeNode *enclosingFunction = NULL;
    
    while (syntaxTree != NULL)
    {
	/*
	 * Examine current symbol: if it's a declaration, insert into
	 *  symbol table.
	 */
	if (syntaxTree->nodekind == DecK)
	    insertSymbol(syntaxTree->name, syntaxTree, syntaxTree->lineno);
	
	/* If entering a new function, tell the symbol table */
	if ((syntaxTree->nodekind == DecK)
	    && (syntaxTree->kind.dec == FuncDecK))
	{
	    /* record the enclosing procedure declaration */
	    enclosingFunction = syntaxTree;
	    
	    if (TraceAnalyse)
		/*
		 *  For functions at least, it's nice to tell the user
		 *   whereabouts in the program the variable comes into
		 *   scope.  We don't bother printing out compound-stmt
		 *   scopes
		*/
		drawRuler(listing, syntaxTree->name);
	    
	    newScope();
	    ++scopeDepth;
	}

	/* If entering a compound-statement, create a new scope as well */
	if ((syntaxTree->nodekind == StmtK)
	    && (syntaxTree->kind.stmt == CompoundK))
	{
	    newScope();
	    ++scopeDepth;
	}

	/*
	 *  If the current node is an identifier, it needs to be checked
	 *   against the symbol table, and annotated with a pointer back to
	 *   it's declaration.
	 */

	if (((syntaxTree->nodekind == ExpK)  /* identifier reference... */
	     && (syntaxTree->kind.exp == IdK))
	    || ((syntaxTree->nodekind == StmtK)  /* function call... */
		&& (syntaxTree->kind.stmt == CallK)))
	{
	    DEBUG_ONLY(
		fprintf(listing,
			"*** Annotating identifier \"%s\" on line %d\n",
			syntaxTree->name, syntaxTree->lineno); );

	    luSymbol = lookupSymbol(syntaxTree->name);
	    if (luSymbol == NULL)
	    {
		/* operation failed; say so to user */
		sprintf(errorMessage,
			"identifier \"%s\" unknown or out of scope\n",
			syntaxTree->name);
		flagSemanticError(errorMessage);
	    }
	    else
	    {
		/*
		 *  Annotate identifier tree-node with a pointer to it's
		 *   declaration.
		 */
		syntaxTree->declaration = luSymbol->declaration;
	    }		 
	}

	/*
	 *  If the current node is a RETURN statement, we need to mark
	 *   it with the enclosing procedure's declaration node.  This
	 *   information is used later by the type checker to check
	 *   function return types.
	 */
	if ((syntaxTree->nodekind == StmtK) &&
	    (syntaxTree->kind.stmt == ReturnK))
	{
	    syntaxTree->declaration = enclosingFunction;

	    DEBUG_ONLY( fprintf(listing,
	       "*** Marking return statement on line %d with pointer to "
	       "%s() declaration\n",
               syntaxTree->lineno,
	       syntaxTree->declaration->name) );
	}
	
	for (i=0; i < MAXCHILDREN; ++i)
	    buildSymbolTable2(syntaxTree->child[i]);
	
	/* If leaving a scope, tell the symbol table */
	if (((syntaxTree->nodekind == DecK)
	    && (syntaxTree->kind.dec == FuncDecK))
	    || ((syntaxTree->nodekind == StmtK)
	    && (syntaxTree->kind.stmt == CompoundK)))
	{
	    if (TraceAnalyse)
		dumpCurrentScope();
	    --scopeDepth;
	    endScope();	
	}
	
	/* do post-order operations here */
	
	/* visit next sibling */
	syntaxTree = syntaxTree->sibling;
    }
}


static void drawRuler(FILE *output, char *string)
{
    int length;
    int numTrailingDashes;
    int i;

    /* empty string? */
    if (strcmp(string, "") == 0)
	length = 0;
    else
	length = strlen(string) + 2;
    
    fprintf(output, "---");
    if (length > 0) fprintf(output, " %s ", string);
    numTrailingDashes = 45 - length;

    for (i=0; i<numTrailingDashes; ++i)
	fprintf(output, "-");
    fprintf(output, "\n");
}


static void flagSemanticError(char *str)
{
    fprintf(listing, ">>> Semantic error (type checker): %s", str);
    Error = TRUE;
}


/* generic tree traversal routine */
static void traverse(TreeNode *syntaxTree,
		     void (*preProc)(TreeNode *),
		     void (*postProc)(TreeNode *) )
{
    int i;
    
    while (syntaxTree != NULL)
    {
	preProc(syntaxTree);

	for (i=0; i < MAXCHILDREN; ++i)
	    traverse(syntaxTree->child[i], preProc, postProc);

	postProc(syntaxTree);

	syntaxTree = syntaxTree->sibling;
    }
}


/*
 * Take a pair of tree nodes whose children contain definitions of formal
 *  parameters, and expressions-as-actual-parameters respectively.
 *
 * Compares the lists of formal and actual parameters, returns TRUE if the
 *  types and number of parameters match, FALSE otherwise.
 */

static int checkFormalAgainstActualParms(TreeNode *formal, TreeNode *actual)
{
    TreeNode *firstList;
    TreeNode *secondList;
    
    firstList = formal->child[0];
    secondList = actual->child[0];

    while ((firstList != NULL) && (secondList != NULL))
    {
	if (firstList->expressionType != secondList->expressionType)
	    return FALSE;

	if (firstList) firstList = firstList->sibling;
	if (secondList) secondList = secondList->sibling;
    }

    if (((firstList == NULL) && (secondList != NULL))
	|| ((firstList != NULL) && (secondList == NULL)))
	return FALSE;
    
    return TRUE;
}


static void checkNode(TreeNode *syntaxTree)
{
    char errorMessage[100];
    
    
    switch (syntaxTree->nodekind)
    {
    case DecK:

	switch (syntaxTree->kind.dec)
	{
	case ScalarDecK:
	    syntaxTree->expressionType = syntaxTree->variableDataType;
	    break;

	case ArrayDecK:
	    syntaxTree->expressionType = Array;
	    break;

	case FuncDecK:
	    syntaxTree->expressionType = Function;
	    break;
	}
	
	break;  /* case DecK */
	
    case StmtK:

	switch (syntaxTree->kind.stmt)
	{
	case IfK:
	    
	    if (syntaxTree->child[0]->expressionType != Integer)
	    {
		sprintf(errorMessage,
			"IF-expression must be integer (line %d)\n",
			syntaxTree->lineno);
		flagSemanticError(errorMessage);
	    }
	    
	    break;
	    
	case WhileK:

	    if (syntaxTree->child[0]->expressionType != Integer)
	    {
		sprintf(errorMessage,
			"WHILE-expression must be integer (line %d)\n",
			syntaxTree->lineno);
		flagSemanticError(errorMessage);
	    }
	    
	    break;
	    
	case CallK:

	    /*  Check types and numbers of formal against actual parameters */
	    if (!checkFormalAgainstActualParms(syntaxTree->declaration,
					       syntaxTree))
	    {
		sprintf(errorMessage, "formal and actual parameters to "
			"function don\'t match (line %d)\n",
			syntaxTree->lineno);
		flagSemanticError(errorMessage);
	    }
		
	    /*
	     *  The type of this expression (procedure invokation) is that
	     *   of the return type of the procedure being called
	     */
	    syntaxTree->expressionType
		= syntaxTree->declaration->functionReturnType;;

	    break;
	    
	case ReturnK:

	    /*
	     *  The expression attached to the RETURN statement (if any)
	     *   must have the type of the return type of the function.
	     */

	    if (syntaxTree->declaration->functionReturnType == Integer)
	    {
		if ((syntaxTree->child[0] == NULL)
		    || (syntaxTree->child[0]->expressionType != Integer))
		{
		    sprintf(errorMessage, "RETURN-expression is either "
			    "missing or not integer (line %d)\n",
			    syntaxTree->lineno);
		    flagSemanticError(errorMessage);
		}
	    }
	    else if (syntaxTree->declaration->functionReturnType == Void)
	    {
		/* does a return-expression exist? complain */
		if (syntaxTree->child[0] != NULL)
		{
		    sprintf(errorMessage, "RETURN-expression must be"
			    "void (line %d)\n", syntaxTree->lineno);
		}
	    }
	    
	    break;

	case CompoundK:

	    /*
	     * I don't think that compound-statement's type attribute
	     *  evaluates to anything in particular... make it "void".
	     */
	    
	    syntaxTree->expressionType = Void;
	    
	    break;
	}
	
	break; /* case StmtK */
	
    case ExpK:

	switch (syntaxTree->kind.exp)
	{
	case OpK:
	    /*
	     * This syntactic category includes both arithmetic and relational
	     *  operators.
	     */
	    
	    /* Arithmetic operators */
	    if ((syntaxTree->op == PLUS) || (syntaxTree->op == MINUS) ||
		(syntaxTree->op == TIMES) || (syntaxTree->op == DIVIDE))
	    {
		if ((syntaxTree->child[0]->expressionType == Integer) &&
		    (syntaxTree->child[1]->expressionType == Integer))
		    syntaxTree->expressionType = Integer;
		else
		{
		    sprintf(errorMessage, "arithmetic operators must have "
			 "integer operands (line %d)\n", syntaxTree->lineno);
		    flagSemanticError(errorMessage);
		}
	    }
	    /* Relational operators */
	    else if ((syntaxTree->op == GT) || (syntaxTree->op == LT) ||
		     (syntaxTree->op == LTE) || (syntaxTree->op == GTE) ||
		     (syntaxTree->op == EQ) || (syntaxTree->op == NE))
	    {
		if ((syntaxTree->child[0]->expressionType == Integer) &&
		    (syntaxTree->child[1]->expressionType == Integer))
		    syntaxTree->expressionType = Integer;
		else
		{
		    sprintf(errorMessage, "relational operators must have "
			  "integer operands (line %d)\n", syntaxTree->lineno);
		    flagSemanticError(errorMessage);
		}
	    }
	    else
	    {
		sprintf(errorMessage, "error in type checker: unknown operator"
			" (line %d)\n", syntaxTree->lineno);
		flagSemanticError(errorMessage);
	    }
	    
	    break;
	    
	case IdK:
	    /*
	     *  Handle identifiers. We can just have arrays, scalars, or
	     *   array element references.
	     */
	    
	    if (syntaxTree->declaration->expressionType == Integer)
	    {
		if (syntaxTree->child[0] == NULL)
		    syntaxTree->expressionType = Integer;
		else
		{
		    /* only Arrays can be indexed */
		    sprintf(errorMessage, "can't access an element in "
			    "somthing that isn\t an array (line %d)\n",
			    syntaxTree->lineno);
		    flagSemanticError(errorMessage);
		}
	    }
	    else if (syntaxTree->declaration->expressionType == Array)
	    {
		if (syntaxTree->child[0] == NULL)
		    syntaxTree->expressionType = Array;
		else
		{
		    /* Identifier is indexed by an expression */
		    if (syntaxTree->child[0]->expressionType == Integer)
			syntaxTree->expressionType = Integer;
		    else
		    {
			sprintf(errorMessage, "array must be indexed by a "
				"scalar (line %d)\n", syntaxTree->lineno);
			flagSemanticError(errorMessage);
		    }
		}
	    }
	    else
	    {
		sprintf(errorMessage, "identifier is an illegal type "
			"(line %d)\n", syntaxTree->lineno);
		flagSemanticError(errorMessage);
	    }
	    
	    break;
	    
	case ConstK:
	    
	    /* C-minus supports only integers - easy to type check. */
	    syntaxTree->expressionType = Integer;

	    break;
	    
	case AssignK:
	    
	    /* Variable assignment */
	    if ((syntaxTree->child[0]->expressionType == Integer) &&
		(syntaxTree->child[1]->expressionType == Integer))
		syntaxTree->expressionType = Integer;
	    else
	    {
		sprintf(errorMessage, "both assigning and assigned expression"
			" must be integer (line %d)\n", syntaxTree->lineno);
		flagSemanticError(errorMessage);
	    }
	    
	    break;	    
	}
	
	break; /* case ExpK */
	
    } /* switch (syntaxTree->nodekind) */     
}


void markGlobals(TreeNode *syntaxTree)
{
    TreeNode *cursor;

    DEBUG_ONLY( char scratch[80]; );
    
    cursor = syntaxTree;

    while (cursor != NULL)
    {
	if ((cursor->nodekind==DecK)&&
	    ((cursor->kind.dec==ScalarDecK)||
	     (cursor->kind.dec==ArrayDecK)))
	{

	    DEBUG_ONLY(
		sprintf(scratch, "*** Marked %s as a global variable\n",
			cursor->name);
		fprintf(listing, scratch);
		);
	    
	    cursor->isGlobal = TRUE;
	}
	
	cursor = cursor->sibling;
    }
}


/* dummy do-nothing procedure used to keep traverse() happy */
static void nullProc(TreeNode *syntaxTree)
{
    if (syntaxTree == NULL)
	return;
    else
	return;
}


/* END OF FILE */

