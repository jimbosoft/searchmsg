static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/find_logmsg.cc,v 1.1 2009/02/16 01:06:43 ies Exp $" ;
/*********************************************************************
*			FIND LOG MESSAGE 
*
* SUMMARY
*	find_logmsg(int startno, time_t end_time, char *ident, 
*					char *msg, CirFile  *cirfile)
*		startno = starting record number
*		end_time = maximum record time
*		ident = record identifier to be searched for
*		cirfile = circular file to be searched.
*       RETURN CODE:
*		-1 = error
*		 1 = found
*		 0 = not found
*		
* FUNCTION	
* To find the log record that contains the required string (ident)
*
* v1.1 28/05/98  Ian Senior     Based on code from xlogext
* v1.2 27/11/00  C.Edington	Fixed for expanding-circular-files.
* v1.3 30/01/03  Ian Senior	Allow multiple log msgs to be 
*				slightly separated.
* v1.4 15/11/04  Ian Senior	Increase max log messages returned.
*
* $Header: /home/commsa/cmdv/cmss/src/RCS/find_logmsg.cc,v 1.1 2009/02/16 01:06:43 ies Exp $
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "cmss.h"
#include "msgdir.h"
#include "cirfile.h"
#include "log.h"
#include "getcommstime.h"
#include "cmmktime.h"
#include "getgroup.h"
#include "find_logmsg.h"
#include "log_maketime.h"

#define MAX_RECORD_LENGTH  512
#define MAX_LOGMSGS	10

static int time_okay(char *logmsg, time_t end_time);
static void trimstring(char *str);
static void CheckMoreRecs(char *ident,char *msg,int recno,CirFile *cirfile,int recordlength);
static int ReadLogRecord(char *linebuffer,int recno,CirFile *cirfile,int recordlength);

/**********************************************************************
*                       FIND LOG MSG
* Starting from "startno" search for a record that contains the 
* required identifier (ident). Search until the record is found or
* passsed the latest record or until the end time is reached.
**********************************************************************/
int find_logmsg(int startno, time_t end_time, char *ident, char *msg, CirFile  *cirfile)
{
char	linebuffer[MAX_RECORD_LENGTH+1];
int	rc, recno, ntbw, recordlength, match = FALSE;
int	count = 0;

recno = startno;

msg[0] = '\0';
recordlength = cirfile->qRecordSize();
ntbw = cirfile->qNtbw();

/* READ RECORD */
rc = ReadLogRecord(linebuffer,recno,cirfile,recordlength);
if (rc != 0) return(-1);

/* SEARCH FILE */
while (!match && time_okay(linebuffer,end_time) && recno != ntbw) {

   /* CHECK IF RECORD MATCHES CRITERIA */
   if (strstr(linebuffer,ident) != NULL) {
      match = TRUE;
      trimstring(linebuffer);
      strcpy(msg,linebuffer);
      }
   else {
      match = FALSE;
      recno++;
      if (recno >= cirfile->qMaxRecords())
         recno = cirfile->CorrectRecordNo (recno);
      count++;

      /* READ RECORD */
      rc = ReadLogRecord(linebuffer,recno,cirfile,recordlength);
      if (rc != 0) return(-1);
      }

   }

/* CHECK FOR ADDITIONAL (CONSECUTIVE MATCHES) */
if (match) CheckMoreRecs(ident, msg, recno, cirfile, recordlength);

return(match);
}

/**********************************************************************
*                       CHECK MORE RECS
* It is possible that there could be multiple Log records for 
* this message. Assume they are close to the first record.
**********************************************************************/
static void CheckMoreRecs(char *ident,char *msg,int recno,CirFile *cirfile,int recordlength)
{
int	matchcount = 0;
int	count = 1;
char    linebuffer[MAX_RECORD_LENGTH+1];

#define MAX_SEARCH	100

do {
  linebuffer[0] = '\0';
  recno = cirfile->CorrectRecordNo (recno+1);
  count++;
  ReadLogRecord(linebuffer,recno,cirfile,recordlength);
  if (strstr(linebuffer,ident) != NULL) {
     trimstring(linebuffer);
     strcat(msg,"\n");
     strcat(msg,linebuffer);
     matchcount++;
     }
} while (count < MAX_SEARCH && matchcount < MAX_LOGMSGS);

}

/**********************************************************************
*                       READ LOG RECORD
**********************************************************************/
static int ReadLogRecord(char *linebuffer,int recno,CirFile *cirfile,int recordlength)
{
int	rc;

rc = cirfile->LookRecord (linebuffer, recno);
if (rc != 0) {
   Log("FIND LOGMSG: ERROR READING LOG FILE");
   return(-1);
   } 
linebuffer[recordlength-1] = ' ';
linebuffer[recordlength]   = '\0';
return(0);
}

/**********************************************************************
*                       TIME OKAY   
* Check that the time in the log message (string) is less than the
* end time.
**********************************************************************/
static int time_okay(char *logmsg, time_t end_time)
{
time_t	log_time;
int	rc;

log_time = log_maketime(logmsg);

if (difftime(log_time,end_time) > 0)
   rc = FALSE;
else
   rc = TRUE;

return(rc);
}

/**********************************************************************
*                       TRIM STRING 
* Trim trailing spaces from the string. 
**********************************************************************/
static void trimstring(char *str)
{
int	len, i;

len = strlen(str);
for (i=len-1;i>0 && str[i] == ' ';i--);

str[i+1] = '\0';
}

/**********************************************************************
*                       TEST MAINLINE
**********************************************************************/
#ifdef _TEST_MAINLINE_FIND_LOGMSG
main(int argc, char *argv[])
{
static CirFile  cirfile;
time_t		test_time;
int		rc,startno;
char		msg[200];

chdir(argv[1]);

rc = cirfile.OpenLooker(argv[2]); 
if (rc != 0) {
   printf("woops\n");
   return(1);
   }

time(&test_time);

startno = atoi(argv[3]);

rc = find_logmsg(startno, test_time, argv[4], msg, &cirfile);
printf("rc = %d msg = %s\n",rc,msg);

}
#endif
