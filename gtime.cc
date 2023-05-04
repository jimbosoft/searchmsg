static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/gtime.cc,v 1.2 1998/06/02 05:38:50 cmdv Exp $" ;
// <T> "$Header: /home/commsa/cmdv/cmss/src/RCS/gtime.cc,v 1.2 1998/06/02 05:38:50 cmdv Exp $ CMSS BoM"
// <T> GTIME  Additional Time Function
//----------------------------------------------------------------------
// 
//  FUNCTION	gtime.cc
// 
//  DESCRIPTION	This module contains the function mkgtime ().
//
//  This function is the inverse of standard time function gmtime ()
//  as mktime() is the inverse of localtime().
// 
//  HISTORY 
//  v1.1  21/06/95  tyc  original
//----------------------------------------------------------------------
#include	"gtime.h"

// <F>
//----------------------------------------------------------------------
//  This function returns a calendar time (seconds since 1/1/1970
//  given a date structure in UTC time
//
//  This is the inverse function of gmtime()
//  cf mktime() which is the inverse function of localtime()
//----------------------------------------------------------------------
time_t mkgtime (struct tm *tp)
{
return (mktime (tp) - timezone);
}
