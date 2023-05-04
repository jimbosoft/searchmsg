static char src_id [] = "$Header: /home/commsa/cmdv/cmss/src/RCS/logfind.cc,v 1.1 2009/02/16 01:25:17 cmdv Exp $";
/*********************************************************************
*				LOGFIND 
*
* SUMMARY
*	int logfind(time_t endtime, int *recno, CirFile  *cfile)	
*		endtime = time of record to be found
*		cfile = circular file to be searched.
*		recno = number of record found
*       Return Code: 
*		0 = found
*		1 = not found
*		-1 = error
*
* FUNCTION	
* Perform a binary search on the given circular file looking for the
* latest record whose time is less than that specified.
* Return the record number of this record.
*
* v1.1 28/05/98  Ian Senior     Based on code from xlogext
* v1.2 28/11/00  C.Edington	Fixed for expanding-circular-files/zero-fill.
*
* $Header: /home/commsa/cmdv/cmss/src/RCS/logfind.cc,v 1.1 2009/02/16 01:25:17 cmdv Exp $
*********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "cmss.h"
#include "msgdir.h"
#include "cirfile.h"
#include "log.h"
#include "getcommstime.h"
#include "cmmktime.h"
#include "getgroup.h"
#include "logfind.h"
#include "log_maketime.h"

#define MAX_RECORD_LENGTH  512

static int    SearchRange(int high, int low, int max);

static CirFile  *cirfile;

/***********************************************************************
*                       LOG FIND
* Try to find the lastest record that has a time less than that 
* required (endtime). Return its record number (recno).
*
* Do this by performing a binary search on the file.
***********************************************************************/
int logfind(time_t endtime, int *recno, CirFile  *cfile)
{
time_t  rectime;
int     matching_recno = -1;
int     low, high, max, curr_recno, mid, status, retcode;
char	linebuffer[MAX_RECORD_LENGTH+1];

cirfile = cfile;

/* SET SEARCH RANGE */
high = cirfile->qNtbwMinus1();
low = cirfile->CorrectRecordNo (high + 2);
max = cirfile->qMaxRecords();

/* GET INITIAL SEARCH RANGE */
mid = SearchRange(high, low, max);

/* SEARCH FILE */
while (mid != 0) {

   curr_recno = low+mid;
   if (curr_recno >= max)  curr_recno -= max;

   /* printf ("Search: low=%i, current=%i, high=%i, mid=%i, max=%i\n",
                low, curr_recno, high, mid, max);*/

   status = cirfile->LookRecord(linebuffer,curr_recno);
   if (status != 0) {
      Log("XLOGEXT: ERROR READING LOG FILE");
      return(-1);
      }

   if (linebuffer[0]=='\0') {
    // Here is a zero-fill record at the end of an expanded file.
    // Reduce the current maximum record number to here.
    max = curr_recno;
    if (high>=max) high = max-1;
    if (low>=max)  low = 0;
    // and then retry the search loop.
     }
   else {
    // check a good record.
   rectime = log_maketime(linebuffer);
   linebuffer[60] = '\0';
   if (difftime(rectime,endtime) < 0) {
      matching_recno = curr_recno;
      low = curr_recno;
      }
   else {
      high = curr_recno;
      }
   }

   mid = SearchRange(high, low, max);
   }

if (matching_recno >= 0) {
   retcode = 0;
   *recno = matching_recno;
   }
else
   retcode = 1;

return(retcode);
}

/**********************************************************************
*                       SEARCH RANGE
* Return the "midpoint" between the "high" and the "low" values,
* allowing for the fact that this is a circular file. 
**********************************************************************/
static int SearchRange(int high, int low, int max) 
{
int     range;

range = high - low;
if (range < 0) 
   range += max;

return(range/2);
}

/**********************************************************************
*                       TEST MAINLINE
**********************************************************************/
#ifdef _TEST_MAINLINE_LOGFIND_
main(int argc, char *argv[])
{
static CirFile  cirfile;
time_t		test_time;
int		rc, recno;

chdir(argv[1]);

rc = cirfile.OpenLooker(argv[2]); 
if (rc != 0) {
   printf("woops\n");
   return(1);
   }

time(&test_time);
test_time -= 3600;

rc = logfind(test_time, &recno, &cirfile);
printf("rc = %d recno = %d\n",rc,recno);

}
#endif
