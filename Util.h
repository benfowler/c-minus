/*********************************************************************
 *  NAME:          Benjamin Fowler
 *  NUMBER:        02251132
 *  SUBJECT:       Modern Compiler Construction
 *  INSTRUCTOR:    Dr Wayne Kelly
 *********************************************************************
 *  MODULE:        Util.h
 *  PURPOSE:       Declares some types and operations that don't 
 *                  belong elsewhere.
 *  DATE STARTED:  23rd March, 2000.
 *  LAST EDITED:   26rd March, 2000.
 *********************************************************************/


#ifndef UTIL_H
#define UTIL_H


/*
 * NAME:    typeName()
 * PURPOSE: Given a type returns a string corresponding with the type name.
 */

char *typeName(ExpType type);


/*
 * NAME:     printToken()
 * PURPOSE:  Prints a token and it's lexeme to the listing file.
 */

void printToken(TokenType token, const char* lexeme);


/*
 * NAME:     printTree()
 * PURPOSE:  Dumps a given syntax tree.
 */

void printTree(TreeNode *tree);


/*
 * NAME:     newStmtNode()
 * PURPOSE:  Creates a new Statement node for AST construction.
 */

TreeNode *newStmtNode(StmtKind kind);


/*
 * NAME:     newExpNode()
 * PURPOSE:  Creates a new Expression node for AST construction.
 */

TreeNode *newExpNode(ExpKind kind);


/*
 * NAME:     newDecNode()
 * PURPOSE:  Creates a new Declaration node for AST construction.
 */

TreeNode *newDecNode(DecKind kind);


/*
 * NAME:     copyString()
 * PURPOSE:  Allocates space for a new string copied from an existing one.
 */

char *copyString(char *source);


#endif

/* END OF FILE */




