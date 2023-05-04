// "$Header: /home/commsa/cmdv/cmss/src/RCS/gtime.h,v 1.2 1995/06/21 10:32:11 tyc Exp $ CMSS,BoM"

#ifndef	_GTIME_H
#define _GTIME_H

//---------------------------------------------------------------------
//							        gtime.h
//---------------------------------------------------------------------
// The source for mkgtime() is in gtime.cc
//
// v1.1  20/06/95 tyc original
//---------------------------------------------------------------------
#include	<time.h>

time_t mkgtime (struct tm*);

#endif
