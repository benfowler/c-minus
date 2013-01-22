/*********************************************************************
 *  NAME:          Benjamin Fowler
 *  NUMBER:        02251132
 *  SUBJECT:       Modern Compiler Construction
 *  INSTRUCTOR:    Dr Wayne Kelly
 *********************************************************************
 *  MODULE:        Scan.h
 *  PURPOSE:       Declares the public portions of the Scan module.
 *  DATE STARTED:  5th March, 2000.
 *  LAST EDITED:   5th March, 2000.
 *********************************************************************/


#ifndef SCAN_H
#define SCAN_H


/* Define the maximum token length (use Turbo Pascal conventions) */
#define MAXTOKENLEN     64

/* tokenString holds the lexeme being scanned */
extern char tokenString[MAXTOKENLEN+1];

/*
 * NAME:     getToken()
 * PURPOSE:  Returns the next token in the source file.
 */

TokenType getToken(void);

#endif

/* END OF FILE */
