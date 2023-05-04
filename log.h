#ifndef	_LOG_H
#define _LOG_H

/*	"$Header: /home/commsa/cmdv/cmss/src/RCS/log.h,v 1.3 2013/11/19 23:12:36 ies Exp $  CMSS,BoM" */

/*<S>	log.h -- header for cmss log toolkit.
*
*       v1.7 20/11/13 Ian Senior Add Log_r and Logv_r
*       v1.6 08/10/10 Ian Senior Add LogClose
*	v1.5 940420 ACE Map all "Log" calls into "logc" calls.
*	v1.4 920827 ACE Allow full 80char user text messages
*	v1.3 920824 ACE Default log file is now "log"
*	v1.2 920824 ACE Add qLogMessage()  (for alarms)
*	v1.1 920814 ACE Initial version.
*/

/* Note:  Use the main program "createlog" to create the initial log file.*/

#include "logc.h"

#define Log(message)		logc(message)
#define Log_r(ident,message)	logc_r(ident,message)
#define Logv			logcv
#define Logv_r			logcv_r

#define LogOpen(logfilename,logid,action)	logc_Open("",logfilename,logid,action)
#define LogClose()              logc_Close()

#endif /* _LOG_H */
