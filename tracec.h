#ifndef _TRACEC_H
#define _TRACEC_H
/* Clives "C" version of TRACE */

/* "$Header: /home/commsa/cmdv/cmss/src/RCS/tracec.h,v 1.2 1993/12/23 04:04:38 ace Exp $  CMSS,BoM" */

/*  trace.h -- Definitions for Traceback tools, for both SUN  C and C++.
*		They generate 2 log messages:
*		(1)   user message text
*		(2)   TRACING number AT LINE nnn IN sourcefilename
*
*  Example usage is:    if (errorno)  return (TRACE(errorno,"BAD X REPLY"));
*
*  v1.5 931123 ACE, converted to ansi-c. added TRACE_ERRNO_CHARS2.
*  v1.4 920818 ACE, add TRACE_SAVE_NEXT, TRACE_ERRNO_CHARS, PRINT_PREVIOUS
*  v1.3 920814 ACE, cosmetic changes.
*  v1.1 920731 ACE, Initial code (adapted from Difacs code).
*/

/* Macros to (1) store up to n previous msgs, (2) recall & log previous msgs.*/
/* Previous msgs are those stored by  TRACE_SAVE_xxx().*/

#define TRACE_INIT(nmsgs)     TracecInit(nmsgs)
#define TRACE_SAVE_NEXT(nmsgs) TracecSaveNext(nmsgs)
#define TRACE_PREVIOUS(nmsgs) TracecPrevious(nmsgs)
#define TRACE_PRINT_PREVIOUS(nmsgs) TracecPrintPrevious(nmsgs)

/* Macros to include traceback source filename and line number.*/

#define TRACE(number,msg)                 Tracec(__FILE__,__LINE__,1,number, msg)
#define TRACE_ERRNO(number,msg)      TracecErrno(__FILE__,__LINE__,1,number, msg)
#define TRACE_INT(number,msg,number2)  TracecInt(__FILE__,__LINE__,1,number,msg,number2)
#define TRACE_CHARS(number,msg,msg2) TracecChars(__FILE__,__LINE__,1,number,msg,msg2)
#define TRACE_ERRNO_CHARS(number,msg,msg2) TracecErrnoChars(__FILE__,__LINE__,1,number,msg,msg2)
#define TRACE_ERRNO_CHARS2(number,msg,msg2,msg2b) TracecErrnoChars2(__FILE__,__LINE__,1,number,msg,msg2,msg2b)

#define TRACE_SAVE(number,msg)            Tracec(__FILE__,__LINE__,0,number,msg)
#define TRACE_SAVE_ERRNO(number,msg) TracecErrno(__FILE__,__LINE__,0,number, msg)
#define TRACE_SAVE_INT(number,msg,number2)  TracecInt(__FILE__,__LINE__,0,number,msg,number2)
#define TRACE_SAVE_CHARS(number,msg,msg2) TracecChars(__FILE__,__LINE__,0,number,msg,msg2)
#define TRACE_SAVE_ERRNO_CHARS(number,msg,msg2) TracecErrnoChars(__FILE__,__LINE__,0,number,msg,msg2)
#define TRACE_SAVE_ERRNO_CHARS2(number,msg,msg2,msg2b) TracecErrnoChars2(__FILE__,__LINE__,0,number,msg,msg2,msg2b)

/* These are the routines that are called to log the trace messages.*/
#ifdef __cplusplus
extern "C" {
#endif

    int TracecInit     (const int nmsgs);
    int TracecSaveNext (const int nmsgs);
    int TracecPrevious (const int nmsgs);
    int TracecPrintPrevious (const int nmsgs);
    int TracecBack   (char *sourcename, int lineno, int logit, int number);
    int Tracec       (char *sourcename, int lineno, int logit, int number,
			const char *msg);
    int TracecErrno (char *sourcename, int lineno, int logit, int number,
			const char *msg);
    int TracecInt   (char *sourcename, int lineno, int logit, int number,
			const char *msg, int number2);
    int TracecChars (char *sourcename, int lineno, int logit, int number,
			const char *msg, const char *msg2);
    int TracecErrnoChars (char *sourcename, int lineno, int logit, int number,
			const char *msg, const char *msg2);
    int TracecErrnoChars2 (char *sourcename, int lineno, int logit, int number,
			const char *msg, const char *msg2, const char *msg2b);
#ifdef __cplusplus
}
#endif
#endif  /* _TRACEC_H */


