#ifndef _logc_H
#define _logc_H
#include <stdarg.h>
#include <stdio.h>
/*	"$Header: /home/commsa/cmdv/cmss/src/RCS/logc.h,v 1.3 2013/11/19 23:13:34 ies Exp $  BoM" */

/* logc.h -- Subroutines to Put text records to Circular file logc.
*
* To write records to the circular file, call:        logc     (text).
*      and preferably initialise a label with:        logc_Open (...)
*
* F77 calls are:  logc, logcopen.
*
* See full documentation in source file:    circtext.c
* This file was generated from master file: circtxxx.h
*
* v1.5  20/11/13 Ian Senior Add logc_r() and logcv_r()
* v1.4  960223 C.Edington, __STDC__ not __stdc__.
* v1.1c 940217 ACE, conditionally compile gets, allow C++ calls.
* v1.1b 940201 ACE, F77 calls.
* v1.1  931025 ACE, initial code.
*/

#define logc_DEFAULT_FILE_PREFIX   "log"
#define logc_DEFAULT_SUBDIR        "."

/* Only define logc_GETS for files with Gets and Puts.*/
/* (i.e. not logs or journals, but yes for alarms, events, etc.) */
/* IMPORTANT:  comment out the define line for logs etc.*/
/*		(The code will then use OpenWriteOnly for endless files).*/
/* Not for logfiles... #define logc_GETS 1 */

#define logc_TEXT_MAX_LENGTH   256
#define logc_LABEL_LENGTH  10

#ifdef __cplusplus
extern "C" {
#endif

//#if defined (__cplusplus) || defined (__STDC__)

int logc      (char *text); 
int logc_r    (char *label,char *text);
int logcv     (const char *text, ...); 
int logcv_r   (char *label,const char *text, ...); 

/* int logc_Open (char *subdir, char *fileprefix, char *label, int printit);
void logc_Close();
 */
//#endif  /* __cplusplus or __stdc__ */

#ifdef __cplusplus
}
#endif

#endif  /* _logc_H */


