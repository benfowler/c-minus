/*********************************************************************
 *  NAME:          Benjamin Fowler
 *  NUMBER:        02251132
 *  SUBJECT:       Modern Compiler Construction
 *  INSTRUCTOR:    Dr Wayne Kelly
 *********************************************************************
 *  MODULE:        Parse.h
 *  PURPOSE:       Implements the syntactic parser for the C-minus
 *                  language using the recursive descent method.
 *  DATE STARTED:  23rd March, 2000
 *  LAST EDITED:   23rd March, 2000
 *
 *  REVISION HISTORY:
 *
 *     23rd March, 2000:   Added some skeleton definitions.
 *     26th March, 2000:   Wrote bits of parser to handle declarations.
 *     27th March, 2000:   Implemented some statement constructs.
 *     28th March, 2000:   More work on statements.
 *      (Decided to make variable assignments an expression, rather 
 *       than a statement).
 *     1st March, 2000:    Put some finishing touches on the parser.
 *     2nd March, 2000:    Performed testing.
 *
 *********************************************************************/


#include "Globals.h"
#include "Util.h"
#include "Scan.h"
#include "Parse.h"


/*  Module-global state to hold the current token */

static TokenType token;


/* Function prototypes for recursive calls */
static TreeNode *declaration_list(void);
static TreeNode *declaration(void);
static TreeNode *var_declaration(void);
static TreeNode *param_list(void);
static TreeNode *param(void);

static TreeNode *compound_statement(void);
static TreeNode *local_declarations(void);
static TreeNode *statement_list(void);
static TreeNode *statement(void);
static TreeNode *expression_statement(void);
static TreeNode *selection_statement(void);
static TreeNode *iteration_statement(void);
static TreeNode *return_statement(void);
static TreeNode *expression(void);
static TreeNode *simple_expression(TreeNode *passdown);
static TreeNode *additive_expression(TreeNode *passdown);
static TreeNode *term(TreeNode *passdown);
static TreeNode *factor(TreeNode *passdown);
static TreeNode *args(void);
static TreeNode *arg_list(void);

static TreeNode *ident_statement(void);


static void syntaxError(char *message)
{
    fprintf(listing, ">>> Syntax error at line %d: %s", lineno, message);
    Error = TRUE;   /* global variable to inhibit subseq. passes on error */
}


static void match(TokenType expected)
{
    if (token == expected)
        token = getToken();
    else
    {
        syntaxError("unexpected token ");
	printToken(token, tokenString);
        fprintf(listing, "\n");
    }
}


static ExpType matchType()
{
    ExpType t_type = Void;


    switch(token)
    {
    case INT:  t_type = Integer; token=getToken(); break;
    case VOID: t_type = Void; token=getToken(); break;
    default: {
             syntaxError("expected a type identifier but got a ");
             printToken(token, tokenString);
             fprintf(listing, "\n");
             break;
        }
    }

    return t_type;
}


static int isAType(TokenType tok)
{
    if ((tok == INT) || (tok == VOID))
        return TRUE;
    else
        return FALSE;
}



static TreeNode *declaration_list(void)
{
    TreeNode *tree;
    TreeNode *ptr;


    DEBUG_ONLY( fprintf(listing, "*** Entered declaration_list()\n"); )

    tree = declaration();
    ptr = tree;

    while (token != ENDOFFILE)
    {
	TreeNode *q;  /* temp node */

	q = declaration();
	if ((q != NULL) && (ptr != NULL))
	{
            ptr->sibling = q;
            ptr = q;
	}
    }

    DEBUG_ONLY( fprintf(listing, "*** Exiting declaration_list()\n"); )

    return tree;
}


static TreeNode *declaration(void)
{
    TreeNode  *tree = NULL;
    ExpType   decType;
    char      *identifier;
    

    DEBUG_ONLY( fprintf(listing, "*** Entered declaration()\n"); )

    decType = matchType();   /* get type of declaration */

    identifier = copyString(tokenString);
    match(ID);
    
    switch(token)
    {
    case SEMI:     /* variable declaration */

        tree = newDecNode(ScalarDecK);

	if (tree != NULL)
	{
	    tree->variableDataType = decType;
	    tree->name = identifier;
	}
	
	match(SEMI);
	break;
	
    case LSQUARE:  /* array declaration */
	tree = newDecNode(ArrayDecK);

	if (tree != NULL)
	{
	    tree->variableDataType = decType;
	    tree->name = identifier;
	}
	
	match(LSQUARE);
	
	if (tree != NULL) tree->val = atoi(tokenString);

	match(NUM);
	match(RSQUARE);
	match(SEMI);
	break;
	
    case LPAREN:   /* function declaration */
	tree = newDecNode(FuncDecK);

	if (tree != NULL)
	{
	    tree->functionReturnType = decType;
	    tree->name = identifier;
	}
	
	match(LPAREN);
	if (tree != NULL) tree->child[0] = param_list();
	match(RPAREN);
	if (tree != NULL) tree->child[1] = compound_statement();
	break;
	
    default:
	syntaxError("unexpected token ");
	printToken(token, tokenString);
        fprintf(listing, "\n");
	token = getToken();
	break;
    }

    DEBUG_ONLY( fprintf(listing, "*** Exiting declaration()\n"); )

    return tree;
}


static TreeNode *var_declaration(void)
{
    TreeNode  *tree = NULL;
    ExpType   decType;
    char      *identifier;
    

    DEBUG_ONLY( fprintf(listing, "*** Entered var_declaration()\n"); )

    decType = matchType();

    identifier = copyString(tokenString);
    match(ID);
    
    switch(token)
    {
    case SEMI:     /* variable declaration */

	tree = newDecNode(ScalarDecK);

	if (tree != NULL)
	{
	    tree->variableDataType = decType;
	    tree->name = identifier;
	}
	
	match(SEMI);
	break;
	
    case LSQUARE:  /* array declaration */
	tree = newDecNode(ArrayDecK);

	if (tree != NULL)
	{
	    tree->variableDataType = decType;
	    tree->name = identifier;
	}
	
	match(LSQUARE);
	
	if (tree != NULL) tree->val = atoi(tokenString);

	match(NUM);
	match(RSQUARE);
	match(SEMI);
	break;
	
    default:
	syntaxError("unexpected token ");
	printToken(token, tokenString);
        fprintf(listing, "\n");
	token = getToken();
	break;
    }

    return tree;
}


static TreeNode *param(void)
{
    TreeNode *tree;
    ExpType  parmType;
    char     *identifier;
    
    
    DEBUG_ONLY( fprintf(listing, "*** Entered param()\n"); )

    parmType = matchType();  /* get type of formal parameter */
 
    identifier = copyString(tokenString);
    match(ID);

    /* array-type formal parameter */
    if (token == LSQUARE)
    {
	match(LSQUARE);
	match(RSQUARE);

	tree = newDecNode(ArrayDecK);
    }
    else
	tree = newDecNode(ScalarDecK);
	
    if (tree != NULL)
    {
	tree->name = identifier;
	tree->val = 0;
	tree->variableDataType = parmType;
	tree->isParameter = TRUE;
    }
    
    return tree;
}


static TreeNode *param_list(void)
{
    TreeNode *tree;
    TreeNode *ptr;
    TreeNode *newNode;


    DEBUG_ONLY( fprintf(listing, "*** Entered param_list()\n"); )

    if (token == VOID)
    {
        match(VOID);
        return NULL;
    }

    tree = param();
    ptr = tree;

    while ((tree != NULL) && (token == COMMA))
    {
	match(COMMA);
	newNode = param();
	if (newNode != NULL)
        {
	    ptr->sibling = newNode;
	    ptr = newNode;
        }
    }

    return tree;
}


static TreeNode *compound_statement(void)
{
    TreeNode *tree = NULL;

    DEBUG_ONLY( fprintf(listing, "*** Entered compound_statement()\n"); )

    match(LBRACE);

    if ((token != RBRACE) && (tree = newStmtNode(CompoundK)))
    {
	if (isAType(token))
	    tree->child[0] = local_declarations();
	if (token != RBRACE)
	    tree->child[1] = statement_list();
    }

    match(RBRACE);

    DEBUG_ONLY( fprintf(listing, "*** Exiting compound_statement()\n"); )

    return tree;
}


static TreeNode *local_declarations(void)
{
    TreeNode *tree;
    TreeNode *ptr;
    TreeNode *newNode;

    DEBUG_ONLY( fprintf(listing, "*** Entered local_declarations()\n"); )

    /* find first variable declaration, if it exists */
    if (isAType(token))
	tree = var_declaration();

    /* subsequent variable declarations */
    if (tree != NULL)
    {
	ptr = tree;

	while (isAType(token))
	{
	    newNode = var_declaration();
	    if (newNode != NULL)
	    {
		ptr->sibling = newNode;
		ptr = newNode;
	    }
	}
    }

    return tree;
}


static TreeNode *statement_list(void)
{
    TreeNode *tree = NULL;
    TreeNode *ptr;
    TreeNode *newNode;

    DEBUG_ONLY( fprintf(listing, "*** Entered statement_list()\n"); )

    if (token != RBRACE)
    {
        tree = statement();
        ptr = tree;

        while (token != RBRACE)
        {
            newNode = statement();
            if ((ptr != NULL) && (newNode != NULL))
            {
                ptr->sibling = newNode;
                ptr = newNode;
            }
        }
    }

    DEBUG_ONLY( fprintf(listing, "*** Exiting statement_list()\n"); )

    return tree;
}


static TreeNode *statement(void)
{
    TreeNode *tree = NULL;

    DEBUG_ONLY( fprintf(listing, "*** Entered statement()\n"); )

    switch(token)
    {
    case IF: 
        tree = selection_statement();
        break;
    case WHILE: 
        tree = iteration_statement();
        break;
    case RETURN:
        tree = return_statement();
        break;
    case LBRACE:
        tree = compound_statement();
        break;
    case ID:
    case SEMI:
    case LPAREN:
    case NUM:
        tree = expression_statement();
        break;
    default:
        syntaxError("unexpected token ");
        printToken(token, tokenString);
        fprintf(listing, "\n");
        token = getToken();
        break;
    }

    DEBUG_ONLY( fprintf(listing, "*** Exiting statement()\n"); )

    return tree;
}


static TreeNode *expression_statement(void)
{
    TreeNode *tree = NULL;

    DEBUG_ONLY( fprintf(listing, "*** Entered expression_statement()\n"); )

    if (token == SEMI) 
        match(SEMI);
    else if (token != RBRACE)
    {
        tree = expression();
        match(SEMI);
    }

    DEBUG_ONLY( fprintf(listing, "*** Exiting expression_statement()\n"); )

    return tree;
}


static TreeNode *selection_statement(void)
{
    TreeNode *tree;
    TreeNode *expr;
    TreeNode *ifStmt;
    TreeNode *elseStmt = NULL;


    DEBUG_ONLY( fprintf(listing, "*** Entered selection_statement()\n"); )

    match(IF);
    match(LPAREN);
    expr = expression();
    match(RPAREN);
    ifStmt = statement();

    if (token == ELSE)
    {
	match(ELSE);
	elseStmt = statement();
    }

    tree = newStmtNode(IfK);
    if (tree != NULL)
    {
	tree->child[0] = expr;
	tree->child[1] = ifStmt;
	tree->child[2] = elseStmt;
    }

    return tree;
}


static TreeNode *iteration_statement(void)
{
    TreeNode *tree;
    TreeNode *expr;
    TreeNode *stmt;


    DEBUG_ONLY( fprintf(listing, "*** Entered iteration_statement()\n"); )

    match(WHILE);
    match(LPAREN);
    expr = expression();
    match(RPAREN);
    stmt = statement();
    
    tree = newStmtNode(WhileK);
    if (tree != NULL)
    {
	tree->child[0] = expr;
	tree->child[1] = stmt;
    }

    return tree;
}


static TreeNode *return_statement(void)
{
    TreeNode *tree;
    TreeNode *expr = NULL;


    DEBUG_ONLY( fprintf(listing, "*** Entered return_statement()\n"); )

    match(RETURN);

    tree = newStmtNode(ReturnK);
    if (token != SEMI) 
	expr = expression();

    if (tree != NULL) 
	tree->child[0] = expr;

    match(SEMI);

    return tree;
}


static TreeNode *expression(void)
{
    TreeNode *tree = NULL;
    TreeNode *lvalue = NULL;
    TreeNode *rvalue = NULL;
    int gotLvalue = FALSE;  /* boolean */

    DEBUG_ONLY( fprintf(listing, "*** Entered expression()\n"); )

    /*
     *  At this point in the parse, we need to make a choice between
     *   parsing either an assignment or a simple-expression.
     *  To make the decision, we need to parse ident_statement *now*.
     */

    if (token == ID)
    {
        DEBUG_ONLY( fprintf(listing, ">>>   Parsing ident_statement\n"); ) ;
        lvalue = ident_statement();
        gotLvalue = TRUE;
    }

    /* Assignment? */
    if ((gotLvalue == TRUE) && (token == ASSIGN))
    {
        if ((lvalue != NULL) && (lvalue->nodekind == ExpK) && 
            (lvalue->kind.exp == IdK))
        {
DEBUG_ONLY( fprintf(listing, ">>>   Generating node for ASSIGN\n"); ) ;
            match(ASSIGN);
            rvalue = expression();

            tree = newExpNode(AssignK);
            if (tree != NULL)
            {
                tree->child[0] = lvalue;
                tree->child[1] = rvalue;
            }
        }
        else
        { 
            syntaxError("attempt to assign to something not an lvalue\n");
            token = getToken();
        }
    }
    else
        tree = simple_expression(lvalue);

    return tree;
}


static TreeNode *simple_expression(TreeNode *passdown)
{
    TreeNode *tree;
    TreeNode *lExpr = NULL;
    TreeNode *rExpr = NULL;
    TokenType operator;

    DEBUG_ONLY( fprintf(listing, "*** Entered simple_expression()\n"); )

    lExpr = additive_expression(passdown);

    if ((token == LTE) || (token == GTE) || (token == GT) 
     || (token == LT) || (token == EQ) || (token == NE))
    {
        operator = token;
        match(token);
        rExpr = additive_expression(NULL);

        tree = newExpNode(OpK);
        if (tree != NULL) 
        {
            tree->child[0] = lExpr;
            tree->child[1] = rExpr;
            tree->op = operator;
        }
    }
    else 
        tree = lExpr;

    return tree;
}


static TreeNode *additive_expression(TreeNode *passdown)
{
    TreeNode *tree;
    TreeNode *newNode;

    DEBUG_ONLY( fprintf(listing, "*** Entered additive_expression()\n"); )

    tree = term(passdown);

    while ((token == PLUS) || (token == MINUS))
    {
        newNode = newExpNode(OpK);
        if (newNode != NULL)
        {
            newNode->child[0] = tree;
            newNode->op = token;
            tree = newNode;
            match(token);
            tree->child[1] = term(NULL);
        }
    }

    return tree;
}


static TreeNode *term(TreeNode *passdown)
{
    TreeNode *tree;
    TreeNode *newNode;

    DEBUG_ONLY( fprintf(listing, "*** Entered term()\n"); )

    tree = factor(passdown);

    while ((token == TIMES) || (token == DIVIDE))
    {
        newNode = newExpNode(OpK);

        if (newNode != NULL)
        {
            newNode->child[0] = tree;
            newNode->op = token;
            tree = newNode;
            match(token);
            newNode->child[1] = factor(NULL);
        }
    }

    return tree;
}


static TreeNode *factor(TreeNode *passdown)
{
    TreeNode *tree = NULL;

    DEBUG_ONLY( fprintf(listing, "*** Entered factor()\n"); )

    /* If the subtree in "passdown" is a Factor, pass it back. */
    if (passdown != NULL)
    {
        DEBUG_ONLY( fprintf(listing, ">>>   Returning passdown subtree\n"); )
        return passdown;
    }

    if (token == ID)
    {
        tree = ident_statement();
    }
    else if (token == LPAREN)
    {
        match(LPAREN);
        tree = expression();
        match(RPAREN);
    }
    else if (token == NUM)
    {
        tree = newExpNode(ConstK);
        if (tree != NULL)
        {
            tree->val = atoi(tokenString);
            tree->variableDataType = Integer;
        }
        match(NUM);
    }
    else
    {
        syntaxError("unexpected token ");
        printToken(token, tokenString);
        fprintf(listing, "\n");
        token = getToken();
    }

    return tree;
}


static TreeNode *ident_statement(void)
{
    TreeNode *tree;
    TreeNode *expr = NULL;
    TreeNode *arguments = NULL;
    char *identifier;


    DEBUG_ONLY( fprintf(listing, "*** Entered ident_statement()\n"); )

    if (token == ID)
        identifier = copyString(tokenString);
    match(ID);

    if (token == LPAREN)
    {
        match(LPAREN);
        arguments = args();
        match(RPAREN);

        tree = newStmtNode(CallK);
        if (tree != NULL)
        {
            tree->child[0] = arguments;
            tree->name = identifier;
        }
    }
    else
    {
        if (token == LSQUARE)
        {
            match(LSQUARE);
            expr = expression(); 
            match(RSQUARE);
        }

        tree = newExpNode(IdK);
        if (tree != NULL)
        {
            tree->child[0] = expr;
            tree->name = identifier;
        }
    }

    return tree;
}


static TreeNode *args(void)
{
    TreeNode *tree = NULL;

    DEBUG_ONLY( fprintf(listing, "*** Entered args()\n"); )

    if (token != RPAREN)
        tree = arg_list();

    return tree;
}


static TreeNode *arg_list(void)
{
    TreeNode *tree;
    TreeNode *ptr;
    TreeNode *newNode;

    DEBUG_ONLY( fprintf(listing, "*** Entered arg_list()\n"); )

    tree = expression();
    ptr = tree;

    while (token == COMMA)
    {
        match(COMMA);
        newNode = expression();

        if ((ptr != NULL) && (tree != NULL))
        {
            ptr->sibling = newNode;
            ptr = newNode;
        }
    }

    return tree;
}


TreeNode *Parse(void)
{
    TreeNode *t;

    DEBUG_ONLY( fprintf(listing, "*** Entered Parse()\n"); )

    token = getToken();
    t = declaration_list();
    if (token != ENDOFFILE)
	syntaxError("Unexpected symbol at end of file\n");

    /* t points to the fully-constructed syntax tree */
    return t;
}

/* END OF FILE */


