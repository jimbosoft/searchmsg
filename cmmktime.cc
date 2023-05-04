static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/cmmktime.cc,v 1.3 2014/12/01 04:50:15 ies Exp $" ;
/**********************************************************************
*  				CMMKTIME
*   Given a date/time structure return the equivalent time_t value.
*   The value returned depends on whether we are using UTC or 
*   localtime.
*   THIS FUNCTIONS SHOULD BE USED IN CONJUNCTURE WITH "getcommstime".
*
* v1.1  20/08/97  Ian Senior
* v1.2  01/12/14  Ian Senior Minor change to make sort of thread safe
*
* $Header: /home/commsa/cmdv/cmss/src/RCS/cmmktime.cc,v 1.3 2014/12/01 04:50:15 ies Exp $
**********************************************************************/
#include	<stdio.h>
#include	<time.h>
#include	<string.h>
#include	"gtime.h"
#include	"cmssparam.h"
#include	"cmmktime.h"

#define TRUE	1
#define FALSE   0

static int	know_timezone = FALSE;
static int	use_utc_time = FALSE;

/**********************************************************************
* If using local time than call the standard mktime else if
* UTC then call our mkgtime.
**********************************************************************/
time_t cmmktime (struct tm *tp)
{
time_t	cmtime;
char	value[80];

if (!know_timezone) {
   if ( cmssparam ("TIMEZONE", value) == 0 ) {
      if ( !strcmp (value, "GMT") || !strcmp (value, "UTC") ) 
	 use_utc_time = TRUE; 
      }
   know_timezone = TRUE;
   }

if ( use_utc_time )
   cmtime = mkgtime(tp);
else
   cmtime = mktime(tp);

return (cmtime);
}

/**********************************************************************
*			TEST MAINLINE
**********************************************************************/
#ifdef TEST_CMMKTIME
main()
{
struct  tm 	tmx;
time_t		timex;

tmx.tm_year = 1997;
tmx.tm_mday = 20;
tmx.tm_mon = 7;
tmx.tm_hour = 02;
tmx.tm_min = 05;
tmx.tm_sec = 59;

timex = cmmktime(&tmx);

printf("LOCALTIME: %s\n",asctime(localtime(&timex)));
printf("UTC: %s\n",asctime(gmtime(&timex)));
}
#endif
