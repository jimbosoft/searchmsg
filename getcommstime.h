#ifndef _GETCOMMSTIME_H
#define _GETCOMMSTIME_H

#include <time.h>

#ifdef __cplusplus
   extern "C" {struct tm *getcommstime (time_t);};
   extern "C" {struct tm *getcommstime_r(time_t,struct tm *tmr);};
#else
   struct tm *getcommstime();
   struct tm *getcommstime_r();
#endif
#endif
