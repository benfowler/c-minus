/*********************************************************************
 *  NAME:          Benjamin Fowler
 *  NUMBER:        02251132
 *  SUBJECT:       Modern Compiler Construction
 *  INSTRUCTOR:    Dr Wayne Kelly
 *********************************************************************
 *  MODULE:        SymTab.h
 *  PURPOSE:       Declares the public portions of the SymTab module.
 *  DATE STARTED:  7th May, 2000.
 *  LAST EDITED:   8th May, 2000.
 *
 *  REVISION HISTORY:
 *
 *  7th May 2000  Created module
 *  8th May 2000  Wrote hash table with high-water-mark support
 *  8th May 2000  Refined and reworked module interface
 *
 *********************************************************************/


#ifndef SYMTAB_H
#define SYMTAB_H

#include "Globals.h"

/*********************************************************************
 * Type definitions that need to be visible from the outside
 */

/*
 * NOTE: for some reason, Louden maintains a list of line numbers for his
 *        identifier references in his symbol table.  I reckon that it's
 *        completely useless, so I'm ditching that functionality for the
 *        time being.
 */

typedef struct hashNode {
    char          *name;        /* the name of the identifier */
    TreeNode      *declaration;  /* pointer to the symbol's dec. node */
    int           lineFirstReferenced;   /* self-explanatory */
    struct hashNode *next;      /* next node on this bucket chain */
} HashNode, *HashNodePtr;


/*
 *  Keep track of how deep the scopes are in the symbol table - used for
 *   reporting.
 */
extern int scopeDepth;


/*
 * NAME:    initSymbolTable()
 * PURPOSE: Ensures that everything is initialised and setup properly prior
 *           to first use.
 */

void initSymbolTable();


/*
 * NAME:    insertSymbol()
 * PURPOSE: Inserts line numbers and a TreeNode pointer to an identifier's
 *           declaration.
 */

void insertSymbol(char *name, TreeNode *symbolDefNode, int lineDefined);


/*
 * NAME:    symbolAlreadyDeclared()
 * PURPOSE: Checks to see if the given symbol is already defined in the
 *           current scope.
 *
 *          In C-, it is illegal to declare an identifier more than once
 *           within a given scope.
 */

int symbolAlreadyDeclared(char *name);


/*
 * NAME:    lookupSymbol()
 * PURPOSE: Retrieves a HashNode* pointer for a supplied identifier that
 *           points to the declaration node for the identifier. If the
 *           operaton failed, NULL is returned.
 */

HashNodePtr lookupSymbol(char *name);


/*
 * NAME:    dumpCurrentScope()
 * PURPOSE: Dumps out the current scope in the symbol table.
 */

void dumpCurrentScope();


/*
 * NAME:    newScope()
 * PURPOSE: Creates a new scope by creating a new "high water mark" on
 *           the symbol table.  This facilitates the destruction of all
 *           symbol table entries for a local scope on a subsequent
 *           call to endScope().
 */

void newScope();


/*
 * NAME:    endScope()
 * PURPOSE: Deletes all the local variables in the local scope by deleting
 *           all symbol table entries up to a high-water mark created by
 *           an earlier call to newScope().
 */

void endScope();


#endif

/* END OF FILE */
