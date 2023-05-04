//---------------------------------------------------------------------
// 				CMSS PARAM
// FUNCTION:	Check the "cmss.params" file for a given parameter.
//		Return the result string.
// 
//  SUMMARY	rc = cmssparam (char *param, char *result)
//
//		Used for parameter file "cmss.params"
//
//		rc = cmssparamg (char *fname, char *param, char *result)
//
//		Used for a specific parameter file (user supplies file
//		name).
// 
//		rc:
//		0  = "param" found. "result" is set.
// 		-1 = "param" not found.
// 
//  DESCRIPTION
//  This function simply opens the CMSS general parameter file and
//  looks for the "param"=.... parameter in the file.  If the parameter
//  is found then a return the result value.
//
//  This function assumes the directory the calling process is 
//  executing in contains the CMSSPARMS file.
//
//  HISTORY 
//  v1.0  17/03/95  Ian Senior	Based on "aifs_system.cc" function.
//  v1.1  21/03/95  Doug Adams  Allow different parameter files to be
//			 	called.
//  v1.2  18/03/97  Ian Senior	Read tokens file once only for CMSSPARMS
//  v1.3  30/06/00  Ian Senior	Fix bug if token exists but no
//				value (eg TOKEN = )
//  v1.7  19/10/00  Ian Senior	Add cmssparamgn function.
//  v1.8  14/07/03  Ian Senior	Move cmss.params into etc dir but
//				also check for old location.
//  v1.9  18/11/03  Ian Senior	Change cmssparam back so that always
//				reads file. Add cmssparam2 that reads
//				once
//  v1.10 02/02/05  Ian Senior	Add cmssparamn
//  v1.11 28/11/05  Ian Senior	Add cmssparamgt
//  v1.12 08/05/08  Ian Senior	Add cmssparamString
//  v1.13 16/07/08  Ian Senior	Remove linktime.h
//  v1.14 30/07/20  Ian Senior	Add cmssparam_GetParms, cmssparam_ReadTokens
// $Header: /home/commsa/cmdv/cmss/src/RCS/cmssparam.cc,v 1.14 2020/08/03 05:07:11 ies Exp $
//---------------------------------------------------------------------
#include  <stdio.h>
#include  <unistd.h>
#include  <fcntl.h>
#include  <string.h>
#include  <stdlib.h>
#include  <sys/types.h>
#include  <sys/stat.h>

#include  "cmss.h"
#include  "tokens.h"
#include  "cmssparam.h"
#include  "log.h"

static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/cmssparam.cc,v 1.14 2020/08/03 05:07:11 ies Exp $" ;

static int ReadTokens(char *fname, TOKENS *t);
static int  GetParm (TOKENS *, char *, char *);
static int  GetParms(TOKENS *t, char *parm, char values[][80], int max);

//---------------------------------------------------------------------
//							      cmssparam
//---------------------------------------------------------------------
// Get a parameter off the file "cmss.params".
//
int  cmssparam (char *param, char *result)
{
   TOKENS   	t;
   int	 	rc;

   rc = ReadTokens(CMSSPARMS, &t);
   if (rc != 0) {
      rc = ReadTokens(CMSSPARMS_OLD, &t);
      if (rc != 0) return(rc);
      }

   /* GET THE PARAMETER */
   rc = GetParm (&t, param, result);

   return (rc);
}

//---------------------------------------------------------------------
//							     cmssparamn
//---------------------------------------------------------------------
// Get a parameter off the file "cmss.params".
//
int  cmssparamn (const char *param, char values[][80], int max)
{
   TOKENS   	t;
   int	 	rc;

   rc = ReadTokens(CMSSPARMS, &t);
   if (rc != 0) {
      rc = ReadTokens(CMSSPARMS_OLD, &t);
      if (rc != 0) return(rc);
      }

   /* GET THE PARAMETERS */
   rc = GetParms(&t, (char *)param, values, max);

   return (rc);
}

//---------------------------------------------------------------------
//							      cmssparam
//---------------------------------------------------------------------
// Get a parameter off the file "cmss.params".
// Only read this file once (ie define as static !!).
//
int  cmssparam2 (char *param, char *result)
{
   static TOKENS   	t;
   static int		have_tokens = FALSE;
   int	 		rc;

   if (!have_tokens) {
      rc = ReadTokens(CMSSPARMS, &t);
      if (rc != 0) {
	 rc = ReadTokens(CMSSPARMS_OLD, &t);
	 if (rc != 0) return(rc);
	 }
      have_tokens = TRUE;
      }

   /* GET THE PARAMETER */
   rc = GetParm (&t, param, result);

   return (rc);
}

//---------------------------------------------------------------------
//							     cmssparamg
//---------------------------------------------------------------------
//
int  cmssparamg (char *fname, char *param, char *result) 
{
   TOKENS   t;
   int      rc = FALSE;

   rc = ReadTokens(fname, &t);
   if (rc != 0) return(rc);

   /* GET THE PARAMETER */
   rc = GetParm (&t, param, result);

   return (rc);
}

/********************************************************************
*			CMSSPARAMGT
* Return the entry (result) from the tokens file (fname) for the
* given parameter (param).
* Only re-read the tokens file if the file has been modified
* since the last time it was read.
********************************************************************/
int cmssparamgt (TOKENS *t, char *fname, time_t *read_time, char *param, char *result) 
{
   int          rc = FALSE;
   struct stat  statbuf;

   /* GET FILE STATUS */
   if (stat(fname,&statbuf) != 0) return(-1);

   /* RE-READ TOKENS FILE IF MODIFIED SINCE LAST READ */
   if (statbuf.st_mtime > *read_time || *read_time == 0) {
      rc = ReadTokens(fname, t);
      if (rc != 0) return(rc);
      *read_time = statbuf.st_mtime;
      }

   /* GET THE PARAMETER */
   rc = GetParm (t, param, result);

   return (rc);
}

/********************************************************************
*			CMSSPARAMGN
* Allow return of multiple words for the given param.
********************************************************************/
int  cmssparamgn(char *fname, char *param, char values[][80], int max) 
{
   TOKENS   t;
   int      rc = FALSE;

   rc = ReadTokens(fname, &t);
   if (rc != 0) return(rc);

   /* GET THE PARAMETER */
   rc = GetParms(&t, param, values, max);

   return (rc);
}

/********************************************************************
*			CMSSPARAMG STRING
* Allow return of multiple words for the given param.
* Return them in a string.
********************************************************************/
int  cmssparamgString(char *fname, char *param, char *str)
{
   TOKENS   t;
   int      rc = FALSE;
   char	    values[80][80];
   int	    i;
   int	    max = 80;

   rc = ReadTokens(fname, &t);
   if (rc != 0) return(rc);

   /* GET THE PARAMETER */
   rc = GetParms(&t, param, values, max);
   
   /* RETURN AS STRING */
   str[0] = '\0';
   for (i=0;i<rc;i++) {
      if (i == 0)
         strcpy(str,values[i]);
      else {
	 strcat(str," ");
	 strcat(str,values[i]);
	 }
      }

   return (rc);
}

/********************************************************************
*			CMSSPARAM STRING
* Allow return of multiple words for the given param.
* Return them in a string.
********************************************************************/
int  cmssparamString(char *param, char *str)
{
   TOKENS   t;
   int      rc = FALSE;
   char	    values[80][80];
   int	    max = 80;
   int      i;

   rc = ReadTokens(CMSSPARMS, &t);
   if (rc != 0) {
      rc = ReadTokens(CMSSPARMS_OLD, &t);
      if (rc != 0) return(rc);
      }

   /* GET THE PARAMETER */
   rc = GetParms(&t, param, values, max);
   
   /* RETURN AS STRING */
   str[0] = '\0';
   for (i=0;i<rc;i++) {
      if (i == 0)
         strcpy(str,values[i]);
      else {
	 strcat(str," ");
	 strcat(str,values[i]);
	 }
      }

   return (rc);
}

//---------------------------------------------------------------------
//							    ReadTokens 
//---------------------------------------------------------------------
// Read the tokens from the given file
//
static int ReadTokens(char *fname, TOKENS *t)
{
   FILE     *stream;

   /* OPEN THE PARAMETER FILE FOR THIS PROCESS.   */
   if ( (stream = fopen (fname, "r")) == NULL ) {
      return (-1);
      }

   /*  INITIALIZE AND READ THE PARAMETER FILE. */
   tokens_Init (t);
   while ( tokens_FileRead1Token (t, stream) == 0 );

   fclose (stream);

   return(0);
}

//---------------------------------------------------------------------
//                                                             Get Parm
//---------------------------------------------------------------------
//  Locate a specified parameter on the parameter file, and return it.
//
static int  GetParm (TOKENS *t, char *parm, char *value)
{
   int       nw;
   char      *words[2], parmx[100];

   strcpy (parmx, parm);
   strcat (parmx, "=");

   ;// GET THE PARAMETER FROM THE PARAMETER FILE BUFFER, AND COPY
   ;// IT INTO A CHARACTER STRING FOR THE CALLING FUNCTION.
   ;//

   nw = tokens_GetThisKeywordsWords (t, parmx, words, 2);
   if ( nw < 2 )
      return (-1);
   else {
      strcpy (value, words[1]);
      return (0);
      }
}

//---------------------------------------------------------------------
//                                                         Get Parms
//---------------------------------------------------------------------
//  Locate a specified parameter on the parameter file, and return 
//  the related values.
//
static int  GetParms(TOKENS *t, char *parm, char values[][80], int max)
{
   int       i, nw;
   char      parmx[100], *results[100];

   if (max > 100) max = 100;

   strcpy (parmx, parm);
   strcat (parmx, "=");

   nw = tokens_GetThisKeywordsWords (t, parmx, results, max);
   if ( nw < 2 ) return (-1);

   for (i=1;i<nw;i++) 
       strcpy(values[i-1],results[i]);

   return(nw-1);
}

/********************************************************************
*                    cmssparam_ReadTokens
********************************************************************/
int cmssparam_ReadTokens(char *fname, TOKENS *t)
{
int  rc;

rc = ReadTokens(fname,t);

return(rc);
}

/********************************************************************
*                    cmssparam_GetParm
********************************************************************/
int cmssparam_GetParm(TOKENS *t, char *parm, char *value)
{
int nw;

nw = GetParm(t, parm, value);
return(nw);
}

//---------------------------------------------------------------------
//                                                       Test Mainline 
//---------------------------------------------------------------------
//
//#define _TEST_CMSSPARAM_MAIN_
#ifdef _TEST_CMSSPARAM_MAIN_

main(int argc, char *argv[])
{
int	rc;
char	value[100];

if (argc != 2) {
   printf("Error: require home dir\n");
   exit(1);
   }

chdir(argv[1]);

rc = cmssparam ("SYSTEM",value);
if (rc == 0) printf("XXXXX = %s\n",value);
else printf("NOT FOUND\n");

}
#endif
