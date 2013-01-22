/*********************************************************************
 *  NAME:          Benjamin Fowler
 *  NUMBER:        02251132
 *  SUBJECT:       Modern Compiler Construction
 *  INSTRUCTOR:    Dr Wayne Kelly
 *********************************************************************
 *  MODULE:        CGen.h
 *  PURPOSE:       Declares the public portions of the code generator
 *                  module.
 *  DATE STARTED:  21st May, 2000.
 *  LAST EDITED:   21st May, 2000.
 *********************************************************************/


#ifndef CGEN_H
#define CGEN_H


#define STACKMARKSIZE  8


#include "Globals.h"

/*
 * Constant definitions that need to be visible elsewhere.
 */

#define WORDSIZE 4


/*
 * externs that need to be visible here
 */

extern int TraceCode;


/*
 * NAME:    codeGen()
 * PURPOSE: Generates code from the program's abstract syntax tree, and
 *           sends the resulting dcodes to the file named by "fileName".
 */

void codeGen(TreeNode *syntaxTree, char *fileName, char *moduleName);

#endif

/* END OF FILE */
