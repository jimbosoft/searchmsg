static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/searchin.cc,v 1.25 2021/05/12 04:58:16 dja Exp $" ;

/*******************************************************************
*				SEARCHIN
* Test tool to allow command line searches of CMSS files.
* Primarily for Fiji where the GUI is unavailable.
*
* v1.1  30/01/98  Ian Senior	original
* v1.2  14/04/98  Ian Senior	Add print option.
*				And add call to xsummary.
* v1.4  14/01/00  Ian Senior	Remove call to xsummary.
* v1.5  14/01/00  Ian Senior	Allow message types as names as well
*				as numbers
* v1.6  20/01/00  Daniel Lang   Enable options for web usage
*                               + Added -w flag (web output)
*                               + Multiple -m <message type> flags
*                               + Multiple -s <stn_id> flags
*                               + Added -S <starttime> -E <endtime> (ttime)
* v1.7  08/03/00  Ian Senior	Allow search by msg type only. 
* v1.8  08/05/00  Ian Senior	? 
* v1.9  24/05/00  Ian Senior	Fix bug in msg type search. 
* v1.10 23/11/00  C.Edington    Fixed for expanding-circular-files/zero-fill.
* v1.11 09/03/01  C.Edington	Fix main-return.
* v1.12 27/04/01  Ian Senior    Dont display header/trailer.
* v1.13 29/04/02  Ian Senior    Change to ReadMsgText.
* v1.14 01/07/02  C.Edington	Better logic when circfile has zeros.
* v1.15 21/03/03  Ian Senior	Add restrictions because of large 
*    number of anonymous Web users doing long CPU intensive searches.
* v1.16 15/04/03  Ian Senior	Allow for MSGIN being smaller than
*				REPORTS file
* v1.17 28/07/04  Ian Senior	Optionally print in reverse order.
* v1.18 03/08/04  Ian Senior	Increase reverse search limit to 10000.
* v1.19 03/08/04  Ian Senior	Make timelimit an option.
* v1.20 04/08/04  Ian Senior	Add text search option.
* v1.21 19/01/06  Doug Adams	Add some more information for web searches.
* v1.22 19/01/06  Doug Adams	Fix linux pointer bug.
* v1.24 17/03/08  Ian Senior	Optionally output decoding information.
*
* $Header: /home/commsa/cmdv/cmss/src/RCS/searchin.cc,v 1.25 2021/05/12 04:58:16 dja Exp $
*******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include "cmss.h"
#include "cirfile.h"
#include "msgdir.h"
#include "kwtab.h"
#include "kwtabdef.h"
#include "ReadMsgText.h"
#include "dispdir.h"
#include "getcommstime.h"
#include "linktime.h"
#include "find_logmsg.h"
#include "logfind.h"

#define MAXTIME	60
#define MAXLOCKS 100
#define MAX_PROCESSES	4
#define MAX_REVERSE     10000

long recpos(long recnum, long lastRecord, long totalRecords);
static void removeCR(char *msg, int *len);
static void LimitCopies();
static int TextMatch(char *textfield);
static void ReportDecodeMsg(int msgnum,time_t rcvtime,char *decode_msg);
static void MsginDecodeMsg(char *decode_msg,int maxlen);
static int ReadJournalRecord(int jnum, char *msgnum,char *ident);
static void GetJournalReports(char *decode_msg,int maxlen);
static int ValidJournalRecord(int jnum);
static void RemoveBraces(char *decode_msg);

struct STN_IDS {
   char* stn_id;
   struct STN_IDS* next;
};

struct MSG_TYPE {
   char* name;
   long num;
   struct MSG_TYPE* next;
};


static CirFile  RepDir, MsginDir, LogdDir, JournalDir;
static MSGDIR		dir;
char*           search_msgtype_name;
static int	cmss_bulletins = FALSE;
static int	print_msg = FALSE;
static int      print_web = FALSE;
static int	print_internal = FALSE;
static int      print_in_rrrcc = FALSE;
static int	rep_search;
//static char	*station_num = NULL;
static struct STN_IDS* stn_ids = NULL;
static struct MSG_TYPE* msg_types = NULL;
static char	*ttaaii = NULL;
static char	*cccc = NULL;
static char     *msg_text = NULL;
static char     *search_rrrcc = NULL;
static time_t   start_time = -1;
static time_t   end_time   = -1;
static int      actual_maxrecs;
static int	maxtime_limit = MAXTIME;
static int      decoding_msgs = FALSE;

static void fatal_error(char *err);
static int  match();
static int  print_it(long recno);
static int  msgtype_num(char *msgtype);
static int  get_actual_maxrecs_in_repdir ();

int main(int argc, char *argv[]) {
   int last_record;
   int	rc, recno, maxrecs, cnt, c, i;
   int	found = 0;
   int	num_msgs = 1;
   int     si = 0;
   char	*filedir;
   struct STN_IDS* curr_stn = NULL;
   struct STN_IDS* new_stn = NULL;
   struct MSG_TYPE* curr_msgtype = NULL;
   struct MSG_TYPE* new_msgtype = NULL;
   char      start_time_string[100];
   char      end_time_string[100], datetime[50];
   time_t	starttime, currtime;
   int		time_range = FALSE, timelimit = FALSE;
   int reverse_list[MAX_REVERSE];
   int num_reverse = 0;
   int print_reverse = FALSE;

   
   recno = -1;
   setbuf(stdout, 0);

   /* REDUCE PRIORITY */
   nice(20);
   
   if (argc < 3) {
      printf("require: home_dir [-t ttaaii] [-c cccc] [-s station_num] [-m msg_type] [-n number_to_be_found] [-S start_time] [-E end_time] [-p] [-x] [-b] [-w] [-r] [-T nn] [-d 'message text'] [-R rrrcc]\n");
      printf("-p = print messages\n");
      printf("-x = print internal information about message\n");
      printf("-b = include CMSS produced bulletins (BUL01)\n");
      printf("-w = Produce web output (HTML)\n");
      printf("-r = print in reverse order - oldest to latest\n");
      printf("-I = print receive circuit\n");
      printf("-T = maximum search time in seconds (default 60 seconds)\n");
      printf("-d = message text to be searched for\n");
      printf("-R = circuit to be searched for\n");
      printf("-X = print decoding information\n");
      exit(1);
   }
   
   rc = chdir(argv[1]);
   if (rc != 0) fatal_error((char *)"chdir failed");
   
   optind = 2;
   while ((c = getopt(argc,argv,"IS:E:t:c:s:m:n:pxbwrT:d:R:X")) != -1) {
      switch(c) {
       case('t'):
	 ttaaii = optarg;
	 break;
       case('c'):
	 cccc = optarg;
	 break;
       case('R'):
	 search_rrrcc = optarg;
	 break;
       case('s'):
	 new_stn = (struct STN_IDS*)malloc(sizeof(struct STN_IDS));
	 new_stn->stn_id = optarg;
	 new_stn->next = NULL;
	 if (curr_stn == NULL){
	    curr_stn = new_stn;
	    stn_ids = curr_stn;
	 }
	 else {
	    curr_stn->next = new_stn;
	    curr_stn = new_stn;
	 }
	 break;
       case('S'):
	 start_time = atol(optarg);
	 break;
       case('E'):
	 end_time = atol(optarg);
	 break;
       case('T'):
	 maxtime_limit = atoi(optarg);
	 break;
       case('w'):
	 print_web = TRUE;
	 break;
       case('r'):
	 print_reverse = TRUE;
	 break;
       case('I'):
	 print_in_rrrcc = TRUE;
	 break;
       case('d'):
	 msg_text = optarg;
	 break;
       case('m'):
	 new_msgtype = (struct MSG_TYPE*)malloc(sizeof(struct MSG_TYPE));
	 new_msgtype->name = optarg;
	 new_msgtype->num  = msgtype_num(optarg);
	 new_msgtype->next = NULL;
	 if (curr_msgtype == NULL){
	    curr_msgtype = new_msgtype;
	    msg_types = curr_msgtype;
	 }
	 else {
	    curr_msgtype->next = new_msgtype;
	    curr_msgtype = new_msgtype;
	 }
	 break;
       case('n'):
	 num_msgs = atoi(optarg);
	 break;
       case('p'):
	 print_msg = TRUE;
	 break;
       case('x'):
	 print_msg = TRUE;
	 print_internal = TRUE;
	 break;
       case('b'):
	 cmss_bulletins = TRUE;
	 break;
       case('X'):
	 decoding_msgs = TRUE;
	 LogdDir.OpenLooker("logd");
	 JournalDir.OpenLooker("journal");
	 break;
       case('?'):
	 printf("Unrecognised argument to cmms_web_extract: -%c\n", optopt);
	 return(1);
       default:
	 printf("Argument not handled correctly in cmms_web_extract: -%c\n", c);
	 return(1);
      }
   }
   
   /*
   if (stn_ids == NULL){
      printf("You must enter a station identifier (WMO number, Ship id, etc)\n");
      return(1);
   }
   */
   
   filedir = (char*)malloc(20);
   if (stn_ids != NULL || msg_types != NULL) {
      strcpy(filedir, "recogdir");
      rep_search = 1;
   } 
   else {
      // filedir = "msgindir";
      strcpy(filedir, "msgindir");
      rep_search = 0;
   }

   // Print out a summary of the data we are searching for
   if (print_web){
      char sep[10] = " ";
      printf("<BR><b>Extracting messages from CMSS:</b>\n");

      curr_stn = stn_ids;
      if ( curr_stn != NULL ) { 
         printf ("<BR>... with station IDs: ");
	 while (curr_stn != NULL){
	    printf("%s<B>%s</B>", sep, curr_stn->stn_id);
	    strcpy(sep, " or ");
	    curr_stn = curr_stn->next;
	    }
	 }

      if (msg_types != NULL){
	 strcpy(sep, " ");
	 printf("<BR>... with message types:");
	 curr_msgtype = msg_types;
	 while (curr_msgtype != NULL){
	    printf("%s<B>%s</B>", sep, curr_msgtype->name);
	    strcpy(sep, " or ");
	    curr_msgtype = curr_msgtype->next;
	 }
      }

      if ( search_rrrcc != NULL ) 
	 printf ("<BR>... with CMSS circuit: <B>%s</B>\n", search_rrrcc);

      if ( ttaaii != NULL ) 
	 printf ("<BR>... with TTAAii: <B>%s</B>\n", ttaaii);

      if ( cccc != NULL ) 
	 printf ("<BR>... with CCCC originator: <B>%s</B>\n", cccc);

      if (start_time != -1 && end_time != -1){
	 strftime(start_time_string,99,"%d-%m-%Y %H:%M.%S %Z", getcommstime(start_time));
	 strftime(end_time_string,99,"%d-%m-%Y %H:%M.%S %Z", getcommstime(end_time));
	 printf("<BR>... where the messages were received between <B>%s</B> and <B>%s</B>\n", start_time_string, end_time_string);
      }
      else if (start_time != -1) {
	 strftime(start_time_string,99,"%d-%m-%Y %H:%M.%S %Z", getcommstime(start_time));
	 printf("<BR>... where the messages were received after <B>%s</B>\n", start_time_string);
      }
      else if (end_time != -1) {
	 strftime(end_time_string,99,"%d-%m-%Y %H:%M.%S %Z", getcommstime(end_time));
	 printf("<BR>... where the messages were received before <B>%s</B>\n", end_time_string);
      }
      printf("<BR>... limited to <B>%d</B> messages\n", num_msgs);
      printf("<BR>... limited to <B>%d</B> seconds search time\n", 
							maxtime_limit);
      printf("<HR SIZE=\"1\">\n");
   }
   
   /* LIMIT NUMBER OF COPIES RUNNING THAT ARE STARTED FROM WEB */
   if (print_web)
      LimitCopies();
   
   rc = RepDir.OpenLooker(filedir);
   if (rc != 0) fatal_error((char *)"OpenLooker failed");
   
   rc = MsginDir.OpenLooker((char *)"msgindir");
   if (rc != 0) fatal_error((char *)"OpenLooker failed");
   
   rc = RepDir.RefreshPointers();
   if (rc != 0) fatal_error((char *)"refresh failed");

   // Assume maxrecords does not change during this short run.
   
   // Allow for trailing zero-fill records in files that have just
   // been expanded. 
   // 'actual_maxrecs' is the number of real data records.
   actual_maxrecs =  get_actual_maxrecs_in_repdir();

   // if we have an end time, then set the recno to just after it
   last_record = RepDir.qNtbw();
   // (woops, ntbw may point to a zero-filled record at the end of the file)
   if (last_record >= actual_maxrecs) actual_maxrecs++;


   if (end_time != -1) {
   
      int before = 1;  // Do not use the zero-th record. Believe me.
      int after;
      int mid;
      
      recno = -1;
      // Assume maxrecords does not change during this short run.
      maxrecs = actual_maxrecs ;
      cnt = 1;
      
      after = maxrecs-1;
      
      // Check if there are any records before the START_TIME
      rc = RepDir.ReadRecord(&dir, recpos(after, last_record, maxrecs));
      if (rc != 0) fatal_error((char *)"read failed");
      
      if (start_time > dir.msgtmrcv){
	 //printf("<BR>Here end_time: %ld   <   before: %ld  (%ld = recpos(%ld, %ld, %ld)\n", end_time, dir.msgtmrcv, recpos(before, last_record, maxrecs), before, last_record, maxrecs);
	 strftime(datetime,99,"%d-%m-%Y %H:%M.%S %Z", getcommstime(end_time));
	 printf("There is no data after the start time: %s\n", datetime);
	 return(0);
      }

      
      // Check if there are any records before the END_TIME
      rc = RepDir.ReadRecord(&dir, recpos(before, last_record, maxrecs));
      if (rc != 0) fatal_error((char *)"read failed");
      
      if (end_time < dir.msgtmrcv){
	 //printf("<BR>Here end_time: %ld   <   before: %ld  (%ld = recpos(%ld, %ld, %ld)\n", end_time, dir.msgtmrcv, recpos(before, last_record, maxrecs), before, last_record, maxrecs);
	 strftime(end_time_string,99,"%d-%m-%Y %H:%M.%S %Z", getcommstime(end_time));
	 strftime(datetime,99,"%d-%m-%Y %H:%M.%S %Z", getcommstime(dir.msgtmrcv));
	 printf("There is no data before the end time: %s (CMSS file starts at %s)\n", end_time_string, datetime);
	 return(0);
      }
      
      // If the last record is before the END_TIME use it
      rc = RepDir.ReadRecord(&dir, recpos(after, last_record, maxrecs));
      if (rc != 0) fatal_error((char *)"read failed");
      
      if (end_time > dir.msgtmrcv){
	 //printf("<BR>Here end_time: %ld   >   after: %ld  (%ld = recpos(%ld, %ld, %ld)\n", end_time, dir.msgtmrcv, recpos(after, last_record, maxrecs), after, last_record, maxrecs);
	 recno = recpos(after, recno, maxrecs);
      }
      
      // if we do not have a recno yet, then search through the circular file
      // for the record with a date just after the one we want
      if (recno == -1){
	 int my_count = 0;
	 while (my_count < 50 && (after - before) > 1){
	    my_count++;
	    
	    mid = before + ((after - before) / 2);
	    
	    rc = RepDir.ReadRecord(&dir, recpos(mid, last_record, maxrecs));
	    if (rc != 0) fatal_error((char *)"read failed");
	    
	    strftime(datetime,99,"%d-%m-%Y %H:%M.%S %Z", getcommstime(dir.msgtmrcv));
	    //printf("<BR>%-5d  %-5ld %s\n", my_count, mid, datetime);

	    if (dir.msgtmrcv <= end_time){
	       before = mid;
	    }
	    else {
	       after = mid;
	    }
	    
	 }
	 recno = recpos(after, last_record, maxrecs);
      }
      
   }

   if (recno == -1) {
      recno = last_record;
      maxrecs =  actual_maxrecs;
      }

   cnt = 1;
   time(&starttime);

   while (found < num_msgs && cnt < maxrecs && !timelimit) {

      if (cnt%5000 == 0) {
	 if (print_web == FALSE)  
	    printf("searched 5000 messages...\n");
	 time(&currtime);
	 if (currtime-starttime > maxtime_limit) 
	    timelimit = TRUE;
         }

      recno--;
      if (recno < 0) 
	 recno = maxrecs-1;
      
      rc = RepDir.ReadRecord(&dir,recno);
      if (rc != 0) 
	 fatal_error((char *)"read failed");
      
      strftime(start_time_string,99,"%d-%m-%Y %H:%M.%S %Z", 
					getcommstime(dir.msgtmrcv));
      if (start_time != -1 && dir.msgtmrcv < start_time) {
	 time_range = 1;
	 break;
	 }
      
      if (match()) {

	 if (print_reverse) {
	    if (num_reverse < MAX_REVERSE)
	       reverse_list[num_reverse++] = recno; 
	    }
	 else if (print_msg) {
	    rc = print_it(recno);
	    if (rc < 0) cnt = maxrecs;
	    }
	 else {
	    strftime(datetime,sizeof(datetime)-1,"%d-%m-%Y %H:%M.%S %Z", 
					   getcommstime(dir.msgtmrcv));
	    if (rep_search)
	       if ( print_in_rrrcc ) 
	          printf(">>>>> Report num: %d. MSGIN num: %d  Received: %s %5.5s\n", 
						   REPORTS_PREFIX+recno, 
						   MSGIN_PREFIX+dir.msgdirno,
						   datetime, dir.msgrrrcc);
	       else 
	          printf(">>>>> Report num: %d. MSGIN num: %d  Received: %s\n", 
						   REPORTS_PREFIX+recno, 
						   MSGIN_PREFIX+dir.msgdirno,
						   datetime);
	    else
	       if ( print_in_rrrcc ) 
	          printf(">>>>> MSGIN num: %d Received: %s %5.5s\n", 
						MSGIN_PREFIX+recno,datetime, dir.msgrrrcc);
	       else 
	          printf(">>>>> MSGIN num: %d Received: %s\n", 
						MSGIN_PREFIX+recno,datetime);
	    }
	 found++;
	 }
      
      if (recno == last_record+1) {
	 break;
         }

      cnt++;
      }
   
   if (print_reverse) {
      for (i=num_reverse-1;i>=0;i--) {
	  RepDir.ReadRecord(&dir,reverse_list[i]);
	  print_it(reverse_list[i]);
	  }
      }

   if (recno == last_record+1 || cnt >= maxrecs ) {
      if (print_web) {
	 printf ("<BR><STRONG>\n");
	 printf("Search terminated because reached end of file.\n");
	 printf ("</STRONG>\n");
	 }
      else 
	 printf ("reached end of file...\n");
      }
   else if (found >= num_msgs ) {
      if (print_web) {
	 printf ("<BR><STRONG>\n");
	 printf("Search terminated because %d msgs found.\n", num_msgs);
	 printf ("</STRONG>\n");
	 }
      else 
	 printf ("found all requested messages...\n");
      }
   else if (time_range) {
      if (print_web) {
	 printf ("<BR><STRONG>\n");
	 printf("Search terminated because search start time reached.\n");
	 printf ("</STRONG>\n");
	 }
      else
	 printf("time limit reached...\n");
      }
   else if (timelimit) {
      if (print_web) {
	 printf ("<BR><STRONG>\n");
	 printf ("Search terminated because search time limit reached.\n");
	 printf ("</STRONG>\n");
	 }
      else
	 printf("time limit reached...\n");
      }
   else {
      if (print_web) {
	 printf ("<BR><STRONG>\n");
	 printf ("Search terminated because ... well it's a mystery.\n");
	 printf ("</STRONG>\n");
	 }
      else 
	 printf ("mysterious search end criteria...\n");
      }

   return (0);
}

      
/*******************************************************************
*			RECPOS
*******************************************************************/
/**   recnum is a virtual record number, 0...max-1 relative to lastrecord.
*     lastrecord is a real record number of the last data written (Ntbw-1).
*     totalrecords is the number of records in the file.
*/
long recpos(long recnum, long lastRecord, long totalRecords){
   long pos;
   
   pos = recnum+lastRecord;
   if (pos >= totalRecords){
      pos = pos - totalRecords;
   }
   return(pos);
}

      
      
/*******************************************************************
*			MATCH
*******************************************************************/
static int match() {
   int	len;
   struct STN_IDS* curr_stn = NULL;
   struct MSG_TYPE* curr_msgtype = NULL;
   
   if (!cmss_bulletins && !strncmp(dir.msgrrrcc,"BUL",3)) return(0);
   
   //if (search_msgtype != 0 && dir.msgtype != search_msgtype)
   //return(0);

   if (stn_ids != NULL) {
      curr_stn = stn_ids;
      while (curr_stn != NULL && strcmp(curr_stn->stn_id, dir.msgstnno)){
	 // printf("<BR>%s === %s\n", curr_stn->stn_id, dir.msgstnno);
	 curr_stn = curr_stn->next;
      }
      if (curr_stn == NULL)
	return(0);
      }

   if (msg_types != NULL){
      curr_msgtype = msg_types;
      while (curr_msgtype != NULL && curr_msgtype->num != dir.msgtype){
	 //printf("<BR>%20s  %d === %d\n", curr_msgtype->name, curr_msgtype->num, dir.msgtype);
	 curr_msgtype = curr_msgtype->next;
      }
      if (curr_msgtype == NULL)
	return(0);
   }

if (!dir.msgwmo && (ttaaii != NULL || cccc != NULL)) return(0);

if (dir.msgwmo) {
   if (ttaaii != NULL) {
      len = strlen(ttaaii);
      if (strncmp(ttaaii,dir.msgorgid,len)) return(0);
      }
   if (cccc != NULL && strncmp(cccc,dir.msgorgid+6,4))
      return(0);
   }

if (search_rrrcc != NULL && strncmp(dir.msgrrrcc,search_rrrcc,strlen(search_rrrcc))) return(0);

if (msg_text != NULL && !TextMatch(msg_text)) return(0);
   
   
return(1);
}

/*******************************************************************
*			TEXT MATCH 
* Read message text and check if it contains required string
*******************************************************************/
static int TextMatch(char *textfield)
{
int	msglen, rc;
char	msg[MAX_MSGSIZE+1];

if (ReadMsgText(&dir,&msglen,msg) != 0) return(FALSE);

msg[msglen] = '\0';

if (strstr(msg,textfield) == NULL)
   rc = FALSE;
else
   rc = TRUE;

return(rc);
}

/*******************************************************************
*			FATAL ERROR
*******************************************************************/
static void fatal_error(char *err)
{
printf("FATAL ERROR: %s\n",err);
exit(1);
}

/*******************************************************************
*			PRINT IT
*******************************************************************/
static int print_it(long recno){
   MSGDIR	dir2;
   int	rc, len = 0;
   char	message[MAX_MSGSIZE+1], hexmsg[MAX_MSGSIZE+1], ident[10];
   struct tm *timeptr;
   char      datetime[100], decode_msg[10000];
   
   hexmsg[0] = '\0';
   decode_msg[0] = '\0';
   
   if (rep_search) {
      rc = MsginDir.ReadRecord(&dir2,dir.msgdirno);
      if (rc != 0) fatal_error((char *)"read failed");

      /* IF MSGIN DIR NEWER THAN REPORTS DIR THEN EOF */
      if (dir2.msgtmrcv > dir.msgtmrcv) return(-1);

      ReadMsgText(&dir2,&len,message);
      sprintf(ident,"%d",dir2.msgdirno);
      if (print_internal) dispdir(ident, &dir2, hexmsg);

      if (decoding_msgs) 
	 ReportDecodeMsg(REPORTS_PREFIX+recno,dir.msgtmrcv,decode_msg);
   }
   else {
      ReadMsgText(&dir,&len,message);
      sprintf(ident,"%d",dir.msgdirno);
      if (print_internal) dispdir(ident, &dir, hexmsg);
      if (decoding_msgs) 
	 MsginDecodeMsg(decode_msg,sizeof(decode_msg));
   }
   
   removeCR(message, &len);

   message[len] = '\0';
   if (print_web == TRUE) {
	 timeptr = getcommstime(dir.msgtmrcv);
	 strftime(datetime,99,"%d-%m-%Y &nbsp; %H:%M.%S %Z",timeptr);

	 printf("<TABLE BORDER=\"0\" WIDTH=\"100%%\" ALIGN=\"CENTER\" CELLSPACING=\"0\" BGCOLOR=\"FFBBBB\">\n");
	 printf("<TR><TD><B>Stn ID:</B>%s<TD><B>Message No.:</B> %ld<TD><B>Report No.:</B> %ld<TD><B>Received:</B> %s\n", dir.msgstnno, dir.msgdirno, recno, datetime);
	 printf("</TABLE>\n");
	 printf("<PRE>%s</PRE><HR SIZE=\"1\">",message);

	 if (decode_msg[0] != '\0')
	    printf("<PRE>%s</PRE><HR SIZE=\"1\">",decode_msg);
   }
   else {
      if (print_internal) {
 	 printf("\n%s\n---------------\n%s\n-------------\n",hexmsg,message);
	 if (decode_msg[0] != '\0')
	    printf("%s\n-------------\n",decode_msg);
	   }
      else {
	 timeptr = getcommstime(dir.msgtmrcv);
	 strftime(datetime,99,"%d-%m-%Y %H:%M.%S %Z",timeptr);
	 if ( print_in_rrrcc ) { 
	    printf("Message No.: %d %5.5s Received: %s\n%s\n-------------\n",MSGIN_PREFIX+dir.msgdirno,dir.msgrrrcc,datetime,message);
	    }
	 else {
	    printf("Message No.: %d Received: %s\n%s\n-------------\n",MSGIN_PREFIX+dir.msgdirno,datetime,message);
	    }
	  if (decode_msg[0] != '\0')
	     printf("%s\n-------------\n",decode_msg);
      }
   }

return(0);
}

/*********************************************************************
*			MSGTYPE NUM
* If the msgtype is not numeric that use the lookup table to try
* to convert it to one.
*********************************************************************/
static int msgtype_num(char *msgtype)
{
int     i, mtype, len;
int     valid = FALSE;

mtype = atoi(msgtype);

if (mtype == 0) {

   /* CHANGE TO UPPER CASE */
   len = strlen(msgtype);
   for (i=0;i<len;i++)
       msgtype[i] = (char)toupper(msgtype[i]);

   /* CHECK IF KEYWORD OR CODENAME SPECIFIED */
   for (i=0; kwtab.tab[i].msgtype != KW_LIMIT && !valid; i++) {
       if (!strcmp(msgtype,kwtab.tab[i].keyword) || !strcmp(msgtype,kwtab.tab[i].codename)) {
          valid = TRUE;
          mtype = kwtab.tab[i].msgtype;
          }
       }
   }

return(mtype);
}



/*********************************************************************
*			REMOVECR
*********************************************************************/
static void removeCR(char *msg, int *len)
{
   int     i, j;
   
   j = 0;
   for (i=0; i < *len; i++) {
      if (msg[i] != '\r') {
	 msg[j] = msg[i];
	 j++;
      }
   }
   
   *len = j;
}


/*********************************************************************
*			get_actual_maxrecs_in_repdir
*********************************************************************/
// This only does something during the short time before the expanded
// file fills to its limit.
//
// To allow for trailing zero-fill records in files that have just
// been expanded, grab the number of the last real data record.
//
// Returns: 'actual_maxrecs' is the number of real data records.

static int  get_actual_maxrecs_in_repdir ()
{
    int maxrec, rc;
    
    maxrec = RepDir.qMaxRecords();

    rc = RepDir.LookRecord (&dir, maxrec-1);
    if (rc != 0) fatal_error((char *)"read failed");

    if (dir.msgtmrcv == 0) {
	// If the last record was all zeros, 
	// then the circular file is still filling up,
	// and the maximum record so far is NTBW.
	maxrec = RepDir.qNtbw();
    }
    return (maxrec);
}

/******************************************************************
*			LIMIT COPIES
* 1. Open lock file (/tmp/searchin.lock)
* 2. If does not exist then create and make large enough.
* 3. Starting from the first character try to lock one character
*    of the file.
* 4. If unable to lock one character after "limit" characters
*    then exit - too many processes running.
******************************************************************/
static void LimitCopies()
{
int	fd, i;
int	nolock = TRUE;
char	buffer[MAXLOCKS];

umask(0);
fd = open("/tmp/searchin.lock",O_CREAT|O_WRONLY,S_IRWXU|S_IRWXG|S_IRWXO);
if (fd >= 0) {
   for (i=0;i<MAXLOCKS;i++)
       buffer[i] = '#';
   write(fd,buffer,MAXLOCKS);
   }
else 
   fd = open("/tmp/searchin.lock",O_WRONLY,S_IRWXU|S_IRWXG|S_IRWXO);

if (fd < 0) return;

/* TRY TO LOCK A RECORD */
for (i=0;nolock && i<MAX_PROCESSES;i++) {
    lseek(fd,i,SEEK_SET);
    if (lockf(fd,F_TLOCK,1) == 0)
       nolock = FALSE;
    }

if (nolock) {
   printf("<BR>Server too busy to perform search - please try again later\n");
   exit(1);
   }
}

/*********************************************************************
*			REPORT DECODE MSG
* Given a report number (msgnum) and the receive time find any related
* decoding messages
*********************************************************************/
static void ReportDecodeMsg(int msgnum,time_t rcvtime, char *decode_msg)
{
char    ident[20];
int	recno;

/* FIND POSITION ON LOG OF MESSAGE RECEIVE TIME (BINARY SEARCH) */
if (logfind(rcvtime,&recno,&LogdDir) == 0) {

   /* SEARCH FOR RELATED LOG MESSAGES */
   sprintf(ident," %d ",msgnum);
   decode_msg[0] = '\0';
   find_logmsg(recno,rcvtime+ONE_HOUR,ident,decode_msg,&LogdDir);
   if (decode_msg[0] != '\0')
      RemoveBraces(decode_msg);
   }

}

/*********************************************************************
*			REMOVE BRACES
* Remove the {} characters that were added as part of the 
* internationalization of CMSS
*********************************************************************/
static void RemoveBraces(char *decode_msg)
{
int	i, len;
int	j = 0;

len = strlen(decode_msg);
for (i=0;i<=len;i++) {
    if (decode_msg[i] != '{' && decode_msg[i] != '}') {
       if (j<i)
	  decode_msg[j] = decode_msg[i];
       j++;
       }
    }

}

/*********************************************************************
*			MSGIN DECODE MSG
* For a given MSGIN message:
* 1. Find the journal records to details of the reports in the message
* 2. Get the decoding messages for each report in the message
*********************************************************************/
static void MsginDecodeMsg(char *decode_msg, int maxlen)
{

/* CHECK IF JOURNALLED YET */
if (dir.msgisdup) {
   strcpy(decode_msg,"*** DUPLICATE MESSAGE ***");
   return;
   }
else if (dir.msgignor) {
   strcpy(decode_msg,"*** IGNORED MESSAGE ***");
   return;
   }
else if (dir.msgjnl == 0) {
   strcpy(decode_msg,"*** MESSAGE NOT PROCESSED YET ***");
   return;
   }
else if (!ValidJournalRecord(dir.msgjnl)) {
   strcpy(decode_msg,"*** DECODING INFORMATION NOT AVAILABLE ***");
   return;
   }

/* GET REPORTS FROM JOURNAL. ALSO CHECK FOR RELATED DECODING MESSAGES */
GetJournalReports(decode_msg,maxlen);

}

/*********************************************************************
*			VALID JOURNAL RECORD
* Check that journal number in the msgdir is still valid - ie that
* the journal record has not been overwritten.
*********************************************************************/
static int ValidJournalRecord(int jnum)
{
int	valid = FALSE;
char	msgnum[80], ident[80];

if (ReadJournalRecord(jnum,msgnum,ident) == 0) {
   if (atoi(msgnum) ==  MSGIN_PREFIX + dir.msgdirno)
      valid = TRUE;
   }

return(valid);
}

/*********************************************************************
*			GET JOURNAL REPORTS
* For the current MSGIN message use the journal to get the report
* numbers of any reports in this message. 
* For each report found get and decode log msgs.
*********************************************************************/
static void GetJournalReports(char *decode_msg, int maxlen)
{
int	more = TRUE;
int	jnum, len;
int	decode_len = 0;
char	msgnum[20], ident[40], report_info[1000], decode_info[1000];

decode_msg[0] = '\0';

/* GET JOURNAL RECORD IMMEDIATELY BEFORE MSGIN ONE */
jnum = dir.msgjnl;
JournalDir.DecrementRecordNo(&jnum);
if (ReadJournalRecord(jnum,msgnum,ident) != 0) return;

/* SEARCH THE JOURNAL FOR REPORT ENTRIES - UP UNTIL A MSGIN ONE */
/* IE SEARCH FOR REPORTS FOR THE CURRENT MSGIN NUMBER */
while (more) {

   /* GET REPORT INFO */
   if (msgnum[0] == '3') {
      ReportDecodeMsg(atoi(msgnum),dir.msgtmrcv,decode_info);
      if (decode_info[0] != '\0')
	 sprintf(report_info,"%s %s\n%s\n",msgnum,ident,decode_info);   
      else
	 sprintf(report_info,"%s %s\n",msgnum,ident);   
      /* APPEND NEW DECODING INFORMATION IF IT FITS INTO STRING */
      len = strlen(report_info);
      if (len + decode_len < maxlen) {
	 strcpy(decode_msg+decode_len,report_info);
	 decode_len+=len;
	 }
      }

   /* READ NEXT JOURNAL RECORD */
   JournalDir.DecrementRecordNo(&jnum);
   if (ReadJournalRecord(jnum,msgnum,ident) != 0 || msgnum[0] == '1')
      more = FALSE;
   } 

}

/*********************************************************************
*			READ JOURNAL RECORD
* Return the fourth and fifth fields from the journal record which
* are the message number and identifier.
*********************************************************************/
static int ReadJournalRecord(int jnum, char *msgnum,char *ident)
{
int     i;
char    jnlrec[250], *ptr;

if (JournalDir.ReadRecord(jnlrec,jnum) != 0 || jnlrec[0]=='\0') 
   return(-1);

/* SKIP FIRST 3 FIELDS */
ptr = strtok(jnlrec," \r\n");
for (i=0;i<3 && ptr != NULL; i++)
    ptr = strtok(NULL," \r\n");

if (ptr == NULL) return(-1);

strcpy(msgnum,ptr);
ptr = strtok(NULL," \r\n");
if (ptr != NULL)
   strcpy(ident,ptr);
else
   ident[0] = '\0';

return(0);
}

