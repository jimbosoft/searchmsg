#ifndef	_ALARM_H
#define _ALARM_H

/*	" CMSS,BoM" */

/*<S>	alarm.h -- header for cmss alarm-log writing subroutine.
*
*	v1.1 920824 ACE Initial version.  (borrowed from log.h)
*	v1.2 940420 ACE Use "alarmc" to do everything now.
*/
/* Internally the alarm file is identical to a log file.*/
/* Hence use "createlog alarm nnnnn"  utility to create the alarms file.*/
/*    or use "cre8logc alarm nnnn"   */

#include "alarmc.h"

/* Use macros to turn "Alarm" calls into "alarmc" calls.*/

#define Alarm(msg)			alarmc(msg)
#define Alarmv				alarmcv
#define AlarmOpen(alarmfilename)	alarmc_Open ("",alarmfilename,"",0)

#endif /* _ALARM_H */
