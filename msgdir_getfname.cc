//---------------------------------------------------------------------
//
// FUNCTION	msgdir_getfname.cc - determine a file product's true
//					filename for CMSS purposes.
//
// PROTOTYPE IN	msgdir_getfname.h
//
// SUMMARY	char * msgdir_getfname (MSGDIR *dir)
//
//    where
//		dir	  - message directory of product.directory into
//
// RETURN VALUE	 
//		 char * = correct message filename
//
// DESCRIPTION	
// This function looks at the message directory field msgfname.  If the
// field is simply a "?", then it is assumed the true filename is too
// large to be stored in the message directory.  In this instance, the
// message stub is read from msgintxt.dat, and the FILEORGNAME field
// is extracted that label returned.
//
// HISTORY	
//  v1.1  21/10/98  Doug Adams	Original Code,
//  v1.2  24/12/98  Doug Adams	Fix bug (too many open files).
//  v1.3  15/07/99  Ian Senior	Add "FILENAME" as well.
//  v1.4  06/12/00  Ian Senior	Return pointer to static otherwise
//				caller may point to overwritten info.
//				Also removed return of NULL.
//  v1.5  22/12/00  Ian Senior	Fix bug in previous version.
//  v1.6  29/04/02  Ian Senior	Replace ReadMessagex with ReadMessage.
//  v1.7  16/07/03  Ian Senior	Add equivalent msgdir_getfext function
//  v1.8  05/08/03  Ian Senior	Allow for no FILETYPE, only FILENAME
//  v1.10 23/10/09  Ian Senior	Add msgdir_fullfilename
//  v1.11 18/11/13  Ian Senior	Create re-entrant version of these
//				functions.
//  v1.12 06/10/14  Ian Senior	In re-entrant function use ReadMessage_r
//
// $Header: /home/commsa/cmdv/cmss/src/RCS/msgdir_getfname.cc,v 1.12 2014/10/09 00:22:57 ies Exp $
//---------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include "msgdir.h"
#include "tokens.h"
#include "cmss.h"
#include "ReadMessage.h"
#include "msgdir_getfname.h"
#include "makestring.h"
#include "linktime.h"
//static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/msgdir_getfname.cc,v 1.12 2014/10/09 00:22:57 ies Exp $" ;

static char FILEORGNAME[] = "FILEORGNAME=";
static char FILENAME[] = "FILENAME=";
static char FILETYPE[] = "FILETYPE=";

//--------------------------------------------------------------------
//		MSGDIR GETFNAME
//--------------------------------------------------------------------
//
char *msgdir_getfname (MSGDIR *dir)
{
   int   rc, i, len;
   char  *ptr, *filename[2], message[MAX_MSGSIZE+1];
   TOKENS  t;
   static char	return_filename[201];

   if ( strcmp (dir->msgfname, "?") != 0 )
      return (dir->msgfname);

   return_filename[0] = '\0';

   if ( ReadMessage(dir, &len, message,FALSE) < 0 ) {
      return (return_filename);
      }

   tokens_Init(&t);
   ptr = message;
   for (i=0;i<len;i++)
      if (ptr[i] < 32) ptr[i] = ' ';
   ptr[len] = '\0';
   tokens_AddString (&t, ptr);

   filename[1] = NULL;

   rc = tokens_GetThisKeywordsWords (&t, FILEORGNAME, filename, 2);
   if (rc != 2) {
      /* CHECK FOR FILENAME - IF FOUND THEN STRIP OFF .ext TO MAKE
      IT CONSISTENT WITH FILEORGNAME - sigh */
      rc = tokens_GetThisKeywordsWords (&t, FILENAME, filename,2);
      if (rc == 2) {
	 len = strlen(filename[1]);
	 for (i=len-1;i>0 && filename[1][i] != '.';i--);
	 if (filename[1][i] == '.') filename[1][i] = '\0';
	 }
      }
 
   /* RETURN FILENAME IN STATIC MEMORY */
   if (filename[1] != NULL) 
      strcpy(return_filename,filename[1]);

   return (return_filename);
}

//--------------------------------------------------------------------
//		MSGDIR GETFNAME (RE-ENTRANT)
// Re-entrant (threadsafe) version of msgdir_getfname.
//--------------------------------------------------------------------
//
char *msgdir_getfname_r(MSGDIR *dir,char *return_filename)
{
   int   rc, i, len;
   char  *ptr, *filename[2], message[MAX_MSGSIZE+1];
   TOKENS  t;

   if ( strcmp (dir->msgfname, "?") != 0 ) {
      strcpy(return_filename,dir->msgfname);
      return (return_filename);
      }

   return_filename[0] = '\0';

   if ( ReadMessage_r(dir, &len, message,FALSE) < 0 ) {
      return (return_filename);
      }

   tokens_Init(&t);
   ptr = message;
   for (i=0;i<len;i++)
      if (ptr[i] < 32) ptr[i] = ' ';
   ptr[len] = '\0';
   tokens_AddString (&t, ptr);

   filename[1] = NULL;

   rc = tokens_GetThisKeywordsWords (&t, FILEORGNAME, filename, 2);
   if (rc != 2) {
      /* CHECK FOR FILENAME - IF FOUND THEN STRIP OFF .ext TO MAKE
      IT CONSISTENT WITH FILEORGNAME - sigh */
      rc = tokens_GetThisKeywordsWords (&t, FILENAME, filename,2);
      if (rc == 2) {
	 len = strlen(filename[1]);
	 for (i=len-1;i>0 && filename[1][i] != '.';i--);
	 if (filename[1][i] == '.') filename[1][i] = '\0';
	 }
      }
 
   /* RETURN FILENAME IN STATIC MEMORY */
   if (filename[1] != NULL) 
      strcpy(return_filename,filename[1]);

   return(return_filename);
}

//--------------------------------------------------------------------
//			MSGDIR_GETFEXT
// Get the file extension (suffix) for this message.
// (1) If the extension is "?" then it was too long to store in the 
//     msgdir and must be obtained from the raw message.
// (2) Otherwise just return the extension from the msgdir.
//--------------------------------------------------------------------
//
char *msgdir_getfext (MSGDIR *dir)
{
   int   i, rc, len;
   char  *filetype[2], message[MAX_MSGSIZE+1];
   TOKENS  t;
   static char	return_filetype[81];

   /* IF FILETYPE IS IN MSGDIR THEN JUST RETURN IT */
   if (dir->msgfext[0] != '?')
      return(dir->msgfext);

   return_filetype[0] = '\0';

   /* READ MESSAGE TO GET FILETYPE */
   if ( ReadMessage(dir, &len, message,FALSE) < 0 ) {
      return (return_filetype);
      }

   /* STRIP OUT <CR><LF> ETC BECAUSE AFFECT TOKENS FUNCTION */
   for (i=0;i<len;i++)
      if (message[i] < 32) message[i] = ' ';
   message[len] = '\0';

   /* INGEST MESSAGE INTO TOKENS HANDLER */
   tokens_Init(&t);
   message[len] = '\0';
   rc = tokens_AddString (&t, message);

   /* LOOK FOR FILETYPE TOKEN */
   rc = tokens_GetThisKeywordsWords (&t, FILETYPE, filetype, 2);
   if (rc == 2)
      strcpy(return_filetype,filetype[1]);
   else {
      /* EXTRACT FILETYPE FROM FILENAME - AFTER THE DOT */
      rc = tokens_GetThisKeywordsWords (&t, FILENAME, filetype, 2);
      if (rc == 2) {
	 for (i=strlen(filetype[1])-1;i>=0 && filetype[1][i] != '.';i--);
	 if (filetype[1][i] == '.')
	    strcpy(return_filetype,filetype[1]+i+1);
	 }
      }

   return(return_filetype);
}

//--------------------------------------------------------------------
//			MSGDIR_GETFEXT (RE-ENTRANT)
// Get the file extension (suffix) for this message.
// (1) If the extension is "?" then it was too long to store in the 
//     msgdir and must be obtained from the raw message.
// (2) Otherwise just return the extension from the msgdir.
//--------------------------------------------------------------------
//
char *msgdir_getfext_r(MSGDIR *dir, char *return_filetype)
{
   int   i, rc, len;
   char  *filetype[2], message[MAX_MSGSIZE+1];
   TOKENS  t;

   /* IF FILETYPE IS IN MSGDIR THEN JUST RETURN IT */
   if (dir->msgfext[0] != '?') {
      strcpy(return_filetype,dir->msgfext);
      return(dir->msgfext);
      }

   return_filetype[0] = '\0';

   /* READ MESSAGE TO GET FILETYPE */
   if ( ReadMessage_r(dir, &len, message,FALSE) < 0 ) 
      return(return_filetype);

   /* STRIP OUT <CR><LF> ETC BECAUSE AFFECT TOKENS FUNCTION */
   for (i=0;i<len;i++)
      if (message[i] < 32) message[i] = ' ';
   message[len] = '\0';

   /* INGEST MESSAGE INTO TOKENS HANDLER */
   tokens_Init(&t);
   message[len] = '\0';
   rc = tokens_AddString (&t, message);

   /* LOOK FOR FILETYPE TOKEN */
   rc = tokens_GetThisKeywordsWords (&t, FILETYPE, filetype, 2);
   if (rc == 2)
      strcpy(return_filetype,filetype[1]);
   else {
      /* EXTRACT FILETYPE FROM FILENAME - AFTER THE DOT */
      rc = tokens_GetThisKeywordsWords (&t, FILENAME, filetype, 2);
      if (rc == 2) {
	 for (i=strlen(filetype[1])-1;i>=0 && filetype[1][i] != '.';i--);
	 if (filetype[1][i] == '.')
	    strcpy(return_filetype,filetype[1]+i+1);
	 }
      }

   return(return_filetype);
}

/*******************************************************************
*			MSGDIR FULLFILENAME
* Return the full filename. ie include the extension.
*******************************************************************/
char *msgdir_fullfilename(MSGDIR *dir)
{
static char	static_filename[201];
char		*filename;

if (!dir->msgfile) return(NULL);

static_filename[0] = '\0';

filename = msgdir_getfname(dir);
strcpy(static_filename,filename);

filename = msgdir_getfext(dir);
if (filename[0] != '\0') {
   strcat(static_filename,".");
   strcat(static_filename,filename);
   }

return(static_filename);
}

/*******************************************************************
*			MSGDIR FULLFILENAME (RE-ENTRANT)
* Return the full filename. ie include the extension.
*******************************************************************/
char *msgdir_fullfilename_r(MSGDIR *dir,char *full_filename)
{
char	*filename, temp_filename[CM_MAX_FILENAME_LEN+1];

full_filename[0] = '\0';

if (!dir->msgfile) return(NULL);

filename = msgdir_getfname_r(dir,full_filename);

filename = msgdir_getfext_r(dir,temp_filename);
if (filename[0] != '\0') {
   strcat(full_filename,".");
   strcat(full_filename,filename);
   }

return(full_filename);
}
