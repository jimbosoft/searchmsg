static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/tracec.c,v 1.6 1998/06/02 05:48:44 cmdv Exp $" ;
/*	tracec -- Clives Ansi-C (non-C++) version of trace.
*	#include tracec.h,  uses logc.
*	FIX:	errno msg lookup ???
*/
/*<S>		trace - message traceback tools. */
/****************************************************************************
*
* FUNCTION	trace -- a toolkit to log traceback messages and numbers.
*
* PROTOTYPE IN	trace.h
*
* SUMMARY	#include "trace.h"
*		and then in your source code,
*		use any of the following Macro calls:
*
*		(n/a yet) TRACE_INIT (nmsgs)		(optional)
*		TRACE_SAVE_NEXT (1)		(dont log the next trace msg)
*		TRACE_PREVIOUS (1)		(log trace SAVED messages)
*		TRACE_PRINT_PREVIOUS (1)	(print SAVED messages)
*
*		TRACE       (number,msg)	(log the msg and traceback)
*		TRACE_ERRNO (errno ,msg)	(append errno reason to msg)
*		TRACE_INT   (number,msg,num2)	(append num2 to msg)
*		TRACE_CHARS (number,msg,msg2)	(append msg2 to msg)
*		TRACE_ERRNO_CHARS (errno,msg,msg2) (append msg2+errtext to msg)
*
*		TRACE_SAVE_xxx  (as above)	(save the msgs internally,
*						to allow the caller the option
*						of logging them with
*						 TRACE_PREVIOUS)
*
* RETURN VALUE	Always returns the number supplied by the caller.
*
* DESCRIPTION	These tools put 2 messages into the system log showing:
*
*		yymmdd hh.mm.ss iiiii Message text
*		yymmdd hh.mm.ss iiiii TRACING n AT LINE x IN filename
*		where:
*		"date/time/iiiii" is supplied by the log routine,
*		"Message text"  is a message supplied by the caller,
*			(with ^ substituted for control characters),
*		"n" is a number (e.g. an error code) supplied by the caller,
*		"line x" is the source code line number where the trace
*			was called, and
*		"filename" is the source code file name.
*
*		Several different variations of the traceback calls can be
*		used to combine extra information within the trace message.
*
*		e.g. TRACE_ERRNO (errno,"I/O error is:")
*			appends 20 chars of the Unix error text for the
*			number errno and would make a final message of (say)
*			"I/O error is: No such file",
*		e.g. TRACE_INT (number,"Error while in step:",stepnum)
*			generates (say) "Error while in step: 3"
*		e.g. TRACE_CHARS (number,"Cannot process file:",filename)
*			generates (say) "Cannot process file: bigfile.dat"
*		e.g. TRACE_ERRNO_CHARS (errno,filename," seek failed: ")
*			generates (say) "freddy.dat seek failed: blah blah"
*
*		In rare cases you may not want to put the trace message onto the
*		log file (e.g. if there was a soft error that was expected),
*		so there is an option to SAVE the trace-messages in memory
*		and (optionally) retreive them later and print them.
*		e.g.  TRACE_SAVE_NEXT (1)
*			TRACE (n,msg)
*			...later the caller can do...
*			TRACE_PREVIOUS (1)  or  TRACE_PRINT_PREVIOUS (1)
*
* WARNINGS	Because the final log msgs are limited to 55 chars,
*		the msg text supplied to these trace tools may have to be
*		truncated in order to append extra int/chars/errno details.
*
* SEE ALSO	log
*
* HISTORY	$Header: /home/commsa/cmdv/cmss/src/RCS/tracec.c,v 1.6 1998/06/02 05:48:44 cmdv Exp $
*		v1.5 960223 ACE, fix unterminated comment in main prog.
*		v1.2 940415 ACE, Ansi-c version adapted from v1.7 trace.cc.
*				 Different errnostring lookup on SunOS.
*		Previous C++ trace.cc was:
*		.v1.7 920831 ACE, Allow up to 80-char user messages.
*		.v1.6 920820 ACE, Upper case the TRACED AT message.
*		.v1.5 920818 ACE, added TraceSaveNext, TraceErrnoChars,
*				 TracePrintPrevious, filter for messages.
*		.v1.4 920814 ACE, Coded SAVE options, add calls to Log.
*		.v1.3 920804 ACE, Fixed errno, etc. (Yet to code SAVE)
*		.v1.0 920731 ACE, Initial code  (adapted from Difacs)
*
***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "tracec.h"
#include "logc.h"

/* Private globals. */
/*  Used to hold the most recent trace_saved message */
static char saved_usermsg  [81]="";
static char saved_tracemsg [81]="";
static int  save_next=0;


/*<F>25
*----------------------------------------------------------------------------
*								Tracec
*----------------------------------------------------------------------------
* User (and internally) called routine to  Trace  number + message.
*/
int Tracec (char *sourcename, int lineno, int logit, int number,const char *msg)
{
    char text[100];
    int i;

    ;/* Filter out control characters from the msg.  Display ^ instead. */
    for (i=0; i<80; i++) {
	if (msg[i]=='\0') { text[i]='\0'; break; }
	if (msg[i]>=32)  text[i] = msg [i];
	else             text[i] = '^';
    }
    text[80] = '\0';

    if (logit && !save_next) logc (text);
    else		strcpy ( saved_usermsg, text);

    TracecBack (sourcename,lineno,logit,number);
    return (number);
}

/*<F>25
*----------------------------------------------------------------------------
*								TracecBack
*----------------------------------------------------------------------------
* Internal routine to  Tracec Back the replynumber, linenumber and
* sourcefilename.
*/
int TracecBack (char *sourcename, int lineno, int logit, int number)
{
    char text[100];

    sprintf (text,"TRACING %i AT LINE %i IN %.40s",
			  number,lineno,sourcename);
    if (logit && !save_next) logc (text);
    else			strcpy (saved_tracemsg, text);
    save_next = 0;
    return (number);
}

/*<F>25
*----------------------------------------------------------------------------
*								TracecErrno
*----------------------------------------------------------------------------
*  Trace  errno number + errno-message.
*/
int TracecErrno (char *sourcename, int lineno, int logit, int number,const char *msg)
{
    char *errnomsg, msg2[81];
    int  errlen, msglen;

    ;/* Try to find the system error text message.*/
    ;/* On ansi-c suns, strerror is supplied.*/
    ;/* but on non-ansi sun/solbourne, we must supply our own strerror */
    ;/* see:  strerror.c */
/*ifdef SunOS
**    if ((number<1) || (number>sys_nerr)) errnomsg = "Unknown errno";
**    else                                 errnomsg = strerror(errno);
**else*/
    errnomsg = strerror (number);
/*endif*/
    if (errnomsg[0]=='\0') errnomsg = "Unknown errno";

    ;/* Make our 80 char err msg from   usrmsg[50]+errnomsg[30] */
    errlen = strlen (errnomsg);
    if (errlen > 30) errlen = 30;
    msglen = strlen (msg);
    if ( (msglen+1+errlen) > 80) msglen = 80 - errlen - 1;
    strncpy (msg2, msg, msglen);
    msg2 [msglen] = '\0';
    strcat (msg2, " ");
    strncat (msg2, errnomsg, (80-strlen(msg2)));
    msg2 [80] = '\0';

    return ( Tracec (sourcename,lineno,logit,number,msg2));
}

/*<F>25
*----------------------------------------------------------------------------
*								TracecInt
*----------------------------------------------------------------------------
* Trace  number + message + number2.
*/
int TracecInt (char *sourcename, int lineno, int logit, int number, const char *msg, int number2)
{
    char msg2[81];

    sprintf (msg2,"%.70s%i",msg,number2);
    return ( Tracec (sourcename,lineno,logit,number,msg2));
}

/*<F>25
*----------------------------------------------------------------------------
*								TracecChars
*----------------------------------------------------------------------------
* Trace  number + message + chars(msg2).
*/
int TracecChars (char *sourcename, int lineno, int logit, int number, const char *msg, const char *msg2)
{
    char msg3[81];

    sprintf (msg3,"%.60s%.20s",msg,msg2);
    return ( Tracec (sourcename,lineno,logit,number,msg3));
}

/*<F>25
*----------------------------------------------------------------------------
*							      TracecErrnoChars
*----------------------------------------------------------------------------
* Trace  errno + message + msg2.
*/
int TracecErrnoChars (char *sourcename, int lineno, int logit, int number, const char *msg, const char *msg2)
{
    char msg3[81];

    sprintf (msg3,"%.60s%.20s",msg,msg2);
    return ( TracecErrno (sourcename,lineno,logit,number,msg3));
}

/*<F>25
*----------------------------------------------------------------------------
*						      TracecErrnoChars2
*----------------------------------------------------------------------------
* Trace  errno + message + msg2 + msg2b.
*/
int TracecErrnoChars2 (char *sourcename, int lineno, int logit, int number,
			const char *msg, const char *msg2, const char *msg2b)
{
    char msg3[81];

    sprintf (msg3,"%.55s%.15s%.10s",msg,msg2,msg2b);
    return ( TracecErrno (sourcename,lineno,logit,number,msg3));
}

/*<F>25
*----------------------------------------------------------------------------
*								TracecInit
*----------------------------------------------------------------------------
* Trace init. * Not implemented yet.
*/
int TracecInit   (const int nmsgs)
{
    int i;
    i = nmsgs;  /* artificial reference to please the compiler */
    return (0);
}

/*<F>25
*----------------------------------------------------------------------------
*								TracecSaveNext
*----------------------------------------------------------------------------
* Trace Save next message (do not log it, but memorise it instead)
* Set a flag to force the next trace message pair to be saved instead of
* logged.
* The message will be retrieved later and printed.
* This routine is called to avoid recursive logging of errors on the log file
* itself.
*/
int TracecSaveNext   (const int nmsgs)
{
    int i;
    save_next = 1;
    i = nmsgs;  /* artificial reference to please the compiler */
    return (0);
}

/*<F>25
*----------------------------------------------------------------------------
*								TracecPrevious
*----------------------------------------------------------------------------
* Retrieve and log previously saved Tracec messages.
*/
int TracecPrevious (const int nmsgs)
{
    int i;
    i = nmsgs;  /* artificial reference to please the compiler */
    ;/* Only implemented for 1 message. */
    logc (saved_usermsg);
    logc (saved_tracemsg);
    return (0);
}

/*<F>25
*----------------------------------------------------------------------------
*							TracecPrintPrevious
*----------------------------------------------------------------------------
* This is used by the logfile routine when it wants to print out
* CirFile errors that occurred on the logfile itself.
*/
int TracecPrintPrevious (const int nmsgs)
{
    int i;
    i = nmsgs;  /* artificial reference to please the compiler */
    ;/* Only implemented for 1 message. */
    printf ("%s\n",saved_usermsg);
    printf ("%s\n",saved_tracemsg);
    return (0);
}

/*<F><S> Test program.
*----------------------------------------------------------------------------
*				(conditional) main program to test Tracec
*----------------------------------------------------------------------------
*
*       main -- test program for 'trace'    (conditionally compiled)
*/
/* #define TEST_TRACE  */
#if defined TEST_TRACE

#include "circfile.h"

/* Declare dummy subroutines */
#define NoParams void
int sub1 (void);
int sub2 (void);
int sub3 (void);
int sub4 (void);
int main (void);

int main (void)
{
    int errorno;
    int reply, nmessages, recsize, printit;
    char *msg="This is a test msg.";
    char *testlogfilename = "traceclf";
    CIRCFILE logcf;

    printf ("\nTESTING 'TRACEC' onto a local logfile '%s'\n",testlogfilename);

    ;/* First create and open a local log file just for our use.*/
    nmessages = 100;
    recsize = 100;
    circfile_Init (&logcf);
    reply = circfile_Create (&logcf, testlogfilename, nmessages, recsize);
    ;/* was... reply = LogcCreate (testlogfilename, nmessages);*/
    if (reply) printf (" LogcCreate bad reply: %i\n",reply);
    printit = 1;

    reply = logc_Open ("",testlogfilename,"tracetst",printit);
    if (reply) printf (" LogcOpen bad reply: %i\n",reply);

    reply = logc ("First log message");
    if (reply) printf (" Logc bad reply: %i\n",reply);


    ;/* Not applicable yet....TRACE_INIT (1);*/

    ;/* Test nested Traces.  (Look at the results on the log file) */
    errorno = sub1 ();
    TRACE (errorno,"from sub1");
    errorno = sub3 ();

    ;/* Test all of the various formats of the Trace calls.*/
    ;/* (do a bit of sleeping too so that the timestamp increments).*/
    TRACE (errorno, msg);
    TRACE (errorno,"12345678901234567890123456789012345678901234567890123456789012345678901234567890");
    TRACE_ERRNO (2,"System errno text is:");
    sleep (1);
    TRACE_INT   (-1,"Error while in state",2);
    sleep (1);
    TRACE_CHARS (-2,"Cannot lock file:","freddir.ptr");
    sleep (1);

    ;/* Now save a msg, then retrieve it.*/
    TRACE_SAVE_NEXT (1);
    TRACE_ERRNO_CHARS2 (1,"saved filename"," with extra chars1, ","chars2");
    printf ("...a saved message should follow...\n");
    TRACE_PREVIOUS (1);

    ;/* Check that none of these write to the log file yet.*/
    TRACE_SAVE (errorno,"12345678901234567890123456789012345678901234567890123456789012345678901234567890");
    TRACE_SAVE_ERRNO (9,"System errno text is:");
    TRACE_SAVE_INT   (-9,"Error while in state",92);
    TRACE_SAVE_CHARS (-92,"Cannot lock file:","freddir.ptr");
    TRACE_SAVE_ERRNO_CHARS (9,"System errno text"," is:");
    TRACE_SAVE_ERRNO_CHARS2 (9,"System errno text"," is",": ");

    ;/* Now retrieve the last one.*/
    TRACE_SAVE_CHARS (-1,"This was a saved message:"," with extra chars");
    TRACE_PREVIOUS (1);

    printf ("...and this repeat msg should only be printed, not logged....\n");
    TRACE_PRINT_PREVIOUS (1);
}


int sub1 (NoParams)   /* dummy subroutine */
{
int errorno;
errorno = sub2 ();
if (errorno)  return (TRACE (errorno,"Within sub1, from sub2"));
}

int sub2 (NoParams)   /* dummy subroutine */
{
int n;
n=-99;
if (n!=10)  return (TRACE (-5,"Within sub2"));
}

int sub3 (NoParams) /* dummy subroutine */
{
int errorno;
errorno = sub4 ();
if (errorno) return (TRACE (errorno,"Within sub3, from sub4"));
}

int sub4 (NoParams) /* dummy subroutine */
{
int n;
n = 99;
if (n != 10) return (TRACE(-6, "TEST_MESSAGE Within sub4"));
}

#endif
