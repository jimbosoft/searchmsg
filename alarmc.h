#ifndef _alarmc_H
#define _alarmc_H
#include <stdarg.h>
#include <stdio.h>
/*	"$Header: /home/commsa/cmdv/cmss/src/RCS/alarmc.h,v 1.5 2004/10/12 00:29:31 ies Exp $  BoM"
*
* alarmc.h -- Subroutines to Put text records to Circular file alarmc.
*	   "alarm" also puts a copy of the msg onto the log file.
*
* To write records to the circular file, call:        alarmc     (text).
*      and preferably initialise a label with:        alarmc_Open (...)
*
* F77 calls are:  alarmc, alarmcopen.
*
* See full documentation in source file:    circtext.c
* This file was generated from master file: circtxxx.h
*
* v1.4  960223 C.Edington, __STDC__ not __stdc__.
* v1.3  941102 C.Edington, Fix bad comment.
* v1.1c 940217 ACE, conditionally compile gets, allow C++ calls.
* v1.1b 940201 ACE, F77 calls.
* v1.1  931025 ACE, initial code.
*/

#define alarmc_DEFAULT_FILE_PREFIX   "alarm"
#define alarmc_DEFAULT_SUBDIR        "."

/* Only define alarmc_GETS for files with Gets and Puts.*/
/* (i.e. not logs or journals, but yes for alarms, events, etc.) */
/* IMPORTANT:  comment out the define line for logs etc.*/
/*		(The code will then use OpenWriteOnly for endless files).*/
#define alarmc_GETS 1

#define alarmc_TEXT_MAX_LENGTH   256
#define alarmc_LABEL_LENGTH  10

#ifdef __cplusplus
extern "C" {
#endif

#if defined (__cplusplus) || defined (__STDC__)

int alarmc (char *text);
int alarmcv (const char *text, ...);
/*int alarmc_Open (char *subdir, char *fileprefix, char *label, int printit);*/

/* Only define these GET routines if you need them.*/
/* e.g. not needed for log files.*/
#ifdef alarmc_GETS
int alarmc_Get  (char *text, int maxlength);
int alarmc_OpenGet (char *subdir, char *fileprefix, int printit);
#endif

#endif  /* __cplusplus or __stdc__ */

#ifdef __cplusplus
}
#endif

#endif  /* _alarmc_H */


