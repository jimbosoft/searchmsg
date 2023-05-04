#ifndef _MSGDIR_H_
#define _MSGDIR_H_

/*********************************************************************
//							       msgdir.h
// v1.1 12/08/92 Doug Adams  Original - plus development modifications.
// v1.2 14/09/92 Doug Adams  Added msgisdup flag (duplicate elim.)
// v1.3 16/09/92 Doug Adams  Added msgdir length equate MSGDIRLEN.
// v1.4 29/09/92 Doug Adams  Added message resend flag msgrsend.   
// v1.5 21/10/92 Doug Adams  Removed msgdirno - not needed.        
// v1.6 21/10/92 Doug Adams  I.S. changed his mind, add msgdirno.  
// v1.7 30/10/92 Doug Adams  Added msgnotxt flag.
// v1.8 05/11/92 Ian Senior  Change comments to make usable by "C"
// v1.9 06/01/93 Doug Adams  Added #ifndef macro statements 
//                           Changed msgaddrs to MAXNOADS array elements
// v1.10 06/01/93 Doug Adams #include cmss.h
// v1.11 07/01/93 Doug Adams Add msgaftn flag (AFTN addr in msgaddr)
//			     Add cmss/aftn union for msgaddr
// v1.11 07/01/93 Doug Adams Add msgaftn flag (AFTN addr in msgaddr)
// v1.12 27/05/93 Doug Adams Add msgrecog and msgtxthd.             
// v1.13 01/07/93 T Crawford change msgrecog to msgrprot            
// v1.14 02/07/93 T Crawford increase msgstnno to 10 chars          
// v1.15 12/11/93 T Crawford add msgausfm          
// v1.16 26/11/93 T Crawford add msgblerr          
// v1.17 02/12/93 T Crawford add msgcor          
// v1.18 07/01/94 Doug Adams add msgcor          
// v1.19 19/01/94 T Crawford increase msgtxthd to 32 chars
// v1.20 10/05/94 T Crawford add msgnonmc        
// v1.21 07/06/94 T Crawford add msgcont        
// v1.22 04/08/94 T Crawford add msgnilrp        
// v1.23 04/08/94 I Senior   add msgmetdt2 - second data/time field
// v1.24 27/10/94 Doug Adams add msgaftnin - message from AFTN flag.
// v1.25 09/11/94 I Senior   add msgjnl - journal record no. of msg
// v1.26 14/02/95 I Senior   add AIFS addressing
// v1.27 22/03/95 I Senior   add time field "msgtmaddr" (AIFS).
// v1.28 28/03/95 I Senior   Allow files from AIFS - "msgfile" flag.
// v1.29 12/05/95 Doug Adams Add msgstx_aftn field.
// v1.30 30/08/95 T Crawford add msgsttype flag
// v1.31 29/11/95 T Crawford add TXTHD_SIZE
// v1.32 19/01/96 T Crawford need 2 bits for msgsttype
// v1.33 23/01/96 Doug Adams Added msgnodb flag.
// v1.34 07/02/96 I Senior   Add typedef struct defn (AIFS/IBM)
// v1.35 12/02/96 I Senior   Change MSGDIRLEN to 220 (gulp)
// v1.36 29/03/96 Doug Adams Added flag msgsplit.
// v1.37 03/07/96 Doug Adams Added flag msgview.
// v1.38 14/10/96 Doug Adams Added msgfname field.
// v1.39 24/10/96 Doug Adams Added msgftype field, and definitions.
// v1.40 21/01/97 Ian Senior Add FT_AREA (satellite pix - AREA file)
// v1.41 03/03/97 Doug Adams Added msgfext field - file extension.
// v1.42 09/04/97 Doug Adams Added FT_G3F (Group III Fax)
// v1.43 16/04/97 Doug Adams Increase msgftype to 11 characters 
//				and move it within the structure.
// v1.44 19/05/97 Doug Adams Add ALPHA_ADDRESSES and MIXED_ADDRESSES
// v1.45 18/06/97 Ian Senior Increase msgfname to 12 characters 
// v1.46 10/09/97 Ian Senior Add msgnorec flag (don't recognize flag)
// v1.47 11/09/97 Doug Adams Added msggtshd flag (used gts_header_tbl).
// v1.48 15/09/97 Doug Adams Add new field, msgprodid.
// v1.49 11/11/97 T Crawford add msgvalrp
// v1.50 23/09/98 Ian Senior For files overlay msgt4code over msgstnno.
// v1.51 21/10/98 Doug Adams Changed documentation for msgfname
// v1.52 12/01/99 Ian Senior Add comment/warning for msgstnno.
// v1.53 01/02/99 Ian Senior Add msg_cctaddr flag - circuit address
// v1.54 23/05/00 T Crawford add toalert, metalert, avalert
// v1.55 18/01/01 Ian Senior Change msgsseq from short to int.
//       22/01/01 Ian Senior Change msgrseq from short to int.
// v1.56 12/10/01 Ian Senior Add msgorgid2 for originator address
// v1.57 24/04/02 Ian Senior Add index_txt which is the text file number
// v1.58 16/02/05 Ian Senior Add msgstx_orig. Points to AFTN originator
//				line.
// v1.59 23/02/05 Ian Senior Add AD_EXT_CMSS, AD_EXT_AFTN.
// v1.70 04/03/05 Ian Senior Add AFTN_ADDR_LEN
// v1.71 21/07/05 Ian Senior Move AIFS_ADDR_LEN to cmss.h
// v1.72 01/09/05 Ian Senior Add FT_GRIB and FT_BUFR.
// v1.73 11/12/06 Ian Senior Change to "C" style comments
// v1.74 20/11/08 Ian Senior Add flag for GTS files - "gtsfilehdr".
// v1.75 25/08/09 Ian Senior Add more space for linux CMSS where
//			     "MSGDIR_EXTENDED" compile option is used.
// v1.76 01/04/10 Ian Senior Add AWS flags - msg_undertest,msg_test_stn
//
// WARNING - WHEN CHANGING THIS STRUCTURE, ENSURE THAT THE OVERALL
//           LENGTH OF THE STRUCTURE REMAINS CORRECT
//
*********************************************************************/

#include	<sys/types.h>
#include        "cmss.h"

#define  MAX_AIFS_DIRADDR 4
#define  MAX_AIFS_ADDR  200
#define  AFTN_ADDR_LEN 	8

#define  AD_CMSS 	0
#define  AD_AFTN 	1
#define  AD_AIFS 	2
#define  AD_EXT_AIFS 	3
#define  AD_EXT_CMSS 	4
#define  AD_EXT_AFTN 	5

#define  ALPHA_ADDRESSES 0
#define  MIXED_ADDRESSES 1

#define  FT_NOTFILE     0
#define  FT_PS          1
#define  FT_ASCII       2
#define  FT_GIF         3
#define  FT_EDT         4
#define  FT_UNKNOWN     5
#define  FT_AREA        6
#define  FT_G3F         7
#define  FT_GRIB        8
#define  FT_BUFR        9

#define	TXTHD_SIZE	32

typedef struct MSGDIR {                      
	/* Flags byte 0 (C bit fields) */
	unsigned int msgignor : 1;     /* X'80' message ignored flag  */
	unsigned int msgrejct : 1;     /* X'40' message rejected flag */
	unsigned int msgttf   : 1;     /* X'20' METAR derived from TTF*/
	unsigned int msgwmo   : 1;     /* X'10' WMO format message    */
	unsigned int msgbinry : 1;     /* X'08' binary message flag   */
	unsigned int msgnohdr : 1;     /* X'04' no header-free format */
	unsigned int msgisdup : 1;     /* X'02' message is duplicate  */
	unsigned int msgrsend : 1;     /* X'01' message is a resend   */
	/* Flags byte 1 (C bit fields) */
	unsigned int msgnotxt : 1;     /* X'80' no text in message    */
	unsigned int msgaftn  : 1;     /* X'40' AFTN addresses        */
	unsigned int msgrport : 1;     /* X'20' Report inside message */
	unsigned int msgnosw  : 1;     /* X'10' don't switch rep/msg  */
	unsigned int msgausfm : 1;     /* X'08' Australian format msg */
	unsigned int msgblerr : 1;     /* X'04' Australian format msg */
	unsigned int msgcor   : 1;     /* X'02' CORrected message     */
	unsigned int msgnonmc : 1;     /* X'01' don't switch to NMC   */
	/* Flags byte 2 (C bit fields) */
	unsigned int msgcont  : 1;     /* X'80' content switch        */
	unsigned int msgnilrp : 1;     /* X'40' NIL report flag       */
	unsigned int msgaftnin : 1;    /* X'20' AFTN msg flag         */
	unsigned int msgfile   : 1;    /* X'10' FILE flag             */
	unsigned int msgsttype : 2;    /* X'0C' LAND/MOBILE flag      */
	unsigned int msgnodb   : 1;    
	unsigned int msgsplit  : 1;    /* AFTN Split message flag     */

	/* Spare flag bits  (C bit fields) */
	unsigned int msgview : 1;      /* POSTSCRIPT file message     */
	unsigned int msgnorec : 1;     /* Don't recognize message     */
	unsigned int msggtshd : 1;     /* Used gts_header_tbl         */
	unsigned int msgvalrp : 1;     /* report is valid             */
	unsigned int msg_cctaddr : 1;  /* All msgs from this circuit as sent to an address via lines_in */
	unsigned int toalert : 1;      /* TO alert                    */
	unsigned int metalert : 1;     /* SPOC aler                   */
	unsigned int avalert : 1;      /* PO2 alert                   */

	/* MSGDIR variables */
	short        msgstx;           /* total length of msg header  */
	short        msgtrlen;         /* length of msg trailer       */
	int          msgrseq;          /* rcvd sequence number        */
	short        msgformt;         /* message format              */
	short        msgnoads;         /* number of addresses         */
	short        msgtype;          /* message type (AAXX, etc.)   */
	short        msgstxof;         /* length upto abbrev. header  */
	unsigned int msglen;           /* length of message           */
	unsigned int msgoffst;         /* text file byte offset       */
	unsigned int msgdirno;         /* block number of directory   */
	unsigned int msgrepno;         /* block no on REPORTS file    */
	union {
	   unsigned int msgjnl;	       /* journal rec no. of msg      */
	   unsigned int msgsseq;       /* send sequence number        */
	   };
	unsigned char msgaddrs_type;   /* "msgaddrs" type             */
	unsigned int gtsfilehdr : 1;   /* File contains TTAAII line */
	unsigned int msg_undertest : 1;   /* AWS is being tested */
	unsigned int msg_test_stn : 1;    /* This is a test station */
	unsigned int msg_spare1 : 5;   
	char	     msgmetdt2[6];     /* Message Date/Time DDHHMM    */
				       /* From WMO header             */
	char         msgdtead[16];     /* Dte address of msg sender   */
	time_t       msgtmrcv;         /* Time of message rcv    */
	time_t       msgtmsnd;         /* Time of message sent   */
	time_t	     msgtmaddr;	       /* Time stamp of extended addr */
				       /* (ext_aifs)		 */
        short        msgstx_aftn;      /* Special AFTN pointer   */
	short        index_txt;        /* TEXT file number       */
	char         msgprty[1];       /* message priority       */
	char         msgrrrcc[5];      /* receive route id       */
	char         msgsrrcc[5];      /* send route identifier  */
	char         msgorgid[10];     /* origin id or message   */
	char         msgretrd[3];      /* retard indicator       */
	char         msgmetdt[6];      /* meteorlogical time     */
	/* WARNING: the msgmetdt field contains the WMO header   */
	/* date for messages and the report date for reports !!! */
	union {
	   char      cmss[MAXNOADS][4];     /* cmss 4 char addrs */
	   char      aftn[MAXNOADS/2][AFTN_ADDR_LEN]; 
	   char      aifs[MAX_AIFS_DIRADDR][AIFS_ADDR_LEN];
	   char	     ext_aifs[MAXNOADS][4];
	   } msgaddrs;
	/* WARNING: msgstnno is NOT NULL TERMINATED FOR ID CODES
	   THAT ARE 10 CHARS LONG */
	union {
	   char         msgstnno[10];	      /* station number    */
	   char		msgt4code[10];        /* WMO chart code    */
	   };
	char         msgtxthd[TXTHD_SIZE];    /* recogn uses this  */
	char	     msgprodid[5];	      /* Aifs product id   */
	char	     msg_spare2;
	short	     msgstx_orig;    /* start of AFTN originator line */
	char	     msg_spare3;
	char         msgfext[5];	      /* Product extension */

	/* WARNING: msgfname is only used if filename can be stored
	   there - otherwise filename is taken from FILEORGNAME
	   field in message stub.  See msgdir_getfname.cc 
	*/

	union {
	   char         msgfname[12];	      /* File name         */
	   char	        msgorgid2[12];
	   };
	short	     msgftype;                /* Type of file      */
        /* ADDITIONAL SPACE FOR LINUX VERSION OF CMSS */
#ifdef MSGDIR_EXTENDED
	char  spare_extended[52];
#endif
	} MSGDIR; 

#endif
