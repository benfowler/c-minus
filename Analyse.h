/*********************************************************************
 *  NAME:          Benjamin Fowler
 *  NUMBER:        02251132
 *  SUBJECT:       Modern Compiler Construction
 *  INSTRUCTOR:    Dr Wayne Kelly
 *********************************************************************
 *  MODULE:        Analyse.h
 *  PURPOSE:       Declares the public portions of the semantic
 *                  analyser module.
 *  DATE STARTED:  7th May, 2000.
 *  LAST EDITED:   10th May, 2000.
 *********************************************************************/


#ifndef ANALYSE_H
#define ANALYSE_H

#include "Globals.h"

/*
 * NAME:    buildSymboltable()
 * PURPOSE: Takes a syntax tree and traverses it, building a symbol table
 *           and decorating all identifiers with references to their
 *           declarations.
 */

void buildSymbolTable(TreeNode *syntaxTree);


/*
 * NAME:    typeCheck()
 * PURPOSE: Traverses the syntax tree and type checks the program.
 */

void typeCheck(TreeNode *syntaxTree);

#endif

/* END OF FILE */

