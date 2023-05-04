static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/ReadTextFile.cc,v 1.7 2014/10/09 00:24:48 ies Exp $" ;
//	"@(#)  ReadTextFile.cc  1.3  93/08/10  CMSS,BoM"
//----------------------------------------------------------------------
// FUNCTION	ReadTextFile.cc - read a message off msgintxt.dat
//
// PROTOTYPE IN	ReadTextFile.h
//
// SUMMARY	rcode = ReadTextFile (int text_index, int offset, int *msglen,
//				       int *format, char *rrrcc, 
//				       char *dtead, char *msg, int flag)
//
//	where	rcode  = (int) return value 
//		text_index = number of text file to be read.
//		offset = byte offset of message entry on text file/
//		msglen = length of message text (returned)
//		format = CMSS format of message text (returned)
//		rrrcc  = circuit identifier (origin of msg, returned)
//		dtead  = x25 SVC dte address of the original sender.  
//		msg    = message text itself (returned)
//		flag   = 1 raise alarms, 0 raise log messages.
//		
// RETURN VALUE	 0 = success,
//		-1 = error occured - a message will be logged "TXT.."
//
// DESCRIPTION	This function is called to read the message text and
//		various pieces of information pertaining to the message
//		off the "msgintxt" file.  Input parameters are the
//		text file number, and the offset that the message is to 
//              be read from.  
//              Two reads are issued on the text file (msgintxtNN.dat)
//		to extract the message, one reads in the message length,
//		message format and circuit identifier into a structure;
//		and the other reads in the message text (once the text
//		length is known from the initial read).  Data is copied
//		from the structure to the parameter areas, and control
//		is passed back to the caller.  If any errors are 
//		detected, an alarm is put on the log and a return code
//		of -1 is returned to the caller.
//
// WARNINGS	Ensure that the offset parameter points to
//		the message length on msgintxt.dat, and not to the text
//		itself.
//
// HISTORY	
//
//  v1.1  09/09/94  Doug Adams	Original code based on ReadMsgText.cc
//  v1.2  06/02/96  Doug Adams    Change sys_errlist to strerror.
//  v1.3  16/02/00  Ian Senior	Fix bug in case where text has been
//				overwritten.
//  v1.4  22/04/02  Ian Senior  Changes to MSGIN text file structure
//  v1.5  14/06/02  Ian Senior  Add ReadTextFile2.
//  v1.6  06/10/14  Ian Senior  Add ReadTextFile_r. (reentrant)
//----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include "log.h"
#include "alarm.h"
#include "cmss.h"
#include "msgintxt.h"
#include "ReadTextFile.h"

static int   OpenTextFile(int index_ntbr);
static void  Alarmx (char *, int);

static int index_msgintxt_dat = -1;
static int fd_text = -1;

//----------------------------------------------------------------------
//			READ TEXT FILE					
//----------------------------------------------------------------------
int ReadTextFile (int index_text, int off, int *len, short *fmt, char *rte,
		   char *daddr, char *msg, int flag)
{
   char       alm [160];
   off_t      seek_code;
   MSGIN_TXT  buffer;

   /* MAKE SURE REQUIRED TEXT FILE IS OPEN */
   if (OpenTextFile(index_text) < 0) return(-1);

   ;// IF OFFSET IS +VE THEN SET THE FILE POINTER TO THE OFFSET
   ;// IF OFFSET IS -VE THEN ASSUME THE FILE POINTER IS ALREADY SET
   if ( off >= 0 ) {
      seek_code = lseek (fd_text,off,SEEK_SET);
      if ( seek_code < 0 ) {
	 sprintf (alm,"TXT000I SEEK SET AT %d FAILED - %s (%d)",
					off, strerror(errno), errno);
	 Alarmx (alm, flag);
	 return (-1);
	 }
      }
   
   ;// READ IN THE MESSAGE LENGTH, MESSAGE FORMAT AND CIRCUIT   
   ;// IDENTIFIER INTO A STRUCTURE - ONLY 11 BYTES ARE READ IN BECAUSE 
   ;// WE DO NOT KNOW THE MESSAGE LENGTH YET.
   if ( read (fd_text,(char *)&buffer,MSGINTXT_INFO_LEN) 
						== MSGINTXT_INFO_LEN ) {
      *len = buffer.msglng;
      *fmt = buffer.format;
      strncpy (daddr, buffer.dtead,sizeof(buffer.dtead)-1);
      daddr[sizeof(buffer.dtead)-1] = '\0';
      strncpy (rte,buffer.rrrcc,5);
      rte[5] = '\0';
      }
   else {
      sprintf (alm,"TXT001I TEXT INFO READ FAILED - %s (%d)",
					strerror(errno), errno);
      Alarmx (alm, flag);
      return (-1);
      }

   ;// CHECK THAT THE MESSAGE LENGTH IS NOT AN ABNORMAL VALUE - 
   ;// THIS COULD HAPPEN IF WRAPAROUNDS OCCUR
   if ( (buffer.msglng < 0) || (buffer.msglng > MAX_MSGSIZE) ) {
      sprintf (alm,"TXT003I TEXT IS TOO OLD AND HAS BEEN OVERWRITTEN");
      Alarmx (alm, flag);
      return (-1);
      }

   ;// READ THE MESSAGE TEXT
   if ( read (fd_text,msg,buffer.msglng) != buffer.msglng ) {
      sprintf (alm,"TXT001I TEXT READ FAILED - %s (%d)",
					strerror(errno), errno);
      Alarmx (alm, flag);
      return (-1);
      }

   return (0);
}

//----------------------------------------------------------------------
//			READ TEXT FILE2					
// Read the text file. 
// In this function the caller knows the length of text required so
// only one "read" is used unlike "ReadTextFile" which uses two.
//----------------------------------------------------------------------
int ReadTextFile2(int index_text, int off, int msglen, int *len, char *msg, int flag)
{
   char       alm [160];
   off_t      seek_code;
   int        readlen;
   MSGIN_TXT  textmsg;

   /* CHECK THE MESSAGE LENGTH */
   if ( (msglen < 0) || (msglen > MAX_MSGSIZE) ) {
      sprintf (alm,"TXT005I TEXT LENGTH ERROR - %d",msglen);
      Alarmx (alm, flag);
      return (-1);
      }

   /* MAKE SURE REQUIRED TEXT FILE IS OPEN */
   if (OpenTextFile(index_text) < 0) return(-1);

   ;// IF OFFSET IS +VE THEN SET THE FILE POINTER TO THE OFFSET
   ;// IF OFFSET IS -VE THEN ASSUME THE FILE POINTER IS ALREADY SET
   if ( off >= 0 ) {
      seek_code = lseek (fd_text,off,SEEK_SET);
      if ( seek_code < 0 ) {
	 sprintf (alm,"TXT000I SEEK SET AT %d FAILED - %s (%d)",
					off, strerror(errno), errno);
	 Alarmx (alm, flag);
	 return (-1);
	 }
      }
   
   /* READ THE HEADER AND MESSAGE */
   readlen = msglen + MSGINTXT_INFO_LEN;
   if ( read (fd_text,(char *)&textmsg,readlen) == readlen ) {
      *len = textmsg.msglng;
      memcpy(msg,textmsg.msg,msglen);
      }
   else {
      sprintf (alm,"TXT001I TEXT READ FAILED - %s (%d)",
					strerror(errno), errno);
      Alarmx (alm, flag);
      return (-1);
      }

   return (0);
}

//----------------------------------------------------------------------
//			READ TEXT FILE (reentrant)
// Read the text file. 
// In this function the caller knows the length of text required so
// only one "read" is used unlike "ReadTextFile" which uses two.
//
// NOTE: Function opens and closes the text file with each read to 
//       make this function reentrant.
//----------------------------------------------------------------------
int ReadTextFile_r(int index_text, int off, int msglen, int *len, char *msg, int flag)
{
   char       alm[160], filename[80];
   off_t      seek_code;
   int        readlen, fd;
   MSGIN_TXT  textmsg;

   /* CHECK THE MESSAGE LENGTH */
   if ( (msglen < 0) || (msglen > MAX_MSGSIZE) ) {
      sprintf (alm,"TXT005I TEXT LENGTH ERROR - %d",msglen);
      Alarmx (alm, flag);
      return (-1);
      }

   /*  OPEN REQUIRED TEXT FILE */
   if (index_text <= 0)
      strcpy(filename,MSGIN_TXT_DAT);
   else
      sprintf(filename,"msgintxt%d.dat",index_text);

   if ( (fd = open(filename,O_RDONLY)) < 0 ) {
      Alarmv("TXT999A MESSAGE TEXT FILE OPEN ERROR - %s",filename);
      Log(strerror(errno));
      return(-11);
      }

   ;// IF OFFSET IS +VE THEN SET THE FILE POINTER TO THE OFFSET
   ;// IF OFFSET IS -VE THEN ASSUME THE FILE POINTER IS ALREADY SET
   if ( off >= 0 ) {
      seek_code = lseek (fd,off,SEEK_SET);
      if ( seek_code < 0 ) {
	 sprintf (alm,"TXT000I SEEK SET AT %d FAILED - %s (%d)",
					off, strerror(errno), errno);
	 Alarmx (alm, flag);
	 close(fd);
	 return (-1);
	 }
      }
   
   /* READ THE HEADER AND MESSAGE */
   readlen = msglen + MSGINTXT_INFO_LEN;
   if ( read (fd,(char *)&textmsg,readlen) == readlen ) {
      *len = textmsg.msglng;
      memcpy(msg,textmsg.msg,msglen);
      }
   else {
      sprintf (alm,"TXT001I TEXT READ FAILED - %s (%d)",
					strerror(errno), errno);
      Alarmx (alm, flag);
      close(fd);
      return (-1);
      }

   close(fd);
   return (0);
}

/**********************************************************************
*                       ALARMX
**********************************************************************/
static void  Alarmx (char *text, int flag)
{
   if ( flag )
      Alarm (text);
   else
      logc (text);
}

/**********************************************************************
*                       OPEN TEXT FILE
* Open the required text file.
* There may be multiple text files and the required one is defined
* by parameter "index_ntbr".
**********************************************************************/
static int OpenTextFile(int index_ntbr)
{
char            filename[80], alm[160];

/* CLOSE THE OPEN DATA FILE IF IT IS THE WRONG ONE */
if (index_msgintxt_dat != index_ntbr && fd_text >= 0) {
   close(fd_text);
   fd_text = -1;
   index_msgintxt_dat = -1;
   }

/* OPEN THE REQUIRED DATA FILE */
if (fd_text < 0) {        
   if (index_ntbr <= 0)
      strcpy(filename,MSGIN_TXT_DAT);
   else
      sprintf(filename,"msgintxt%d.dat",index_ntbr);

   if ( (fd_text = open(filename,O_RDONLY)) < 0 ) {
      sprintf(alm,"TXT999A MESSAGE TEXT FILE OPEN ERROR - %s",filename);
      Alarm(alm);
      Log(strerror(errno));
      return(-11);
      }

   index_msgintxt_dat = index_ntbr;
   }

return(0);
}
