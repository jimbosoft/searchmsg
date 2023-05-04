#ifndef LINKTIME_H
#define LINKTIME_H
/* Main programs should hash-include this header file, */
/* to force IBM's to load the linktime ident string we generate by linktime.sh*/
/* $Header: /home/commsa/cmdv/cmss/src/RCS/linktime.h,v 1.1 1998/11/19 07:06:16 ace Exp $ */
/* v1.1 981119 C.Edington, initial code.*/
#ifdef AIX
extern char linktime[];
static char *linktime_=linktime;
#endif
#endif
