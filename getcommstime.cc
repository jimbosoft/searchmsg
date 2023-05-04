static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/getcommstime.cc,v 1.6 2013/11/18 05:12:05 ies Exp $" ;
//---------------------------------------------------------------------
// 
//  FUNCTION	getcommstime.cc
// 
//  PROTOTYPE   getcommstime.h
// 
//  SUMMARY	getcommstime (time_t thetime)
//		
//  RETURN
//  A pointer to the broken down time structure "tm"
// 
//  DESCRIPTION	
//  Given a time_t value this function return the broken down time
//  in Local or UTC time depending on a CMSS parameter.
//  ie depending on whether CMSS is using Local time or UTC.
// 
//  HISTORY 
//  v1.0  02/06/97  Doug Adams    Original code.
//  v1.2  08/08/97  Ian Senior	  Change a bit
//  v1.3  18/11/13  Ian Senior	  Create re-entrant version 
// 
//  $Header: /home/commsa/cmdv/cmss/src/RCS/getcommstime.cc,v 1.6 2013/11/18 05:12:05 ies Exp $
//---------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <memory.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include "alarm.h"
#include "cmssparam.h"
#include "getcommstime.h"

#define TRUE  1
#define FALSE 0

static  int   checked = FALSE, use_gmt_time = FALSE;

struct tm *getcommstime (time_t the_time)
{
   struct tm 	*ts;
   char		value[80];

   if ( !checked ) {
      checked = TRUE;
      if ( cmssparam ("TIMEZONE", value) == 0 ) {
	 if ( !strcmp (value, "GMT") || !strcmp (value, "UTC") ) 
	    use_gmt_time = TRUE; 
	 }
      }

   if ( use_gmt_time )
      ts = gmtime(&the_time);
   else
      ts = localtime(&the_time);

   return(ts);
}

/******************************************************************
*		GET COMMS TIME (re-entrant)
* Re-entrant (thread safe) version.
* Not truely threadsafe because of the globals checked and 
* use_gmt_time but they should not matter as they dont change.
******************************************************************/
struct tm *getcommstime_r(time_t the_time, struct tm *tsr)
{
   char   value[80];
   struct tm *ts;

   if ( !checked ) {
      checked = TRUE;
      if ( cmssparam ("TIMEZONE", value) == 0 ) {
	 if ( !strcmp (value, "GMT") || !strcmp (value, "UTC") ) 
	    use_gmt_time = TRUE; 
	 }
      }

   if ( use_gmt_time )
      ts = gmtime_r(&the_time,tsr);
   else
      ts = localtime_r(&the_time,tsr);

   return(ts);
}
