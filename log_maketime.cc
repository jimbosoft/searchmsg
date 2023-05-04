static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/log_maketime.cc,v 1.2 2014/12/01 03:18:44 ies Exp $" ;
/*********************************************************************
*				LOG_MAKETIME
* FUNCTION	
* Convert a Log time string into a "time_t" value.
*
* v1.1 28/05/98  Ian Senior     Based on code from xlogext
* v1.2 01/12/14  Ian Senior     Use getcommstime_r()
*
* $Header: /home/commsa/cmdv/cmss/src/RCS/log_maketime.cc,v 1.2 2014/12/01 03:18:44 ies Exp $
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "log.h"
#include "getcommstime.h"
#include "cmmktime.h"
#include "log_maketime.h"

static int    cvt_int(char *str, int spos, int len);

/**********************************************************************
*                           LOG_MAKETIME
* Convert the date/time at the start of a log record from 
* YYMMDD hh.mm.ss to "time_t"
**********************************************************************/
time_t log_maketime(char *time_string)
{
int 		year, month, day, hour, min, sec;
time_t          timex, curr_time;
struct  tm      *tmx, tmr;

year = cvt_int(time_string,0,2);
month = cvt_int(time_string,2,2);
day = cvt_int(time_string,4,2);
hour = cvt_int(time_string,7,2);
min = cvt_int(time_string,10,2);
sec = cvt_int(time_string,13,2);

/* DETERMINE THE YEAR (SINCE 1900) */
/* ASSUME IS CURRENT YEAR OR PREVIOUS YEAR */
time(&curr_time);
tmx = getcommstime_r(curr_time,&tmr);
if (tmx->tm_year%100 != year)
   tmx->tm_year--;

tmx->tm_mon = month-1;
tmx->tm_mday = day;
tmx->tm_hour = hour;
tmx->tm_min = min;
tmx->tm_sec = sec;

timex = cmmktime(tmx);

return(timex);
}

/**********************************************************************
*                           CVT_INT
* Convert the given part of a character string into an integer.
**********************************************************************/
static int cvt_int(char *str, int spos, int len)
{
char	temp[20];

strncpy(temp,str+spos,len);
temp[len] = '\0';

return(atoi(temp));
}

