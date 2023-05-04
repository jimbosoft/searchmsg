static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/getgroup.cc,v 1.12 2014/12/05 00:55:35 ies Exp $" ;
//<F><T> GETGRP - Extract a group of text of a type
/********************************************************************
*
*  FUNCTION	To extract a substring of text from a passed string,
* 		delimited by spaces, newline characters, or a change
*		of type.
*  
*  $Header: /home/commsa/cmdv/cmss/src/RCS/getgroup.cc,v 1.12 2014/12/05 00:55:35 ies Exp $ 
*  
*  HISTORY
*        
*  1.12  05/12/14  Ian Senior     Add getngrp_r() as a threadsafe
*				  version of getngrp()
*  1.11  03/08/10  T Crawford     remove PLUS processing
*  ....
*  1.07  24/07/96  T Crawford     fix dangling pointer
*
*    00  01/07/93  T Crawford     based on getngrp.cc                   
*    01  04/06/96  T Crawford     add PLUS processing
*        05/06/96  T Crawford     modify EQUALS processing
*    02  06/06/96  T Crawford     add DOT processing
*    03  12/06/96  T Crawford     fix bug in DOT processing
*
*  SOURCE IN	getgrp.c
*
*  PROTOTYPE 	getgroup.h
*
*  SUMMARY	strcpy (buf,getgrp(text,textlen,start,number,type));
*
*	where	char * buf              destination of substring
*		char * text	    	string substring is taken from
*		int    textlen		length of string
*		int  * start		starting position within string
*  		int  * number		length of substring
*		int  * type		type of string returned
*	
*  RETURN VALUE     There is no actual return code from this function,
*		    the data is returned in start, number and type.
*	
*  DESCRIPTION	
*
*	This function accepts as input parameters, a block of text,   
*	the length of this block of text, and the offset into the text 
*	the scan for a group is to start.  From this it sets up two 
*	character pointers - t1 and t2, at the offset into the block
*	of text specified by the input variable "start".  From this 
*	point in the text block it scans through the block, ignoring
*	leading spaces, and control characters that do not compose a 
*  	newline group (i.e. carriage returns and line feeds).
*
*	As soon as a newline or alphanumeric character is detected, the 
*	pointer t1 is "locked" at this character, and the pointer t2 
* 	continues to scan the text block until another space character, 
*       or a change of type is detected (i.e. from alphabetic to
*	numeric, or vice versa).  This signifies the end of the group.
*
*	The length of the group is determined by differencing the 
*	pointers, and the substring is copied into the character buffer
* 	that will be returned to the caller. The substring is null 
*       terminated.
*
*       If the end of the message is reached (i.e. pointer reaches 
*	the string address + string length) without extracting a group
*	(i.e. t2 is still set to the string address + start), then 
*	a null string is returned with the type set to EOM.
*
*	The return buffer is verified against newline, alphabetic and
*	numeric character sets and the "type" return parameter is set
*   	as follows -
*
*		1 - alphabetic data (alphabet characters plus the '$')
*		2 - numeric data    (numeric characters 0 to 9
*		3 - newline group   (only car. returns and line feeds)
*		4 - mixed data      (alphabetic and numeric)
*		5 - end of message detected
*
***********************************************************************/
#include <stdio.h>
#include <string.h>
#include "getgroup.h"
#include "cmss.h"

#define  ALPHABETICS  "ABCDEFGHIJKLMNOPQRSTUVWXYZ?"
#define  ALPHABETIX   "ABCDEFGHIJKLMNOPQRSTUVWXYZ?abcdefghijklmnopqrstu\vwxy"
#define  NUMERICS     "0123456789"
#define	 NUMERIX      "0123456789/X."
#define  NEWLINES     "\n\r"
#define  SOLIDUS      "/"


//<F><T> GETNGRP - Extract a group of text
char * getgrp (char *txt,int len,int *start,int *num,int *type) 
{
   int    group_found = FALSE;
   char   *t1, *t2, charbuf[2];
   static char   group [MAX_MSGSIZE+1]; /* v1.07 */

   t1 = txt + *start;          /* Set pointers to offset into buffer */
   t2 = t1;

   do
      {
      charbuf[0] = *t1; charbuf[1] = '\0';
      if ( strspn (charbuf,ALPHABETICS) == 1 )
	 {                                           /* Alphabetic   */
	 t2 = t1;
	 do
	    {                           /* Scan to far side of group */
	    t2++;
            charbuf[0] = *t2; charbuf[1] = '\0';
	    } while ( ( strspn (charbuf,ALPHABETICS) == 1 ) &&
	              ( (t2 - txt) < len) );
         group_found = TRUE;
         }
      else if ( strspn (charbuf,NUMERICS) == 1 )
	 {                                           /* Numeric      */
         t2 = t1;
	 do
	    {                           /* Scan to far side of group */
	    t2++;
            charbuf[0] = *t2; charbuf[1] = '\0';
	    } while ( ( strspn (charbuf,NUMERICS) == 1 ) &&
	                                       ( (t2 - txt) < len) );
         group_found = TRUE;
         }
      else if ( strspn (charbuf,NEWLINES) == 1 )
	 {                                          /* Newline       */ 
         t2 = t1;
	 do 
	    {                           /* Scan to far side of group */
	    t2++;
            charbuf[0] = *t2; charbuf[1] = '\0';
	    } while ( ( strspn (charbuf,NEWLINES) == 1 ) &&
	                                       ( (t2 - txt) < len) );
         group_found = TRUE;
         }
      else if ( strspn (charbuf,SOLIDUS) == 1 )
	 {                                          /* Solidi        */ 
         t2 = t1;
	 do 
	    {                           /* Scan to far side of group */
	    t2++;
            charbuf[0] = *t2; charbuf[1] = '\0';
	    } while ( ( strspn (charbuf,SOLIDUS) == 1 ) &&
	                                       ( (t2 - txt) < len) );
         group_found = TRUE;
         }
      if (!group_found) 
	 t1++;
      } while ( (!group_found) && ((t1 - txt) < len) );

   if ( t2 == (txt + *start) )
      {                           /* If no groups extracted ...       */
      *type = EOM;
      *num = 0;
      return ((char*) "");
      }
   else
      {
      *start = t2 - txt;         /* Set offset pointer past group    */
      *num = t2 - t1;            /* Calculate length of group        */
      strncpy (group,t1,*num);   /* Copy group into return buffer    */
      strcpy (group+*num,"\0");  /* Copy null terminater into buffer */
      if (strspn(group,ALPHABETICS) == *num) 
	 *type = ALPHA;
      else if (strspn(group,NUMERICS) == *num)
	 *type = NUMERIC;
      else if (strspn(group,NEWLINES) == *num)
	 *type = NEWLINE;
      else if (strspn(group,SOLIDUS) == *num)
	 *type = SOLIDI;
      else 
	 *type = MIXED;
      }
   return (group);
}


/*<F><T> GETNGRP - Extract a group of text
*********************************************************************
*
*  FUNCTION	To extract a substring of text from a passed string,
* 		delimited by spaces or by newline characters.
*  
*  SOURCE IN	getngrp.c
*
*  PROTOTYPE 	getngrp.h
*
*  SUMMARY	strcpy (buf,getngrp(text,textlen,start,number,type));
*
*	where	char * buf              destination of substring
*		char * text	    	string substring is taken from
*		int    textlen		length of string
*		int  * start		starting position within string
*  		int  * number		length of substring
*		int  * type		type of string returned
*	
*  RETURN VALUE     There is no actual return code from this function,
*		    the data is returned in start, number and type.
*	
*  DESCRIPTION	
*
*	This function accepts as input parameters, a block of text,   
*	the length of this block of text, and the offset into the text 
*	the scan for a group is to start.  From this it sets up two 
*	character pointers - t1 and t2, at the offset into the block
*	of text specified by the input variable "start".  From this 
*	point in the text block it scans through the block, ignoring
*	leading spaces, and control characters that do not compose a 
*  	newline group (i.e. carriage returns and line feeds).
*
*	As soon as a newline or alphanumeric character is detected, the 
*	pointer t1 is "locked" at this character, and the pointer t2 
* 	continues to scan the text block until another delimeter is
*	detected at the other end of the group.
*
*	The length of the group is determined by differencing the 
*	pointers, and the substring is copied into the character buffer
* 	that will be returned to the caller.  The returned substring is
*       null terminated.
*
*       If the end of the message is reached (i.e. pointer reaches 
*	the string address + string length) without extracting a group
*	(i.e. t2 is still set to the string address + start), then 
*	a null string is returned with the type set to EOM.
*
*	The return buffer is verified against newline, alphabetic and
*	numeric character sets and the "type" return parameter is set
*   	as follows -
*
*		1 - alphabetic data (alphabet characters plus the '$')
*		2 - numeric data    (numeric characters 0 to 9
*		3 - newline group   (only car. returns and line feeds)
*		4 - mixed data      (alphabetic and numeric)
*		5 - end of message detected
*
*  HISTORY
*        
*    00  10/08/92  Doug Adams     Original code - based on Facom version
*    01  27/08/93  T Crawford     use C string processing if len == -1
*
***********************************************************************/

char * getngrp (char *txt,int len,int *start,int *num,int *type) 
{
int    group_found = FALSE;
char   *t1, *t2;
static char   group [MAX_GROUP_LENGTH+1]; /* v1.07 */



//if (*start >= strlen (txt))
//   printf("NGP 1 %s\n%i %i\n",txt, *start, strlen(txt));
t1 = txt + *start;      /* Set pointers to offset into buffer */
t2 = t1;
//printf("GETNGRP\nt1 %i  t2 %i  txt %i  start %i\n\n", t1, t2, txt, *start);

if (t1[0] == '\0') {     /* If no groups were extracted */
   //printf("text string is finished !!\n");
   *type = EOM;
   *num = 0;
   return ((char*) "");
   }

if (len == -1) {         /* txt is null terminated */
   do {
      if ( *t1 > ' ') {          /* Printable character detected    */
	 t2 = t1;
	 do {                     /* Scan to far side of group       */
	    t2++;
	    } while ( (*t2 > ' ') && (t2[0] != '\0') );
         group_found = TRUE;
         }
      else                       /* Non printable character detected */ 
	 if ( (*t1 == '\r') || (*t1 == '\n') ) {
				 /* NL character     */
            t2 = t1;
	    do {                 /* Scan to far side of NL group     */
	       t2++;
	       } while ( ( (*t2 == '\r') || (*t2 == '\n') ) &&
	                  (t2[0] !='\0') );
            group_found = TRUE;
            }
      if (!group_found) 
	 t1++;
      } while ( (!group_found) && (t1[0] != '\0') );
   }
else {
   do {
      if ( *t1 > ' ' ) {          /* Printable character detected    */
	 t2 = t1;
	 do {                     /* Scan to far side of group       */
	    t2++;
	    } while ( (*t2 > ' ') && ((t2 - txt) < len) );
         group_found = TRUE;
         }
      else                       /* Non printable character detected */ 
	 if ( (*t1 == '\r') || (*t1 == '\n') ) {
				 /* NL character     */
            t2 = t1;
	    do {                 /* Scan to far side of NL group     */
	       t2++;
	       } while ( ( (*t2 == '\r') || (*t2 == '\n') ) &&
	                 ( (t2 - txt) < len) );
            group_found = TRUE;
            }
      if (!group_found) 
	 t1++;
      } while ( (!group_found) && ((t1 - txt) < len) );
   }

   if ( t2 == (txt + *start) ) 
      {                           /* If no groups were extracted */
      *type = EOM;
      *num = 0;
      return ((char*) "");
      }
   else
      {
      //printf("GETNGRP\nt1 %i  t2 %i  txt %i  start %i\n\n", t1, t2, txt, *start);
      if (t2 - t1 > MAX_GROUP_LENGTH) /* group too long! */
	 t2 = t1 + MAX_GROUP_LENGTH;
      *start = t2 - txt;         /* Set offset pointer past group    */
      //printf("start : %i\n\n", *start);
      *num = t2 - t1;            /* Calculate length of group        */
      strncpy (group,t1,*num);   /* Copy group into return buffer    */
      strcpy (group+*num,"\0");  /* Copy null terminater into buffer */
      if (strspn(group,ALPHABETICS) == *num) 
	 *type = ALPHA;
      else if (strspn(group,NUMERICS) == *num)
	 *type = NUMERIC;
      else if (strspn(group,NEWLINES) == *num)
	 *type = NEWLINE;
      else 
	 *type = MIXED;
      }
   
//if (*start > strlen (txt))
   //printf("NGP 2 %s\n%i %i\n",txt, *start, strlen(txt));
   return (group);
}

/*********************************************************************
*			GETNGRP_R
*
* RE-ENTRANT (THREADSAFE) VERSION OF getngrp. DIFFERENCE IS THAT THIS
* DOES NOT USE A STATIC VARIABLE
*
*  FUNCTION	To extract a substring of text from a passed string,
* 		delimited by spaces or by newline characters.
*  
*  SOURCE IN	getngrp.c
*
*  PROTOTYPE 	getngrp.h
*
*  SUMMARY	getngrp_r(text,textlen,start,number,type,group,glen);
*
*	where	char * text	    	string substring is taken from
*		int    textlen		length of string
*		int  * start		starting position within string
*  		int  * number		length of substring
*		int  * type		type of string returned
*	        char * group            destination of substring
*               int    glen             maximum length of group
*	
*  RETURN VALUE     There is no actual return code from this function,
*		    the data is returned in start, number and type.
*	
*  DESCRIPTION	
*
*	This function accepts as input parameters, a block of text,   
*	the length of this block of text, and the offset into the text 
*	the scan for a group is to start.  From this it sets up two 
*	character pointers - t1 and t2, at the offset into the block
*	of text specified by the input variable "start".  From this 
*	point in the text block it scans through the block, ignoring
*	leading spaces, and control characters that do not compose a 
*  	newline group (i.e. carriage returns and line feeds).
*
*	As soon as a newline or alphanumeric character is detected, the 
*	pointer t1 is "locked" at this character, and the pointer t2 
* 	continues to scan the text block until another delimeter is
*	detected at the other end of the group.
*
*	The length of the group is determined by differencing the 
*	pointers, and the substring is copied into the character buffer
* 	that will be returned to the caller.  The returned substring is
*       null terminated.
*
*       If the end of the message is reached (i.e. pointer reaches 
*	the string address + string length) without extracting a group
*	(i.e. t2 is still set to the string address + start), then 
*	a null string is returned with the type set to EOM.
*
*	The return buffer is verified against newline, alphabetic and
*	numeric character sets and the "type" return parameter is set
*   	as follows -
*
*		1 - alphabetic data (alphabet characters plus the '$')
*		2 - numeric data    (numeric characters 0 to 9
*		3 - newline group   (only car. returns and line feeds)
*		4 - mixed data      (alphabetic and numeric)
*		5 - end of message detected
*
*  HISTORY
*    00  05/12/14  Ian Senior
***********************************************************************/
void getngrp_r(char *txt,int len,int *start,int *num,int *type, char *group, int max_group_length) 
{
int    group_found = FALSE;
char   *t1, *t2;

group[0] = '\0';

//if (*start >= strlen (txt))
//   printf("NGP 1 %s\n%i %i\n",txt, *start, strlen(txt));
t1 = txt + *start;      /* Set pointers to offset into buffer */
t2 = t1;
//printf("GETNGRP\nt1 %i  t2 %i  txt %i  start %i\n\n", t1, t2, txt, *start);

if (t1[0] == '\0') {     /* If no groups were extracted */
   //printf("text string is finished !!\n");
   *type = EOM;
   *num = 0;
   group[0] = '\0';
   return;
   }

if (len == -1) {         /* txt is null terminated */
   do {
      if ( *t1 > ' ') {          /* Printable character detected    */
	 t2 = t1;
	 do {                     /* Scan to far side of group       */
	    t2++;
	    } while ( (*t2 > ' ') && (t2[0] != '\0') );
         group_found = TRUE;
         }
      else                       /* Non printable character detected */ 
	 if ( (*t1 == '\r') || (*t1 == '\n') ) {
				 /* NL character     */
            t2 = t1;
	    do {                 /* Scan to far side of NL group     */
	       t2++;
	       } while ( ( (*t2 == '\r') || (*t2 == '\n') ) &&
	                  (t2[0] !='\0') );
            group_found = TRUE;
            }
      if (!group_found) 
	 t1++;
      } while ( (!group_found) && (t1[0] != '\0') );
   }
else {
   do {
      if ( *t1 > ' ' ) {          /* Printable character detected    */
	 t2 = t1;
	 do {                     /* Scan to far side of group       */
	    t2++;
	    } while ( (*t2 > ' ') && ((t2 - txt) < len) );
         group_found = TRUE;
         }
      else                       /* Non printable character detected */ 
	 if ( (*t1 == '\r') || (*t1 == '\n') ) {
				 /* NL character     */
            t2 = t1;
	    do {                 /* Scan to far side of NL group     */
	       t2++;
	       } while ( ( (*t2 == '\r') || (*t2 == '\n') ) &&
	                 ( (t2 - txt) < len) );
            group_found = TRUE;
            }
      if (!group_found) 
	 t1++;
      } while ( (!group_found) && ((t1 - txt) < len) );
   }

   if ( t2 == (txt + *start) ) 
      {                           /* If no groups were extracted */
      *type = EOM;
      *num = 0;
      group[0] = '\0';
      return;
      }
   else
      {
      //printf("GETNGRP\nt1 %i  t2 %i  txt %i  start %i\n\n", t1, t2, txt, *start);
      if (t2 - t1 > max_group_length) /* group too long! */
	 t2 = t1 + max_group_length;
      *start = t2 - txt;         /* Set offset pointer past group    */
      //printf("start : %i\n\n", *start);
      *num = t2 - t1;            /* Calculate length of group        */
      strncpy (group,t1,*num);   /* Copy group into return buffer    */
      strcpy (group+*num,"\0");  /* Copy null terminater into buffer */
      if (strspn(group,ALPHABETICS) == *num) 
	 *type = ALPHA;
      else if (strspn(group,NUMERICS) == *num)
	 *type = NUMERIC;
      else if (strspn(group,NEWLINES) == *num)
	 *type = NEWLINE;
      else 
	 *type = MIXED;
      }
   
   return;
}


/**********************************************************************
*			GET NEXT GROUP
*  v1.1 16/11/95 tyc 
**********************************************************************/
void get_next_group (char *group, char **text, int *num, int *type)
{
char	*equals_sign;
char	*plus_sign;
char	*dot_sign;
int	st, ln, ln2, tp;

if (*text[0] == '\0') { /* empty string */ 
   *type = EOM;
   group = "";
   *num = 0;
   return;
   }

// v1.1
// TRANSFER NEWLINE PROCESSING TO HIGHER LEVEL
// SEE get_next () IN recog_tools.cc

//tp = NEWLINE; v1.1
st = 0;

//while (tp == NEWLINE) { v1.1
   strcpy (group, getngrp (*text, -1, &st, &ln, &tp));
   //printf("GETGROUP %i %s %i %i\n", st, group, ln, tp);
   //printf(".... %s\n", *text);
//   } v1.1

if (st > strlen (*text)) {
   //printf("STR %s\n%i %i\n",*text, st, strlen(*text));
   *text = *text + strlen (*text);
   }
else
   *text	= *text + st;

*num	= ln;
*type	= tp;

if (ln > MAX_GROUP_LENGTH) {
   *type = MAXCH;
   group [MAX_GROUP_LENGTH] = '\0';
   *num	= MAX_GROUP_LENGTH;
   return;
   }

#ifdef DEBUG
printf ("group [%s] type [%d]\n", group, tp);
#endif
if (tp == MIXED) {
   // V03 
#ifdef DEBUG
printf ("mixed group [%s]\n", group);
#endif
   for (;;) {
      if (strspn (group, NUMERIX) == ln) {
         *type = NUMERIC;
#ifdef DEBUG
printf ("mixed -> numeric\n");
#endif
	 break;
         }

      if (strspn (group, ALPHABETIX) == ln) {
	 *type = ALPHA;
#ifdef DEBUG 
printf ("mixed -> alpha\n"); 
#endif
	 break;
	 }

      equals_sign = strchr (group, '=');
      if (equals_sign != NULL) {
	 // V01 ACCEPT GROUP STARTING WITH =
	 //     NOT JUST A SINGLE ONE
	 if (equals_sign == group) {
	    *type	= EQUALS;
#ifdef DEBUG 
printf ("mixed -> equals\n"); 
#endif

	    }
	 else {
	    st = 0;
	    ln2 = ln;
	    group[equals_sign-group] = '\0';
	    get_next_group (group, &group, &ln, &tp);
	    *text	= *text - ln2 + ln;
	    *num	= ln;
	    *type	= tp;
#ifdef DEBUG 
printf ("split group at equals\n"); 
#endif
	    }
         break;
	 }

// V1.11
goto SKIP;   

      // V01 check for plus signs in ship reports
      plus_sign = strchr (group, '+');
      if (plus_sign != NULL) {
	 if (plus_sign == group) {
	    *type	= PLUS;
#ifdef DEBUG 
printf ("mixed -> plus\n"); 
#endif
	    }
	 else {
	    st = 0;
	    ln2 = ln;
	    group[plus_sign-group] = '\0';
	    get_next_group (group, &group, &ln, &tp);
	    *text	= *text - ln2 + ln;
	    *num	= ln;
	    *type	= tp;
#ifdef DEBUG 
printf ("split group at plus\n"); 
#endif
	 }
      break;
      }
SKIP:;


   // V02 check for dots in ship reports
   dot_sign = strchr (group, '.');
   if (dot_sign != NULL) {
      if (dot_sign == group) {
	 *type	= DOT;
#ifdef DEBUG 
printf ("mixed -> dot\n"); 
#endif
	    }
         break;
	 }
      break;
      }
   }
return;
}
