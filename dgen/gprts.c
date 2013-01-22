/*
$Log: gprts.c,v $
Revision 1.1  2000/06/08 12:21:15  fowlerb

Added lecturer-supplied material from ILE to get build environment working.

Revision 1.1.1.1  1998/12/18 04:00:55  chan
Created

Revision 1.1  1996/09/06 08:08:48  lederman
Initial revision

Revision 1.5  1996/03/06 06:05:00  lederman
SIGBUS stuff moved outside i386 as some sytems don't even define it anymore

Revision 1.4  1996/01/04 00:48:09  lederman
cleaned up StateVec definition so size required by each platform is #defined

Revision 1.3  1995/10/17 04:49:06  lederman
Fiddling about with Alpha overflow checks and removed FPovf message

Revision 1.2  1995/10/12 00:47:48  lederman
Disabled the FP_Overflow msg. (Alpha) as it gives spurious msg. for MOD & REM
and is pretty silly anyway.  The variable itself is used by GPMD...

Revision 1.1  1995/09/26 02:40:09  lederman
Initial revision

 * *************************************************************************/
/*
 *     Copyright (c) 1990--92 Faculty of Information Technology
 *          Queensland University of Technology, BRISBANE
 *
 *    This is the C include file for runtime operations in the 
 *    generic native code backends for all gardens point compilers.
 *    Language-specific code is in "gpXrts.c", where X = (m|o)
 *
 *        --------------- gprts.c --------------
 *             SPECIAL VERSION WITH COROUTINE SUPPORT
 *        --------------- gprts.c --------------
 *
 *           John Gough, November 1992
 *    extracted from the current m2rts for i386 flat address
 *     Open Array Parameters (soap) manager code included for
 *    backward compatability for "via-C" object code ...
 *
 *        version with display vector
 * 
 *    (kjg Mar 91) Modified to support optional param of Assert
 *    (kjg Mar 91) Modified to support _fLow as well as _fVec
 *    (kjg Oct 92) Modified for i386 native code traps ...
 *    (paa Nov 92) Modified for i386 trunc and round tables
 *    (kjg Nov 92) Merging proto signal handling with
 *            above code. Convert tables disabled.
 *    (jl  Mar 93) Merged with decmips version
 *    (pms May 93) Merged with generic code from m2rts.c.  Currently
 *                 NATIVE code is implemented for decmips and i386
 *    (pms Jun 93) Added APOLLO specifics
 *    (pms Jun 93) Added dummy _exPtr for coroutine implementation.
 *                 Shoud be removed when coroutines and exceptions are
 *                 upgraded.
 *    (pms Oct 93) Added modifications for 64 bit int and unsigned
 *                 int for DEC Alpha implementation.
 *    (pms Jan 94) Added more NATIVE Code traps and changed error
 *                 messages.
 *    (pms Feb 94) Added options for LINUX (386) version
 *    (pms Aug 95) Included signal.h for machten
 *  
 * ****************************************************************** * 
 * ****************************************************************** * 
 *                                                                    *
 *                            <<<< NOTE >>>>                          *
 *                                                                    *
 *   As from 6 June 1993, specific code is included for the Native    *
 *   code backends (currently SUN Solaris, decmips and i386).         *
 *   The relevant code is protected by #ifdef NATIVE precompiler      *
 *   directives.  NATIVE is defined only if the platform type is      *
 *   either Solaris, Decmips or i386.  This means that NATIVE specific*
 *   code is INCLUDED for both Native and NON Native compilations     *
 *   of GPM.  At present this inclusion of extra (NATIVE) code into   *
 *   "via C" versions does not appear to cause any problems.          *
 *                                                                    *
 *                                                                    *
 *                           <<<< long int >>>>                       *
 *                                                                    *
 *   With the porting of GPM to the 64 bit DEC Alpha (October 93),    *
 *   the following changes have been made :                           *
 *                                                                    *
 *             type int            -> long int                        *
 *                  unsigned int   -> unsigned long int               *
 *                  bitsInWord     -> 64 for Alpha, 32 for all other  *
 *                                    platforms                       *
 *                  LOB            -> (Log of bitsInWord), 6 for      *
 *                                    Alpha, 5 for other platforms    *
 *   The modification of type int to long int and corresponding       *
 *   unsigned int, is almost universal, except for places where "c"   *
 *   system functions return only type int.   On non Alpha platforms  *
 *   long int and unsigned long int both map to the 32 bit int and    *
 *   unsigned respectively.                                           *
 *                                                                    *
 * ****************************************************************** * 
 * ****************************************************************** * 
 */
/*
 *     32 / 64 BIT DIFFERENCES
 */
# ifdef alpha
#    define bitsInWord 64
#    define LOB 6
     /*  Alpha also uses the decmips code variations as well. */
# else
#    define bitsInWord 32
#    define LOB 5
# endif

/* ****************************************************************** 
 *     EXPORT LIST 
 * 
 *  unit vectors ...
 *
 *    extern unsigned long int _eVec[bitsInWord];
 *    extern unsigned long int * _fVec;
 *    extern unsigned long int _fLow[bitsInWord + 1]];
 *
 *  various traps ...
 *
 *    extern long int  _gp_timTrp(long int);
 *    extern void _gp_stackTrp(void);
 *    extern void _gp_storTrp(void);
 *    extern void _gp_soapTrp(void);
 *    extern void _gp_funTrp(void);
 *    extern void _gp_casTrp(long int);
 *    extern void _gp_assTrp(un_chr *, unsigned long int);
 *    extern void _gp_iTrpLHI(long int, long int, long int);
 *    extern void _gp_iTrpLHU(unsigned long, unsigned long, unsigned long);
 *    extern void _gp_rTrpLHI(long int, long int, long int);
 *    extern void _gp_rTrpLHU(unsigned long, unsigned long, unsigned long);
 *    extern void _gp_bndTrp(long int, long int, long int);
 *
 *  various back-compatiblity data ...
 *
 *    extern unsigned long argCount;
 *    extern  char **argPtrs;
 *    extern char * _stackLim;
 *    extern char * _soapTop;
 *    extern char * _soapLim;
 *    char * _getSoap(unsigned long);
 */
/* ****************************************************************** 
 *     INCLUDE FILES
 */
#include <stdio.h>

#ifdef machten
#    include <signal.h>
#endif
# ifndef _APOLLO_SOURCE
#    include <stdlib.h>
# endif

#ifdef iris
#   include <signal.h>
#endif

#ifdef djgpp
#include <go32.h>
#  ifndef linux
#    define linux
#  endif
#endif

/* use -Dos2 -DGPMGUI to get gprtsw.o version */
#ifdef os2
#  define INCL_DOSMODULEMGR
#  define INCL_WINDIALOGS
#  include <os2.h>
#  include <string.h>
#  include <time.h>
#  include <stdlib.h>
#  ifdef GPMGUI
#    include "gprts.h"
#  else
#    include <stdio.h>
#  endif
#  ifndef linux
#    define linux
#  endif
#endif

#ifdef linux    /* defined for djgpp & os2 as well */
#  ifndef i386
#    define i386
#  endif
#endif

#ifdef i386
#   define NATIVE
#endif
#ifdef solaris
#   define NATIVE
#endif
#ifdef decmips
#   define NATIVE
#endif
#ifdef alpha
#   define NATIVE
#   include <siginfo.h>
#endif
#ifdef ibmrs6000
#   define NATIVE
#endif
#ifdef NATIVE
#   include <signal.h>
#   ifdef linux
       extern void stackunwind();
#   else
#      ifdef i386
#         include <ucontext.h>
#         include <sys/siginfo.h>
	  extern void stackunwind();
#      else
#         ifdef decmips
	     extern void stackunwind();
#         else
#            define stackunwind()
#         endif
#      endif
#   endif
#else
#   include <setjmp.h>
#endif

/* ****************************************************************** 
 *      OTHER DEFINITIONS AND TYPEDEFS
 */
#define SOAPDEF   4096
#define STACKDEF 65536

#ifdef KandR_C
  typedef char un_chr;
#else
# ifdef _APOLLO_SOURCE
   typedef char un_chr;
# else
   typedef unsigned char un_chr;
# endif
#endif

typedef un_chr **bundle; /* pointer to array of pointer to char */

/*
     The following declaration and definition of _exPtr is for the
     implementation of coroutines.  The original definition was removed
     when exception handling was removed.

     NOTE: This should be removed when exception handling is added
	   and the coroutine code is updated.
*/

char * _exPtr = 0;


#ifdef NATIVE
unsigned long _soapSize;
unsigned long _stackSize;
unsigned long ProgArgs_bldTime;
#else
unsigned long _soapSize;  /* determined at build time */
unsigned long _stackSize; /* determined at build time */
#endif

/* *************************************************************** 
 * coroutine state vector now defaults to 32 words unless #defined 
 * otherwise below.
 * all versions assume 16 word display vector (built into front end)
 * *************************************************************** */
 
#undef STATESIZE
#ifdef alpha
#define STATESIZE 176
#endif
#ifdef i386
#define STATESIZE 88
#endif
#ifdef ibmrs6000
#define STATESIZE 256
#endif
#ifndef STATESIZE
#define STATESIZE 128
#endif

  typedef struct {
      char *display[16];
      char  state[STATESIZE];
  } StateVec;

  static StateVec costate;

  StateVec *_currentCo = &costate;    /* currently active coroutine */
  char *_soapTop;
  char *_soapLim;
  char *_stackLim;

  unsigned long argCount;
  char **argPtrs;

/*
 * ====================================================== *
 * Now, the message components are defined
 * for the various error messages
 * ====================================================== *
 */

static un_chr arg1[22], arg2[22], arg3[22], arg4[22];
static un_chr gprts[] = "**** gp.rts: ";
static un_chr gpend[] = " ****\n";
#ifdef GPMGUI
static un_chr rngM1[] = "Range error: ";
static un_chr rngDB[] = " not in [";
static un_chr rngDD[] = " .. ";
static un_chr rngSQ[] = "]";
static un_chr casM1[] = "Case selector error: ";
static un_chr idxM1[] = "Index error: ";
static un_chr oflM1[] = "Arithmetic overflow";
static un_chr divM1[] = "Divide by zero error";
static un_chr memM1[] = "Memory error";
static un_chr storM[] = "Storage error";
static un_chr assM1[] = "Assert error";
static un_chr assM2[] = ": <";
static un_chr assM3[] = ">";
static un_chr funM1[] = "Function ended without RETURN";
static un_chr stkM1[] = "Stack limit exceeded";
static un_chr linM1[] = "Error";
static un_chr inMod[] = " in module ";
static un_chr atLin[] = " at line ";
static un_chr nanM1[] = "Real compare with NaN";

#else      /* Console msgs */
static un_chr rngM1[] = "range error: ";
static un_chr rngDB[] = " not in [";
static un_chr rngDD[] = " .. ";
static un_chr rngSQ[] = "]";
static un_chr casM1[] = "case selector error: ";
static un_chr idxM1[] = "index error: ";
static un_chr oflM1[] = "arithmetic overflow";
static un_chr divM1[] = "divide by zero error";
static un_chr memM1[] = "memory error";
static un_chr storM[] = "storage error";
static un_chr assM1[] = "assert error";
static un_chr assM2[] = ": <";
static un_chr assM3[] = ">";
static un_chr funM1[] = "function ended without RETURN";
static un_chr stkM1[] = "stack limit exceeded";
static un_chr linM1[] = "error";
static un_chr inMod[] = " in module ";
static un_chr atLin[] = " at line ";
# ifdef NATIVE
static un_chr nanM1[] = "real compare with NaN";
# endif
#endif
static un_chr busM1[] = "bus error";
static un_chr soap1[] = "out of soap space";

static un_chr *rngMS[] = {rngM1, arg1, rngDB, arg2, rngDD, arg3, rngSQ, 0};
static un_chr *ixtMS[] = {idxM1, arg1, rngDB, arg2, rngDD, arg3, rngSQ, 0};
static un_chr *casMS[] = {casM1, arg1, 0};
static un_chr *oflMS[] = {oflM1, 0};
static un_chr *div0M[] = {divM1, 0};
static un_chr *memMS[] = {memM1, 0};
static un_chr *soapM[] = {soap1, 0};
static un_chr *busMS[] = {busM1, 0};
static un_chr *funMS[] = {funM1, 0};
static un_chr *stkMS[] = {stkM1, 0};
static un_chr *strMS[] = {storM, 0};
static un_chr *assMS[] = {assM1, inMod, 0, atLin, arg4, 0};
static un_chr *linMS[] = {linM1, inMod, 0, atLin, arg4, 0};
#ifdef NATIVE 
static un_chr *oflXS[] = {oflM1, inMod, 0, atLin, arg4, 0};
static un_chr *divXS[] = {divM1, inMod, 0, atLin, arg4, 0};
static un_chr *stkXS[] = {stkM1, inMod, 0, atLin, arg4, 0};
static un_chr *sopXS[] = {soap1, inMod, 0, atLin, arg4, 0};
static un_chr *nanMS[] = {nanM1, inMod, 0, atLin, arg4, 0};
#endif


bundle _exVal[12] = {rngMS, ixtMS, rngMS, casMS, memMS, funMS,
		     rngMS, div0M, rngMS, rngMS, rngMS, ixtMS};

/* unit vectors for the set operations */

unsigned long _eVec[bitsInWord] = {
  0x00000001,   0x00000002,   0x00000004,   0x00000008, 
  0x00000010,   0x00000020,   0x00000040,   0x00000080, 
  0x00000100,   0x00000200,   0x00000400,   0x00000800, 
  0x00001000,   0x00002000,   0x00004000,   0x00008000, 
  0x00010000,   0x00020000,   0x00040000,   0x00080000, 
  0x00100000,   0x00200000,   0x00400000,   0x00800000, 
  0x01000000,   0x02000000,   0x04000000,   0x08000000, 
  0x10000000,   0x20000000,   0x40000000,   0x80000000
# ifdef alpha
  ,0x0000000100000000,0x0000000200000000,0x0000000400000000,0x0000000800000000,
   0x0000001000000000,0x0000002000000000,0x0000004000000000,0x0000008000000000,
   0x0000010000000000,0x0000020000000000,0x0000040000000000,0x0000080000000000,
   0x0000100000000000,0x0000200000000000,0x0000400000000000,0x0000800000000000,
   0x0001000000000000,0x0002000000000000,0x0004000000000000,0x0008000000000000,
   0x0010000000000000,0x0020000000000000,0x0040000000000000,0x0080000000000000,
   0x0100000000000000,0x0200000000000000,0x0400000000000000,0x0800000000000000,
   0x1000000000000000,0x2000000000000000,0x4000000000000000,0x8000000000000000
# endif 
  };

unsigned long _fLow[bitsInWord + 1] = {
  0x00000000,    /* minus 1 element of _fVector */
  0x00000001,   0x00000003,   0x00000007,   0x0000000F, 
  0x0000001F,   0x0000003F,   0x0000007F,   0x000000FF, 
  0x000001FF,   0x000003FF,   0x000007FF,   0x00000FFF, 
  0x00001FFF,   0x00003FFF,   0x00007FFF,   0x0000FFFF, 
  0x0001FFFF,   0x0003FFFF,   0x0007FFFF,   0x000FFFFF, 
  0x001FFFFF,   0x003FFFFF,   0x007FFFFF,   0x00FFFFFF, 
  0x01FFFFFF,   0x03FFFFFF,   0x07FFFFFF,   0x0FFFFFFF, 
  0x1FFFFFFF,   0x3FFFFFFF,   0x7FFFFFFF,   0xFFFFFFFF
# ifdef alpha
  ,0x00000001FFFFFFFF,0x00000003FFFFFFFF,0x00000007FFFFFFFF,0x0000000FFFFFFFFF,
   0x0000001FFFFFFFFF,0x0000003FFFFFFFFF,0x0000007FFFFFFFFF,0x000000FFFFFFFFFF,
   0x000001FFFFFFFFFF,0x000003FFFFFFFFFF,0x000007FFFFFFFFFF,0x00000FFFFFFFFFFF,
   0x00001FFFFFFFFFFF,0x00003FFFFFFFFFFF,0x00007FFFFFFFFFFF,0x0000FFFFFFFFFFFF,
   0x0001FFFFFFFFFFFF,0x0003FFFFFFFFFFFF,0x0007FFFFFFFFFFFF,0x000FFFFFFFFFFFFF,
   0x001FFFFFFFFFFFFF,0x003FFFFFFFFFFFFF,0x007FFFFFFFFFFFFF,0x00FFFFFFFFFFFFFF,
   0x01FFFFFFFFFFFFFF,0x03FFFFFFFFFFFFFF,0x07FFFFFFFFFFFFFF,0x0FFFFFFFFFFFFFFF,
   0x1FFFFFFFFFFFFFFF,0x3FFFFFFFFFFFFFFF,0x7FFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFF
# endif
  };

unsigned long * _fVec = &(_fLow[1]);

#ifdef i386
short _gp_fpucw[3] = {0x0F3F, 0x073F, 0x033F};
#endif

#ifdef NATIVE
double _gp_fpZero = 0.0;
#endif

#ifdef GPMGUI
/* ***************************************************************
 *  bundle handling code
 *  this code is used for all runtime error messages
 *  including those with variable elements
 */
static bundle msgHead = NULL;

static void WriteMsg(bundle b)
{
    msgHead = b;   /* Just remember it for later */
}


/* *************************************************************** */
void _catcher(bundle s)
{
    char    msg[512];
    char   *b[4];
    HMODULE hmod;
    PFNWP   DlgProc;
    int  c  = 0,
         bx = 0;

    if (msgHead != NULL) {
      for (msg[0] = '\0'; *msgHead != NULL; ++msgHead) strcat(msg,*msgHead);
      b[bx++] = msg;
      c = strlen(msg) + 1;
    }
    for (msg[c] = '\0'; *s != NULL; ++s) strcat(&msg[c],*s);
    b[bx++] = &msg[c];
    b[bx++] = "Application will terminate now";
    b[bx]   = NULL;

    if (DosLoadModule(NULL, 0, "GPRTS", &hmod) ||
        DosQueryProcAddr(hmod, 0, "DlgProc", (PFN *)&DlgProc))
      WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
                    "Unable to load error message","GPM Fatal Error",
                    0, MB_ICONHAND | MB_OK);
    else {
      WinAlarm(HWND_DESKTOP, WA_ERROR);
      WinDlgBox(HWND_DESKTOP, HWND_DESKTOP, DlgProc, hmod, GPRTS_DLG, b);
      DosFreeModule(hmod);
    }
    DosExit(EXIT_PROCESS, 0);
}

#else   /* Console version */
/* *************************************************************** */
static void WriteMsg(b)
    bundle b;
{
    fflush(stdout);    
    fputs((char *)gprts, stderr);
    for (; *b != (un_chr *) 0; ++b) 
	fputs((char *)*b,stderr);
    fputs((char *)gpend, stderr);
}

/* *************************************************************** */
void _catcher(s)
   bundle s;
{
    WriteMsg(s);
#if 0    /* djgpp - doesn't work under Win95 (what does?) */
  { int  i;
    char *p = (char *) _go32_info_block.linear_address_of_original_psp;
    p += 0x81;     /* if it starts off with "-d" then assume gdb... */
    for (i = *(p-1); i && *p <= ' '; i--, p++);
    if (*p == '-' && *(p+1) == 'd') __asm__("int $3\n" : : );
  }
#endif
#ifdef NATIVE    
    stackunwind();
#endif
    abort();
}
#endif

/* *************************************************************** */
static void bRecurse(v, n)
   un_chr **v;
   unsigned long n;
{
    if (n >= 10) bRecurse(v,n/10);
    **v = n % 10 + '0';
    ++(*v);
}

/* *************************************************************** */
void _bundleCrd(b, o, n) 
   bundle b;
   unsigned long o, n; /* bundle, offset, number */
{
    un_chr *p = *(b + o);
    bRecurse(&p,n);
    *p = '\0';
}

/* *************************************************************** */
void _bundleInt(b, o, n) 
   bundle b;
   unsigned long o;
   long int n; /* bundle, offset, number */
{
    un_chr *p = *(b + o);
    if (n < 0) {
	*p++ = '-';
	n = -n;
    }
    bRecurse(&p,n);
    *p = '\0';
}

/* *************************************************************** */
int  _gp_timTrp(dummy) 
    int dummy;
{
     return (time(0));
 }

/* *************************************************************** */
/*                        Inline functions                         */
/* *************************************************************** */

void _gp_stackTrp()    { _catcher(stkMS); }
void _gp_storTrp()     { _catcher(strMS); }
void _gp_soapTrp()     { _catcher(soapM); }
void _gp_funTrp()      { _catcher(funMS); }

/* ****************************************************************** */
void _gp_casTrp(val) 
   long int val;
{
    _bundleInt(casMS, 1, val);
    _catcher(casMS);
}

/* *************************************************************** */
void _gp_assTrp(mod, lin)
   un_chr *mod;
   unsigned long lin;
{
    *(assMS + 2) = mod;
    if (lin) 
       _bundleCrd(assMS, 4, lin);
    else {    /* lin == 0 ==> variant form with user message */
       *(assMS + 1) = assM2;
       *(assMS + 3) = assM3;
       *(assMS + 4) = (un_chr *)0;
    }
    _catcher(assMS);
}

#ifdef NATIVE
   /* *************************************************************** *
    *                      NATIVE CODE TRAPS                          *
    * *************************************************************** */

   /* *************************************************************** */
void _gp_XovflowTrp(mod, lin)
      un_chr *mod;
      unsigned long lin;
{
       *(oflXS + 2) = mod;
       _bundleCrd(oflXS, 4, lin);
       _catcher(oflXS);
}

   /* *************************************************************** */
void _gp_Xdiv0Trp(mod, lin)
      un_chr *mod;
      unsigned long lin;
{
       *(divXS + 2) = mod;
       _bundleCrd(divXS, 4, lin);
       _catcher(divXS);
}

   /* *************************************************************** */
void _gp_XstackTrp(mod, lin)
      un_chr *mod;
      unsigned long lin;
{
       *(stkXS + 2) = mod;
       _bundleCrd(stkXS, 4, lin);
       _catcher(stkXS);
}
   
   /* *************************************************************** */
void _gp_XsoapTrp(mod, lin)
      un_chr *mod;
      unsigned long lin;
{
       *(sopXS + 2) = mod;
       _bundleCrd(sopXS, 4, lin);
       _catcher(sopXS);
}

   /* *************************************************************** */
void _gp_NaNTrp(mod, lin)
      un_chr *mod;
      unsigned long lin;
{
       *(nanMS + 2) = mod;
       _bundleCrd(nanMS, 4, lin);
       _catcher(nanMS);
}

/* *************************************************************** */
void _gp_XerrorMsg(mod, lin)
      un_chr *mod;
      unsigned long lin;
{
       *(linMS + 2) = mod;
       _bundleCrd(linMS, 4, lin);
       WriteMsg(linMS);
}

/* *************************************************************** */
/*                end traps for native code versions               */
/* *************************************************************** */
#endif

/* *************************************************************** */
void _gp_iTrpLHI(low, hi, idx) 
   long int low, hi, idx;
{
    _bundleInt(ixtMS,1,idx);
    _bundleInt(ixtMS,3,low);
    _bundleInt(ixtMS,5,hi);
    _catcher(ixtMS);
}

/* *************************************************************** */
void _gp_iTrpLHU(low, hi, idx) 
   unsigned long low, hi, idx;
{
    _bundleCrd(ixtMS,1,idx);
    _bundleCrd(ixtMS,3,low);
    _bundleCrd(ixtMS,5,hi);
    _catcher(ixtMS);
}

/* *************************************************************** */
void _gp_rTrpLHI(min, max, val) 
   long int min, max, val;
{
    _bundleInt(rngMS, 1, val);
    _bundleInt(rngMS, 3, min);
    _bundleInt(rngMS, 5, max);
    _catcher(rngMS);
}

/* *************************************************************** */
void _gp_rTrpLHU(min, max, val) 
   unsigned long min, max, val;
{
    _bundleCrd(rngMS, 1, val);
    _bundleCrd(rngMS, 3, min);
    _bundleCrd(rngMS, 5, max);
    _catcher(rngMS);
}

#ifdef NATIVE
   /* *************************************************************** */
   /* *************************************************************** */
   /*                       Native Code Traps                         */
   /*                Non native equivalents follow                    */
   /* *************************************************************** */
   /* *************************************************************** */

   /* *************************************************************** */
static void memErr(sig) 
      int sig;
{
       signal(SIGSEGV,SIG_DFL);
       WriteMsg(memMS);
#  ifdef i386
       stackunwind();
#  endif
# ifdef linux
#  ifndef os2
	abort();  /* doesn't seem to core dump anyway */
#  endif
# endif 
}

#ifndef i386
   /* *************************************************************** */
static void busErr(sig) 
      int sig;
{
       signal(SIGBUS,SIG_DFL);
       WriteMsg(busMS);
}

#else /* i386 */
#ifndef linux
   /* --------------------------------------------------- *
    * bound parameter block for i386 
    *    .long min, max
    *    .long _gp_xTrp      ;Ptr. to message procedure
    *    .long linenumber
    *    .long _mNam         ;Ptr. to module name string
    * --------------------------------------------------- */
   /* *************************************************************** */
static void boundErr(int sig, siginfo_t *x, ucontext_t *cont)
{
       typedef void (*TRAPPROC)(unsigned, unsigned, unsigned);

       unsigned *pi, reg, min, max, val, line;
       char     *cheip;
       TRAPPROC proc;
       static int regno[8]={11,10,9,8,7,6,5,4};

       if (cont->uc_mcontext.gregs[12] != 5)  /* not a bound trap ... */
	    memErr(SIGSEGV);
       else {
	   signal(SIGSEGV,SIG_DFL);

	   /* Get the instruction pointer */
	   cheip = (char *) cont->uc_mcontext.gregs[14];
	   reg = *(cheip+1); 
	   reg = (reg >> 3) & 7;

	  /* Contents of the bounds register */
	  val = cont->uc_mcontext.gregs[regno[reg]];

	  /* Grab the parameters */
	  pi   = *((unsigned **) (cheip+2));
	  min  = *pi++;
	  max  = *pi++;
	  proc = (TRAPPROC) *pi++;
	  line = *pi++;

	  /* Print message */
	  _gp_XerrorMsg(*((un_chr**) pi), line);
	  (*proc)(min,max,val); 
	  /* This ends up at _catcher and doesn't come back */
       }
}
   /* --------------------------------------------------- */
#endif
#endif

   /* *************************************************************** */
static void oflowErr(sig)
      int sig; 
{
       signal (SIGTRAP,SIG_DFL);
       WriteMsg(oflMS);
#  ifdef i386
       stackunwind();
#  endif
}

extern char ProgArgs_FP_Overflow;
   /* *************************************************************** */
#ifdef alpha
static void divErr(int sig, struct siginfo *info, struct sigcontext *cont) 
#else
static void divErr(sig)
      int sig; 
#endif
   {
#ifndef alpha
       signal(SIGFPE, SIG_DFL);
#endif
#ifdef decmips
       WriteMsg(oflMS);
#else
#  ifdef alpha
	/*
	 *  printf("inst = %x\n",*((unsigned *)cont->sc_pc));
	 */
       if (*((unsigned *)cont->sc_pc)     == 0x63ff0000 ||
	   *((unsigned *)(cont->sc_pc+4)) == 0x63ff0000) { /* trapb */
	 WriteMsg(oflMS);          /* mulqv or cvttqv.. conversions */
	 if (*((unsigned *)cont->sc_pc) == 0x63ff0000)
	   cont->sc_pc -= 4;
	 signal(SIGFPE, SIG_DFL);
	 sigreturn(cont);          /* Should core dump properly now */
       }
       else {                                     /* divide by zero */
	 if (info->si_code == FPE_INTDIV || info->si_code == FPE_FLTDIV) {
	   WriteMsg(div0M);
	   exit(0);       /* use exit as core dump is useless here  */
	 }
	 else if (info->si_code == FPE_INTOVF) {
	   WriteMsg(oflMS);
	   exit(0);
	 }
	 else if (info->si_code == FPE_FLTOVF || info->si_code == FPE_FLTINV) {
	   ProgArgs_FP_Overflow = '\01';	/* INF + X gives this ^^^^^ */
	 }
       } /* the rest are fp errors of some sort which we ignore */
#  else
       WriteMsg(div0M);
#  endif
#endif
#ifdef i386
       stackunwind();
#endif
}

#else

/* *************************************************************** */
/* *************************************************************** */
/*                      Non native code traps                      */
/* *************************************************************** */
/* *************************************************************** */

#  ifdef ANSI_C
#      define version0
#    ifdef _APOLLO_SOURCE
#      undef version0
#      define version1
#    endif
#  else           /* KandR_C */
#    define version1
#  endif

#    ifdef version1

	static void memErr ()
	{
	   signal (11, memErr);
	   *memMS = memM1;
	   _catcher (memMS);
	}

	static void busErr ()
	{
	   signal (10, busErr);
	   *memMS = busM1;
	   _catcher (memMS);
	}

	static void divErr ()
	{
	    signal (8, divErr);
	    _catcher (div0M);
	}

#    endif
#    ifdef version0

	static void memErr (void)
	{
#          ifdef iris
	       signal (11, ((void (*) ())0));
#          else
	       signal (11,SIG_DFL);
#          endif
	   fputs (gprts, stderr);
	   *memMS = memM1;
	   WriteMsg (memMS);
	   fputs (gpend, stderr);
	}
 
	static void busErr (void)
	{
#          ifdef iris
	       signal (10, ((void (*) ())0));
#          else
	       signal (10,SIG_DFL);
#          endif
	   fputs(gprts, stderr);
	   *memMS = busM1;
	   WriteMsg(memMS);
	   fputs(gpend, stderr);
	}

	static void divErr(void)
	{
#          ifdef iris
	       signal (5, ((void (*) ())0));
#          else
	       signal (5,SIG_DFL);
#          endif
	   fputs(gprts, stderr);
	   WriteMsg(div0M);
	   fputs(gpend, stderr);
	}
#    endif
#    undef version1
#    undef version0
#endif

/* *************************************************************** */
char * _getSoap(size)
    unsigned size; 
{
    char *top;

    size = (size + 7) & 0xfffffff8;
    top  = _soapTop;
    if ((top + size) < _soapLim) {
    _soapTop += size;
	return (top);
    }
    else _gp_soapTrp();
}

/* *************************************************************** */
#ifdef NATIVE
static void setSignals(void) 
#else
void setSignals() 
#endif
{
/*
 *  get C to tell us the approximate $sp value ...
 *
 */
       unsigned long autoVariable;
# ifdef i386
#   ifdef linux
       __asm__("fninit\n" : : );
#   else
       static struct sigaction a;
#   endif
# endif
# ifdef alpha
       static struct sigaction a;
# endif

    if (_stackSize == 0) _stackSize = STACKDEF;
#   ifdef NATIVE
	       _stackLim = (char *) & autoVariable - _stackSize;
#   else
#     ifdef KandR_C
#       ifdef Sparc
	       _stackLim = (char *) & autoVariable - _stackSize;
#       else
#         ifdef hp68k
	       _stackLim = (char *) ((long int) & autoVariable - _stackSize);
#         else
#            ifdef sony68k
	       _stackLim = (char *) ((long int) & autoVariable - _stackSize);
#            else
	       _stackLim = (char *) ((long int) & autoVariable + _stackSize);
#            endif
#         endif
#       endif
#     else
#       ifdef _APOLLO_SOURCE
	       _stackLim = (char *) ((long int) & autoVariable - _stackSize);
#       else
	       _stackLim = (char *) & autoVariable - _stackSize;
#       endif
#     endif       
#   endif

    if (_soapSize < SOAPDEF) _soapSize = SOAPDEF;

# ifdef NATIVE
#  ifdef decmips
    _soapTop = (char *) malloc(_soapSize);
    _soapLim = _soapTop + _soapSize - 1;
#  else
    _soapTop = (char *) 0;
    _soapLim = 0;
#  endif
# else
    _soapTop = (char *) malloc(_soapSize);
    _soapLim = _soapTop + _soapSize - 1;
# endif

#   ifdef NATIVE
	signal(SIGTRAP,oflowErr);
#     ifdef alpha
	a.sa_flags   = SA_SIGINFO;
	a.sa_handler = divErr;
	sigaction(SIGFPE,&a,NULL);
#     else
	signal(SIGFPE, divErr);
#     endif
#     ifdef i386
#      ifdef linux
	signal(SIGSEGV,memErr);
#      else
	a.sa_flags   = SA_SIGINFO;
	a.sa_handler = boundErr;
	sigaction(SIGSEGV,&a,NULL);
#      endif
#     else
	signal(SIGBUS, busErr);
	signal(SIGSEGV,memErr);
#     endif
#   else
#     ifndef _APOLLO_SOURCE
	signal(10,busErr);
	signal(11,memErr);
	signal(5,divErr);
#     endif
#   endif
}

#ifdef NATIVE
typedef void (*InitProc)();

#if 0
static un_chr *FPovf[] = {"floating-point overflow occurred during execution",0};
#endif

void _gp_Init(argc, argv, stack, soap, bldTime, pCallChain)
  int argc; char *argv[]; 
  unsigned long stack, soap, bldTime;
  InitProc *pCallChain;
{
#ifdef os2
  _wildcard(&argc,&argv);
#endif
  argCount   = argc;
  argPtrs    = argv;
  _stackSize = stack;
  _soapSize  = soap;
  ProgArgs_bldTime = bldTime;

  setSignals();

  while (*pCallChain != (InitProc)0) {
    (**pCallChain)();
    pCallChain++;
  }

#if 0	/* This generates spurious msgs in the case of REM */
  if (ProgArgs_FP_Overflow) WriteMsg(FPovf);
#endif
#if 0   /* was this here for any good reason? */
  exit(0);
#endif
}

#ifdef i386
/* Dummy entry for 386 convex color backend */
void _gp_Unwind() {}
#endif

#ifdef os2
unsigned long DLLInit_module_handle;

int _CRT_init (void);
void _CRT_term (void);
void __ctordtorInit (void);
void __ctordtorTerm (void);
  
unsigned long _gp_DLLInit(mod_handle, flag, bldTime, pCallChain)
  unsigned long mod_handle, flag, bldTime;
  InitProc *pCallChain;
{
  argCount   = 0;
  _stackSize = 0;
  _soapSize  = 0;
  ProgArgs_bldTime = bldTime;
  DLLInit_module_handle = mod_handle;

  switch (flag) {
    case 0:
      if (_CRT_init () == 0) {
        __ctordtorInit ();
 
        setSignals();

        while (*pCallChain != (InitProc)0) {
          (**pCallChain)();
          pCallChain++;
        }
        return (1);
      }
      return (0);

    case 1:
      __ctordtorTerm ();
      _CRT_term ();
      return (1);
  }
  return (0);
}

MPARAM  MakeMPARAM(unsigned a, unsigned b)
{   return MPFROM2SHORT(a, b);  }

PSZ     MakePSTR(PSZ a)
{   return a;  }

USHORT  MakeSHORT1(unsigned a)
{   return SHORT1FROMMP(a);  }

USHORT  MakeSHORT2(unsigned a)
{   return SHORT2FROMMP(a);  }
#endif

#endif
