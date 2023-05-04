#ifndef _CMSS_H_
#define _CMSS_H_
/*
*  Some CMSS related pre-processor equates
*  v1.2  01/10/92  Doug Adams  Changed equate ASCII_SPACE back to
*                              numeric value.
*  v1.3  08/10/92  Doug Adams  Removed unused equates and added output
*                              format equates (used in msgformat.cc)
*  v1.4  13/10/92  Doug Adams  Added output message directories file
*         			name - MSGOTDIR.
*  v1.5  19/10/92  Doug Adams  Added maximum sequence number equate,
*				MAXSEQNO.
*  v1.6  19/11/92  Doug Adams  Added CMSS_OUT_LINES equate.
*  v1.7   6/01/93  Doug Adams  Added ifndef macro statements
*  v1.8   5/02/93  Doug Adams  Added service file definition
*  v1.9  17/02/93  Doug Adams  Added SEND header format     
*  v1.10 22/02/93  Doug Adams  Add msgformat field to msgintxt.dat 
*				"record" structure
*  v1.11 02/03/93  Ian Senior  Add the prefix codes for main datasets
*  v1.12 15/03/93  Doug Adams  Remove duplicate definitions          
*  v1.13 21/04/93  Ian Senior  Add prefix for Prepared Message dataset.
*  v1.14 21/05/93  Clive	Include HPSUN.h for conditional compiles
*  v1.15 27/05/93  Doug Adams	Added #define TEXT_INFO_LEN 
*  v1.16 01/07/93  T Crawford  Remove getgroup.h equates  
*  v1.17 02/07/93  Ian Senior  Add REPORTS file equate
*  v1.18 07/07/93  Ian Senior  Add REPORTS file prefix equate
*  v1.19 02/08/93  T Crawford  Add SWITCH file
*  v1.20 10/08/93  Doug Adams  Added MSGINTXT_INFO_LEN equate.
*  v1.21 23/11/92  Ian Senior  change to "little C" comments  
*  v1.22 06/01/94  Doug Adams  Added dtead to the msgintxt.dat structure
*  v1.23 25/01/94  Ian Senior  Add PIDS_DIR
*  v1.24 16/02/94  T Crawford  Add ONE_MINUTE, ONE_HOUR, ONE_DAY, time_item
*  v1.25 05/09/94  I.Senior    Add EOL (WMO Bulletin end of line)
*  v1.26 09/01/95  Doug Adams  Add MAXSEQNO2 equate. 
*  v1.27 09/02/95  Doug Adams  Add CMSSPARMS equate. 
*  v1.28 14/02/95  Ian Senior  Add AIFS address (overflow) directory.
*  v1.29 07/02/96  Ian Senior  Add typedef struct definitions (AIFS/IBM)
*  v1.30 22/05/96  T Crawford  Add MAX_CHAR_MSGSIZE
*  v1.31 09/07/99  Ian Senior  Add MSGOUTHDR, MSGOUT_HDRLEN
*  v1.32 23/11/00  Doug Adams  Add MAX_OFFSET (for msgintxt.ptr)
*  v1.33 15/01/01  Ian Senior  Expand MSGIN, MSGOUT and REPORTS prefixes
*			       to 8 digits.
*  v1.34 09/10/01  Doug Adams  Add FAXLOG file.
*  v1.35 22/04/02  Ian Senior	Move MSGIN text info to msgintxt.h 
*  v1.36 01/07/02  Ian Senior	Add MAX_OUTPUT_LINES
*  v1.37 14/07/03  Ian Senior	Move cmss.params into "etc" dir
*  v1.38 02/02/04  Ian Senior	Add MAX_BINARY_MSGSIZE and change 
*				MAX_OUTPUT_LINES
*  v1.39 21/07/05  Ian Senior	Add AIFS_ADDR_LEN, MAX_ADDR_LEN
*  v1.40 24/07/08  Doug Adams   Add MAX_OUT_MEMORY
*  v1.41 02/03/09  Ian Senior	Add DEFAULT_GTS_FILE_TTL
*  v1.42 12/03/09  Doug Adams   Add CM_MAX_FILENAME_LEN, CM_MAX_EXTENSION_LEN
*  v1.43 07/01/13  Doug Adams   Add parenthesis to MAX_OUT_MEMORY
*  v1.44 30/09/14  Ian Senior   Increase MAX_OUTPUT_LINES to 400
*  v1.45 01/09/15  Ian Senior   Remove hpsun.h
*  v1.46 25/05/17  Ian Senior   Change MAX_OUTPUT_LINES 400 to 500
*/

#define MAX_MSGSIZE	        15000
#define MAX_OUT_MEMORY          (MAX_MSGSIZE+100)
#define MAX_BINARY_MSGSIZE	500000
#define MAX_BUFR_BUL_TEXT	500000
#define MAX_CHAR_MSGSIZE	3800
#define MAX_OUTPUT_LINES        500

#define CM_MAX_FILENAME_LEN     200  /* MAX FILENAME LENGTH INTO CMSS */
#define CM_MAX_EXTENSION_LEN    10   /* MAX FILENAME EXTENSION LEN INTO CMSS */

#define MAXSEQNO	999
#define MAXSEQNO2       9999
#define MAXNOADS        10
#define RRRCC_SIZE      5
#define MSGOUT_HDRLEN	100

#define DEFAULT_GTS_FILE_TTL 24

#define AIFS_ADDR_LEN   9
#define MAX_ADDR_LEN   AIFS_ADDR_LEN+1

#define TRUE		1
#define FALSE		0
#define MAYBE		2

#define ASCII_SOH       1
#define ASCII_SPACE     32
#define ASCII_HYPHEN    45
#define ASCII_CR        "\r"
#define ASCII_LF        "\n"
#define EOL		"\r\r\n"

/* CMSS FILES */
#define MSGIN_TXT_DAT   "msgintxt.dat"   /* message rcv file text     */
#define MSGIN_TXT_PTR   "msgintxt.ptr"   /* message rcv file pointers */
#define LOGFILE         "log"            /* cmss log file text        */
#define FAXLOG          "faxlog"         /* cmss fax log file         */
#define ALARMFILE       "alarm"          /* cmss alarm file text      */
#define REJECT          "reject"         /* cmss alarm file text      */
#define SERVICE         "service"        /* cmss alarm file text      */
#define MSGINDIR        "msgindir"       /* cmss msgin directories    */
#define MSGOUTHDR       "msgouthdr"      /* MSGOUT header file        */
#define MSGOTDIR        "msgotdir"       /* cmss msgout directories   */
#define REPORTS        	"recogdir"       /* cmss reports directories  */
#define SWITCH          "switchdir"      /* cmss switch directories   */
#define ADDRDIR         "addrdir"        /* AIFS address directories  */
#define LOGDIR          "logdir"         
#define CMSSPARMS       "etc/cmss.params"    
#define CMSSPARMS_OLD   "cmss.params"    

#define MSGANAL_PIPE    "msganal.pipe"   /* msganal pipe file         */
#define PIDS_DIR	"pids"		 /* Dir where PIDS are stored */

#define MSGIN_PREFIX	100000000
#define MSGOUT_PREFIX   200000000
#define REPORTS_PREFIX  300000000
#define REJECT_PREFIX   40000
#define SERVICE_PREFIX  50000
#define PREPFILE_PREFIX 60000
#define LOGDIR_PREFIX   8000000
#define SWITCH_PREFIX   90000


#define ONE_MINUTE	60
#define ONE_HOUR	60 * ONE_MINUTE
#define ONE_DAY		24 * ONE_HOUR

/*  TIME structure used by METAR ACARS AMDAR etc.  */
typedef struct time_item {
   short day;
   short hour;
   short min;
   } time_item;

#endif
