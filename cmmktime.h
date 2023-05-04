/*********************************************************************
*			CMMKTIME.H
* v00 		Ian Senior	Original
* v1.2 09/02/07 Ian Senior      Make little C compatible
*********************************************************************/
#ifndef _CMMKTIME_H
#define _CMMKTIME_H

#include <time.h>
#ifdef __cplusplus
   extern "C" {time_t cmmktime (struct tm *tp);};
#else
   time_t cmmktime (struct tm *tp);
#endif



#endif
