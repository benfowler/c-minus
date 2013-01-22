/*********************************************************************
 *  NAME:          Benjamin Fowler
 *  NUMBER:        02251132
 *  SUBJECT:       Modern Compiler Construction
 *  INSTRUCTOR:    Dr Wayne Kelly
 *********************************************************************
 *  MODULE:        Main.c
 *  PURPOSE:       Main program.
 *  DATE STARTED:  5th March, 2000.
 *  LAST EDITED:   10th May, 2000.
 *
 *  REVISION HISTORY:
 *
 *   5th March     Created module.
 *   6th March     Polished, added getopt() command-line parsing.
 *   26th March    Altered code to use parser.
 *   10th May      Altered code to use semantic analyser.
 *
 *********************************************************************/

#include <getopt.h>

#include "Globals.h"
#include "Util.h"

/*
 * We will use conditional compilation in the same style of Louden's
 *  Tiny C compiler.  This lets us write the compiler incrementally
 *  and test as we go.
 *
 * This code sports some minor refinements over that presented in Louden's
 *  book: it attempts to make output slightly easier to follow, and includes
 *  proper command-line parsing, so selectively enabling/disabling the
 *  various types of debug output sucks less.
 */

/* Set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE   FALSE

/* Set NO_ANALYSE to TRUE to get a parser-only compiler */
#define NO_ANALYSE FALSE

/* Set NO_CODE to get a compiler that does not generate code */
#define NO_CODE    FALSE

#if NO_PARSE
#define BUILDTYPE "SCANNER ONLY"
#include "Scan.h"   /* Test the scanner only */
#else
#include "Parse.h"
#if !NO_ANALYSE
#define BUILDTYPE "SCANNER/PARSER/ANALYSER ONLY"
#include "Analyse.h"
#if !NO_CODE
#undef BUILDTYPE
#define BUILDTYPE "COMPLETE COMPILER"
#include "CGen.h"
#endif
#else
#define BUILDTYPE "SCANNER/PARSER ONLY"
#endif
#endif

/*
 * Global variables.  See "Globals.h".
 */

int lineno = 0;

FILE* source;    /*  The input source file            */
FILE* listing;   /*  The listing output file          */
FILE* code;      /*  The target code destination file */

/*
 *  Various command-line settable options.  See "Globals.h".
 */

#define MAXFILENAMESIZE  64

char sourceFileName[MAXFILENAMESIZE];

int EchoSource   = FALSE;
int TraceScan    = FALSE;
int TraceParse   = FALSE;
int TraceAnalyse = FALSE;
int TraceCode    = FALSE;

int Error = FALSE;

/* The syntax tree */
TreeNode *syntaxTree;


/*
 *  Here is a routine that uses getopt() to parse command-line arguments.
 *  With luck, this will be reasonably portable.
 *
 *  Function returns a boolean value: if non-zero, then an error occurred.
 *
 *  FIXME: Test to see if this code will compile as a Windows console-mode
 *         executable.
 */

int ParseCommandLine(int argc, char **argv)
{
    char c;                   /* the option being parsed    */
    int  errorFlag = 0;       /* has an error occurred yet? */
    int  gotSourceName = 0;   /* we need the source file name */


    opterr = 0;  /* Suppress getopt()'s default error-handing behavior */
    while ((c = getopt(argc, argv, "slyacf:")) != EOF)
    {
        switch(c)
        {
            case 's':
                EchoSource = TRUE;
                break;
            case 'l':
                TraceScan = TRUE;
                break;
            case 'y':
                TraceParse = TRUE; 
                break;
            case 'a':
                TraceAnalyse = TRUE;
                break;
            case 'c':
                TraceCode = TRUE; 
                break;
            case 'f':
                /* Can't specify filename more than once */
                if (gotSourceName)
                    errorFlag++;
                else
                {
                    gotSourceName = TRUE;
                    strncpy(sourceFileName, optarg, MAXFILENAMESIZE);
                }
                break;
            default:
                errorFlag++;
        }  /* switch(c) */
    }

    /* Source file argument is mandatory */
    if (!gotSourceName) ++errorFlag;

    return errorFlag;
}

/*
 * Print the usage for the compiler.
 */

void usage(void)
{
    fprintf(stderr, COPYRIGHT);
    fprintf(stderr, USAGE);
}


/*
 * Implementation of main().
 */


int main(int argc, char **argv)
{
    /* Handle the fiddliness of command line arguments elsewhere */
    if (ParseCommandLine(argc, argv) != 0)
    {
        usage();
        exit(1);
    }

    /* If the supplied filename lacks an extension, add one. */
    if (strchr(sourceFileName, '.') == NULL)
	strcat(sourceFileName, ".cm");

    /* Open the source file */
    source = fopen(sourceFileName, "r");

    /* If it failed, bomb out. */
    if (source == NULL)
    {
	fprintf(stderr, "Sorry, but the source file %s could not be found.\n",
		sourceFileName);
	exit(1);
    };

    /* By default, send output to standard output */
    listing = stdout;

    fprintf(listing, COPYRIGHT "\n");
    fprintf(listing, "*** C- COMPILATION: %s\n", sourceFileName);
    fprintf(listing, "*** Compiler built as " BUILDTYPE " version.\n");

    /* If the compiler was built scanner-only, then only run the scanner */
#if NO_PARSE
    while (getToken() != ENDOFFILE)
    {
        /* do nothing */
    };
#else
    fprintf(listing, "*** Parsing source program...\n");
    syntaxTree = Parse();

    /* Tracing enabled?  Let's have it... */
    if (TraceParse)
    {
        fprintf(listing, "*** Dumping syntax tree\n");
	printTree(syntaxTree);
    };

#if !NO_ANALYSE
    if (!Error)
    {
	fprintf(listing, "*** Building symbol table...\n");
	buildSymbolTable(syntaxTree);
	fprintf(listing, "*** Performing type checking...\n");
	typeCheck(syntaxTree);
    }

#if !NO_CODE
    if (!Error)
    {
	codeGen(syntaxTree, "output.dcl", "output");

	/* did code generation succeed? */
	if (!Error)
	{
	    fprintf(listing,"*** Output written to \"output.dcl\"\n");

	    /* tracing? remind user */
	    if (TraceCode)
		fprintf(listing,
			"*** CODE TRACING OPTION ENABLED; see output\n");
	}
    }
    
#endif    
#endif
#endif
    
    if (!Error)
        fprintf(listing,"*** COMPILATION COMPLETE: %d lines processed.\n", 
                lineno);
    else
        fprintf(listing,"*** ERRORS WERE ENCOUNTERED: %d lines processed.\n", 
                lineno);
    
    return EXIT_SUCCESS;
}


/* END OF FILE */

