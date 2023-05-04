static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/dispdir.cc,v 1.38 2014/12/04 04:58:36 ies Exp $" ;
//
//  Crack open the passed message directory and display it
//  Calling sequence - dispdir MSGDIR *dir
//
// v1.1  23/03/95  Ian Senior	Add AIFS addressing
// v1.2  07/05/96  Ian Senior	Add msgaddrs_type.
// v1.3  22/08/97  Ian Senior	Replace localtime with getcommstime
// v1.4  11/11/97  Ian Senior	Add msgvalrp
// v1.5  28/01/98  Ian Senior	Change receive/send times to include
//				seconds.
// v1.6  19/02/98  T Crawford	fix problem with msgtxthd
// v1.24 12/06/98  Ian Senior	Return data as string instead of printf.
// v1.25 23/06/98  Ian Senior	add msgcor flag
// v1.27 31/05/98  Ian Senior	add msgnorec flag
// v1.28 09/11/99  Ian Senior	add msgmetdt2
// v1.29 18/05/00  T Crawford	add alert flags
// v1.30 04/10/01  Ian Senior	add msgorgid2 flags
// v1.31 30/04/02  Ian Senior	add index_txt
// v1.32 16/07/03  Ian Senior	Allow for long file extensions.
// v1.33 16/02/05  Ian Senior	add msgstx_orig.
// v1.34 11/05/05  Ian Senior	add msgttf, remove msgtelex.
// v1.35 20/11/08  Ian Senior	add gtsfilehdr
// v1.36 01/12/14  Ian Senior	Use reentrant getcommstime_r()
// v1.37 01/12/14  Ian Senior	Use reentrant msgdir_getfname_r()
//				and msgdir_getfext()
// v1.38 04/12/14  Ian Senior	Remove hexdump.h
//
// INCLUDE FILES
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include "cmss.h"
#include "msgdir.h"
#include "dispdir.h"
#include "RWaddresses.h"
#include "getcommstime.h"
#include "msgdir_getfname.h"

/*----------*/
/* MAINLINE */  
/*----------*/
void dispdir (char *dirnum, MSGDIR *dir, char *str)
{
   int       i, numaddr, len, lenx;
   struct tm *timeptr, tmr;
   char      datetime[15], datetimes[15];
   char	     addr[MAX_AIFS_ADDR][AIFS_ADDR_LEN+1];
   char	     flags[1000], fields[1000], addrd[1000];
   char	     filedir[1000];
   char      tempfname[1000], tempfext[500];

   RWReadList(dir,addr,MAX_AIFS_ADDR,&numaddr);


   sprintf(flags,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
      dir->msgignor  ? ("MSGIGNOR \t") : ("msgignor \t"),
      dir->msgrejct  ? ("MSGREJCT \t") : ("msgrejct \t"),
      dir->msgwmo    ? ("MSGWMO   \t") : ("msgwmo   \t"),
      dir->gtsfilehdr  ? ("GTSFILEHDR\t") : ("gtsfilehdr\t"),
      dir->msgfile   ? ("MSGFILE  \t") : ("msgfile  \n"),
      dir->msgbinry  ? ("MSGBINRY \t") : ("msgbinry \t"),
      dir->msgisdup  ? ("MSGISDUP \n") : ("msgisdup \t"),
      dir->msgaftn   ? ("MSGAFTN  \t") : ("msgaftn  \t"),
      dir->msgaftnin ? ("MSGAFTNIN\t") : ("msgaftnin\t"),
      dir->msgrsend  ? ("MSGRSEND \n") : ("msgrsend \n"),
      dir->msgnohdr  ? ("MSGNOHDR \t") : ("msgnohdr \t"),
      dir->msgrport  ? ("MSGRPORT \t") : ("msgrport \t"),
      dir->msgnosw   ? ("MSGNOSW  \t") : ("msgnosw  \t"),
      dir->msgnonmc  ? ("MSGNONMC \t") : ("msgnonmc \t"),
      dir->msgvalrp  ? ("MSGVALRP \n") : ("msgvalrp \n"),
      dir->msgcont   ? ("MSGCONT  \t") : ("msgcont  \t"),
      dir->msgnilrp  ? ("MSGNILRP \t") : ("msgnilrp \t"),
      dir->msgnodb   ? ("MSGNODB  \t") : ("msgnodb  \t"),
      dir->msggtshd  ? ("MSGGTSHD \t") : ("msggtshd \t"),
      dir->msgausfm  ? ("MSGAUSFM \n") : ("msgausfm \n"),
      dir->msgnorec  ? ("MSGNOREC \t") : ("msgnorec \t"),
      dir->msgcor    ? ("MSGCOR   \t") : ("msgcor   \t"),
      dir->msgttf    ? ("MSGTTF   \n") : ("msgttf   \n"),
      dir->msgnotxt  ? ("MSGNOTXT \t") : ("msgnotxt \t"),
      dir->toalert   ? ("TOALERT  \t") : ("toalert  \t"),
      dir->metalert  ? ("METALERT \t") : ("metalert \t"),
      dir->avalert   ? ("AVALERT  \n") : ("avalert  \n") );

timeptr = getcommstime_r(dir->msgtmrcv,&tmr);
strftime (datetime,14,"%j %T",timeptr);
timeptr = getcommstime_r(dir->msgtmsnd,&tmr);
strftime (datetimes,14,"%j %T",timeptr);

if (dir->msgfile)
   sprintf(filedir,"msgfname    = %7s\nmsgfext     = %7s\tmsgftype    = %7d\n",msgdir_getfname_r(dir,tempfname),msgdir_getfext_r(dir,tempfext),dir->msgftype);
else
   filedir[0] = '\0';

sprintf(fields,\
"msgadtype   = % 7d\n\
%s\
msgstx      = % 7d\tmsgstxof    = % 7d\tmsgstx_orig = %7d\n\
msgstx_aftn = % 7d\tmsgtrlen    = % 7d\tmsgrcvseq   = % 7d\n\
msgsndseq   = % 7d\tmsgnumads   = % 7d\tmsgtype     = % 7d\n\
msgformt    = % 7d\tmsgdirno    = % 7d\tmsgjnlno    = % 7d\n\
msgrepno    = % 7d\tmsgprty     =       %1.1s\tmsgretrd    =     %3.3s\n\
msgrrrcc    =   %5.5s\tmsgsrrcc    =   %5.5s\tmsgmetdt    = %6.6s\n\
msgprodid   =   %5.5s\tmsgorgid    = %10.10s\tmsgorgid2   = %s\n\
msgmetdt2 = %6.6s\tmsgtmrcv    = %s   msgtmsnd    = %8s\n\
msgstnno    = %10.10s\n\
msgdtead    = %16.16s\n\
msgtxthd    = %s\n",
     dir->msgaddrs_type, filedir,
     dir->msgstx, dir->msgstxof, dir->msgstx_orig,
     dir->msgstx_aftn, dir->msgtrlen, dir->msgrseq,
     dir->msgsseq, numaddr, dir->msgtype,
     dir->msgformt, dir->msgdirno, dir->msgjnl,
     dir->msgrepno, dir->msgprty,dir->msgretrd,
     dir->msgrrrcc, dir->msgsrrcc, dir->msgmetdt,
     dir->msgprodid, dir->msgorgid,dir->msgfile ? "" : dir->msgorgid2,
     dir->msgmetdt2, datetime, datetimes,
     dir->msgstnno, 
     dir->msgdtead,
     dir->msgtxthd);


strcpy(addrd,"Addresses   = ");
len = 0;
for (i=0;i<numaddr;i++) {
    lenx = strlen(addr[i])+1;
    if (len+lenx > 65) {
       strcat(addrd,"\n            ");
       len = 0;
       }
    strcat(addrd,addr[i]);
    strcat(addrd," ");
    len+=lenx;
    }
    
/* PRODUCE FINAL DISPLAY */

sprintf(str,"\
--------------------------------------------------------------------------\n\
Message #%s: msgoffst = %d:%d\tmsglen = %d\n\
----------------------------------- Flags --------------------------------\n%s\
----------------------------------- Fields -------------------------------\n%s\
%s\n\n",
		dirnum,dir->index_txt,dir->msgoffst,dir->msglen,flags,fields,addrd);

return;
}
