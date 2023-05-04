static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/ReadMsgText.cc,v 1.11 2015/04/22 23:03:43 ies Exp $" ;
//	"@(#)  ReadMsgText.cc  1.3  93/08/10  CMSS,BoM"
//---------------------------------------------------------------------
//  FUNCTION    ReadMsgText.cc - read body of message text off text file
//
//  PROTOTYPE	ReadMsgText.h
//
//  SUMMARY	(int) rc = ReadMsgText (MSGDIR *dir, int *len,
//							char *msg)
//
//	where	rc	= return value 
//		dir	= address of a message directory structure.
//		len	= length of text (returned)
//		msg	= message text itself (returned)
//		
//  RETURNS	 0  = success,
//		-1  = error occured - an TXTnnni alarm will be raised.
//
//  DESCRIPTION	
//  This function is called to read message text itself off the 
//  msgintxt.dat file.  The passed parameter is the address of the
//  message directory.
//  The contents of the message directory determine what part of the 
//  text file is read.  
//  The text is read into a local buffer.
//  No headers or trailers are returned to the caller, the text returned
//  is delimited by the msgstxof and msgtrlen message directory fields.
//
//  WARNINGS	
//  Ensure that the offset parameter points to the message length on 
//  msgintxt.dat, and not to the text itself.
//
//  The returned string is not null terminated and may contain embedded
//  null characters.  To convert the string into a C string, null    
//  terminate the string (the length is returned), and call "makestring"
//  to remove any embedded nulls.
//
//  HISTORY	
//  v1.1  09/08/93  Doug Adams	Original code based on ReadMessage.cc
//  v1.2  10/08/93  Doug Adams	Use msgstxof for non-report messages.
//  v1.3  10/08/93  Doug Adams	"The Final Edit", sequel to the v1.2
//				blockbusting extravaganza.
//  v1.4  07/01/94  Doug Adams	Added dte address parameter to the 
//				ReadTextFile calling sequence.
//  v1.5  29/04/02  Ian Senior	Remove text file from parameter list.
//  v1.6  17/06/02  Ian Senior	Change to ReadTextFile2.
//  v1.7  08/10/13  Ian Senior	Add ReadMsgText2
//  v1.8  06/10/14  Ian Senior	Add ReadMsgText_r (re-entrant)
//  v1.9  17/04/15  Ian Senior	In ReadMsgText_r log errors
//  v1.10 23/04/15  Ian Senior	Distinguish between the TXT003I alarms
//----------------------------------------------------------------------
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include "alarm.h"
#include "log.h"
#include "cmss.h"
#include "msgdir.h"
#include "ReadMsgText.h"
#include "ReadTextFile.h"

//----------------------------------------------------------------------
//			READ MSG TEXT
//----------------------------------------------------------------------
int  ReadMsgText (struct MSGDIR *dir, int *msglen, char *msg)
{
   char       buf[MAX_MSGSIZE];
   int	      msgnum;

   ;// READ THE TOTAL MESSAGE TEXT OFF THE MSGIN TEXT FILE
   if ( ReadTextFile2(dir->index_txt,dir->msgoffst,dir->msglen,msglen,buf,TRUE) < 0 )
      return (-1);

   ;// CHECK FOR A WRAPAROUND OF THE TEXT FILE
   if ( dir->msglen != *msglen ) {
      if (dir->msgrport)
	 msgnum = REPORTS_PREFIX + dir->msgrepno;
      else
	 msgnum = MSGIN_PREFIX + dir->msgdirno;
      Alarmv("TXT003I %d TEXT IS TOO OLD AND HAS BEEN OVERWRITTEN (1)",msgnum);
      return (-1);
      }

   if ( dir->msgrport ) 
      *msglen = dir->msglen - dir->msgstx - dir->msgtrlen;
   else
      *msglen = dir->msglen - dir->msgstxof - dir->msgtrlen;

   ;// CALCULATE LENGTH OF MESSAGE/REPORT TO BE EXTRACTED FROM READ TEXT
   if ( *msglen < 0 ) {
      if ( dir->msgnotxt )
	 *msglen = 0;
      else {
	 Alarmv("TXT004I NEGATIVE LENGTH REPORT DETECTED AT "
		      "OFFSET %d (%X)", dir->msgoffst, dir->msgoffst);
	 return (-1);
	 }
      }

   ;// IF MESSAGE IS A REPORT THEN PREPEND THE REPORT HEADER TO THE TEXT
   if ( dir->msgrport ) {
      strcpy (msg, dir->msgtxthd);
      memcpy (msg + strlen (dir->msgtxthd), buf+dir->msgstx, *msglen);
      *msglen += strlen (dir->msgtxthd);
      }
   else
      memcpy (msg, buf+dir->msgstxof, *msglen);

   return (0);
}

//----------------------------------------------------------------------
//			READ MSG TEXT (re-entrant)
//----------------------------------------------------------------------
int  ReadMsgText_r(struct MSGDIR *dir, int *msglen, char *msg)
{
   char       buf[MAX_MSGSIZE];
   int	      msgnum;

   ;// READ THE TOTAL MESSAGE TEXT OFF THE MSGIN TEXT FILE
   if ( ReadTextFile_r(dir->index_txt,dir->msgoffst,dir->msglen,msglen,buf,TRUE) < 0 )
      return (-1);

   ;// CHECK FOR A WRAPAROUND OF THE TEXT FILE
   if ( dir->msglen != *msglen ) {
      if (dir->msgrport)
	 msgnum = REPORTS_PREFIX + dir->msgrepno;
      else
	 msgnum = MSGIN_PREFIX + dir->msgdirno;
      Logv("TXT003I %d TEXT IS TOO OLD AND HAS BEEN OVERWRITTEN (2)",msgnum);
      return (-1);
      }

   if ( dir->msgrport ) 
      *msglen = dir->msglen - dir->msgstx - dir->msgtrlen;
   else
      *msglen = dir->msglen - dir->msgstxof - dir->msgtrlen;

   ;// CALCULATE LENGTH OF MESSAGE/REPORT TO BE EXTRACTED FROM READ TEXT
   if ( *msglen < 0 ) {
      if ( dir->msgnotxt )
	 *msglen = 0;
      else {
	 Logv("TXT004I NEGATIVE LENGTH REPORT DETECTED AT "
		      "OFFSET %d (%X)", dir->msgoffst, dir->msgoffst);
	 return (-1);
	 }
      }

   ;// IF MESSAGE IS A REPORT THEN PREPEND THE REPORT HEADER TO THE TEXT
   if ( dir->msgrport ) {
      strcpy (msg, dir->msgtxthd);
      memcpy (msg + strlen (dir->msgtxthd), buf+dir->msgstx, *msglen);
      *msglen += strlen (dir->msgtxthd);
      }
   else
      memcpy (msg, buf+dir->msgstxof, *msglen);

   return (0);
}

//----------------------------------------------------------------------
//			READ MSG TEXT2
// Read message text. 
// Alarm errors if alarmit is TRUE;
//----------------------------------------------------------------------
int ReadMsgText2(struct MSGDIR *dir,int *msglen, char *msg,int alarmit)
{
   char       buf[MAX_MSGSIZE];
   int	      msgnum;

   ;// READ THE TOTAL MESSAGE TEXT OFF THE MSGIN TEXT FILE
   if ( ReadTextFile2(dir->index_txt,dir->msgoffst,dir->msglen,msglen,buf,alarmit) < 0 )
      return (-1);

   ;// CHECK FOR A WRAPAROUND OF THE TEXT FILE
   if ( dir->msglen != *msglen ) {
      if (dir->msgrport)
	 msgnum = REPORTS_PREFIX + dir->msgrepno;
      else
	 msgnum = MSGIN_PREFIX + dir->msgdirno;
      if (alarmit)
	 Alarmv("TXT003I %d TEXT IS TOO OLD AND HAS BEEN OVERWRITTEN (3)",msgnum);
      else
	 Logv("TXT003I %d TEXT IS TOO OLD AND HAS BEEN OVERWRITTEN (4)",msgnum);

      return (-1);
      }

   if ( dir->msgrport ) 
      *msglen = dir->msglen - dir->msgstx - dir->msgtrlen;
   else
      *msglen = dir->msglen - dir->msgstxof - dir->msgtrlen;

   ;// CALCULATE LENGTH OF MESSAGE/REPORT TO BE EXTRACTED FROM READ TEXT
   if ( *msglen < 0 ) {
      if ( dir->msgnotxt )
	 *msglen = 0;
      else {
	 if (alarmit)
	    Alarmv("TXT004I NEGATIVE LENGTH REPORT DETECTED AT OFFSET %d (%X)", dir->msgoffst, dir->msgoffst);
	 else
	    Logv("TXT004I NEGATIVE LENGTH REPORT DETECTED AT OFFSET %d (%X)", dir->msgoffst, dir->msgoffst);
	 return (-1);
	 }
      }

   ;// IF MESSAGE IS A REPORT THEN PREPEND THE REPORT HEADER TO THE TEXT
   if ( dir->msgrport ) {
      strcpy (msg, dir->msgtxthd);
      memcpy (msg + strlen (dir->msgtxthd), buf+dir->msgstx, *msglen);
      *msglen += strlen (dir->msgtxthd);
      }
   else
      memcpy (msg, buf+dir->msgstxof, *msglen);

   return (0);
}
