/*********************************************************************
 *  NAME:          Benjamin Fowler
 *  NUMBER:        02251132
 *  SUBJECT:       Modern Compiler Construction
 *  INSTRUCTOR:    Dr Wayne Kelly
 *********************************************************************
 *  MODULE:        Globals.h
 *  PURPOSE:       Defines a set of constants, variables and global
 *                  variables used throughout the program.
 *  DATE STARTED:  5th March 2000.
 *  LAST EDITED:   10th May 2000.
 *
 *  SPECIAL USAGE NOTES:
 *
 *  This module must be included before all other include files.
 *  The reader will also note that most of these definitions are
 *  similar to that of Louden in "Compiler Construction: Principles
 *  and Practice".  In a way, this can't be helped.
 *
 *********************************************************************/


/* Prevent multiple includes */

#ifndef GLOBALS_H
#define GLOBALS_H


/* Some stuff that gets used in the main program */

#define REVISION "20000510"

#define COPYRIGHT "Ben Fowler\'s C- compiler, revision " REVISION ".\n"\
"Written by Ben Fowler (02251132), for ITB464 Modern Compiler Construction.\n"\
"Parts borrowed from K. J. Louden\'s Tiny C Compiler.\n"

#define USAGE \
"\nUsage:  compiler [-s|-l|-y|-a|-c] -f <file>\n"\
"\n"\
"The following are valid command-line options:\n"\
"\n"\
"  -s    Echo the source code as parsing proceeds.\n"\
"  -l    Show lexer debug output in source listing.\n"\
"  -y    Show parser debug output in source listing.\n"\
"  -a    Show semantic analyser output in source listing.\n"\
"  -c    Show code generation output in source listing.\n"\
"\n"\
"  -f <filename>     Specify the source file to compile.\n"


/* Includes that are used everywhere */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

/* Redefine FALSE and TRUE */

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif


/* A useful debugging macro */
#ifdef DEBUG
#define DEBUG_ONLY(foo) foo
#else
#define DEBUG_ONLY(foo) /* foo */
#endif

/* The maximum number of reserved words in our language */

#define MAXRESERVED  6


/*
 * Define an enumeration to hold the various tokens we can expect from
 *  a valid C- source file.
 */

typedef enum {
    /* Bookkeeping tokens */
    ENDOFFILE, ERROR,
    /* Reserved words */
    IF, ELSE, INT, RETURN, VOID, WHILE,
    /* More complex tokens */
    ID, NUM,
    /* Various operators */
    PLUS, MINUS, TIMES, DIVIDE, LT, GT, ASSIGN, NE, SEMI, COMMA,
    LPAREN, RPAREN, LSQUARE, RSQUARE, LBRACE, RBRACE, LTE, GTE, EQ
} TokenType;

/*
 * Define the constants, enumerations and structs that define the 
 *  abstract syntax tree representation of the C-minus program in memory.
 */

typedef enum { StmtK, ExpK, DecK } NodeKind;
typedef enum { IfK, WhileK, ReturnK, CallK, CompoundK } StmtKind;
typedef enum { OpK, IdK, ConstK, AssignK } ExpKind;
typedef enum { ScalarDecK, ArrayDecK, FuncDecK } DecKind;

/* Used in the type checker */
typedef enum { Void, Integer, Boolean, Array, Function } ExpType;

#define MAXCHILDREN  3

typedef struct treeNode 
{
    struct treeNode *child[MAXCHILDREN];
    struct treeNode *sibling;
    int             lineno;

    NodeKind        nodekind;  /* the type of the AST node */

    union {
        StmtKind        stmt;
        ExpKind         exp;
        DecKind         dec;
    } kind;

    TokenType  op;
    int        val;
    char       *name;

    /*
     * If the node is a function definition, this holds the function's
     *  return type.
     */
    ExpType    functionReturnType;

    /*
     *  If the node is a variable, then we need to record the data type.
     */
    ExpType    variableDataType;
    
    /*
     * The following is used in the type checking of expressions
     */
    ExpType         expressionType;

    /*
     * If isParameter is TRUE, then this node declares an actual parameter
     *   to a function.
     */
    int isParameter;

    /* If isGlobal is TRUE, then the variable is a global */
    int isGlobal;
    
    /*
     * (bjf, 7/5/2000) The following is used in the semantic analyser to
     *  allow the code generator to locate type information for a given
     *  identifier.
     */
    struct treeNode *declaration;

    /*
     * (bjf, 21/5/2000) Added a pair of attributes needed to generate
     *  DCode from the abstract syntax tree.  "assemblyAreaSize" is
     *  used in procedure declarations to indicate how big the
     *  assembly area needs to be for preparing parameters for passing
     *  to other functions. "localSize" is the size of the area on the
     *  stack that holds local variables.
     */

    int assemblyAreaSize;
    int localSize;

    /*
     * (bjf, 9/6/2000) Added an attribute to store the relative offset
     *  from the LP if the node is a local variable declaration, or
     *  relative offset from AP if the node is a function parameter defn.
     */
    int offset;
    
} TreeNode;


/**********************************************************************
 * Variables that are global to the entire program
 **********************************************************************/

extern FILE* source;    /* Input source file           */
extern FILE* listing;   /* Listing output              */
extern FILE* code;      /* Output file of the compiler */

extern int lineno;      /* The current line number of the source file */

/* Tracing options, in the style of Louden's C- compiler */

/*
 * EchoSource - if set to TRUE, causes the source program to be echoed
 *  out to the listing file with line numbers during scanning.
 */

extern int EchoSource;


/*
 * TraceScan - if set to TRUE, causes token information to get dumped
 *  to the listing file during lexical scanning of the source file.
 */

extern int TraceScan;

/* TraceParse: get a parse tree displayed in the listing file */
extern int TraceParse;

/* TraceAnalyse: get a dump of the symbol table during semantic analysis */
extern int TraceAnalyse;

/*
 * Error - if set to TRUE, prevents execution of subsequent passes if an
 *  error occurs.
 */

extern int Error;

#endif

/* END OF FILE */
