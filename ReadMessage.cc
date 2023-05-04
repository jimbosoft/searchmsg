static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/ReadMessage.cc,v 1.8 2014/10/09 00:23:41 ies Exp $" ;
//---------------------------------------------------------------------
//  FUNCTION    ReadMessage.cc - read a message/report off msgintxt.dat
//
//  PROTOTYPE	ReadMessage.h
//
//  SUMMARY	(int) rc = ReadMessage(MSGDIR *dir, int *len, char *msg,
//						int flag)
//
//	where	rc	= return value 
//		dir	= address of a message directory structure.
//		len	= length of returned text
//		msg	= message text itself (returned)
//		flag    = TRUE if raise Alarm. FALSE otherwise.
//		
//  RETURNS	 0  = success,
//		-1  = error occured - an TXTnnni alarm will be raised.
//
//  DESCRIPTION	
//  This function is called to read either a message report or the 
//  message text itself off the msgintxt.dat file.  
//  The contents of the message directory determine what part of the 
//  text file is read.  
//  The text is read into a local buffer.
//  If the msgdir.msgrport flag is set, then the returned text is a   
//  report from the message.  Reports consist of a substring of the 
//  message delimited by the dir.msgstxof and dir.msgtrlen values -
//  this is copied from the local buffer into the returned buffer and
//  the length of the report is determined and returned as well.
//  If the report flag is not set, then the text and message length is
//  returned.
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
//  v1.1  08/09/94  Doug Adams	Original code based on ReadMessage.cc
//  v1.2  29/04/02  Ian Senior  Revert to ReadMessage
//  v1.3  14/06/02  Ian Senior  Include msgnum in alarm and change call
//				to ReadTextFile2.
//  v1.4  06/10/14  Ian Senior  Add ReadMessage_r() (reentrant)
//----------------------------------------------------------------------
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include "log.h"
#include "alarm.h"
#include "cmss.h"
#include "msgdir.h"
#include "ReadMessage.h"
#include "ReadTextFile.h"

static void  Alarmx (char *, int);

//----------------------------------------------------------------------
//							   ReadMessage
//----------------------------------------------------------------------
int  ReadMessage(struct MSGDIR *dir, int *msglen, char *msg, int flag)
{
   int	      msgnum;
   char       alm[160], buf[MAX_MSGSIZE];

   if ( ReadTextFile2(dir->index_txt,dir->msgoffst, dir->msglen, msglen,
		        buf, flag ) < 0 )
      return (-1);

   ;// CHECK THAT THE MESSAGE LENGTH IS NOT AN ABNORMAL VALUE - 
   ;// THIS COULD HAPPEN IF WRAPAROUNDS OCCUR
   if ( dir->msglen != *msglen ) {
      if (dir->msgrport)
	 msgnum = REPORTS_PREFIX + dir->msgrepno;
      else
	 msgnum = MSGIN_PREFIX + dir->msgdirno;
      sprintf (alm,"TXT003I %d TEXT IS TOO OLD AND HAS BEEN OVERWRITTEN",msgnum);
      Alarmx (alm, flag);
      return (-1);
      }

   ;// IF THE RECOGNISER FLAG IS SET THEN INVOKE REPORT PROCESSING
   if ( dir->msgrport ) {
      *msglen = dir->msglen - dir->msgstx - dir->msgtrlen;
      if ( *msglen < 0 ) {
	 sprintf (alm,"TXT006I NEGATIVE LENGTH REPORT DETECTED AT "
		      "OFFSET %d (%X)", dir->msgoffst, dir->msgoffst);
	 Alarmx (alm, flag);
	 return (-1);
	 }
      strcpy (msg, dir->msgtxthd);
      memcpy (msg+strlen(msg), buf+dir->msgstx, *msglen);
      *msglen += strlen (dir->msgtxthd);
      }
   else 
      memcpy (msg, buf, *msglen);

   return (0);
}

//----------------------------------------------------------------------
//		   ReadMessage_r (reentrant)
//----------------------------------------------------------------------
int  ReadMessage_r(struct MSGDIR *dir, int *msglen, char *msg, int flag)
{
   int	      msgnum;
   char       alm[160], buf[MAX_MSGSIZE];

   if ( ReadTextFile_r(dir->index_txt,dir->msgoffst,dir->msglen,msglen,
		        buf, flag ) < 0 )
      return (-1);

   ;// CHECK THAT THE MESSAGE LENGTH IS NOT AN ABNORMAL VALUE - 
   ;// THIS COULD HAPPEN IF WRAPAROUNDS OCCUR
   if ( dir->msglen != *msglen ) {
      if (dir->msgrport)
	 msgnum = REPORTS_PREFIX + dir->msgrepno;
      else
	 msgnum = MSGIN_PREFIX + dir->msgdirno;
      sprintf (alm,"TXT003I %d TEXT IS TOO OLD AND HAS BEEN OVERWRITTEN",msgnum);
      Alarmx (alm, flag);
      return (-1);
      }

   ;// IF THE RECOGNISER FLAG IS SET THEN INVOKE REPORT PROCESSING
   if ( dir->msgrport ) {
      *msglen = dir->msglen - dir->msgstx - dir->msgtrlen;
      if ( *msglen < 0 ) {
	 sprintf (alm,"TXT006I NEGATIVE LENGTH REPORT DETECTED AT "
		      "OFFSET %d (%X)", dir->msgoffst, dir->msgoffst);
	 Alarmx (alm, flag);
	 return (-1);
	 }
      strcpy (msg, dir->msgtxthd);
      memcpy (msg+strlen(msg), buf+dir->msgstx, *msglen);
      *msglen += strlen (dir->msgtxthd);
      }
   else 
      memcpy (msg, buf, *msglen);

   return (0);
}

static void  Alarmx (char *text, int flag)
{
   if ( flag ) 
      Alarm (text);
   else
      logc (text);
}
