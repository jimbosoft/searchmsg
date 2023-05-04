static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/RWaddresses.cc,v 1.20 2014/12/01 04:08:11 ies Exp $" ;
/*<S> RW_ADDRESSES 
************************************************************************
*				RW_ADDRESSES
* FUNCTION	To allow CMSS/AIFS addresses to be stored and to be read
*
* SUMMARY 	int RWwriteAddr(MSGDIR *dir,char addrs[][AIFS_ADDR_LEN], int num_addr);
*		Store the given addresses (addrs). Store them in the
*		Msgdir if possible, otherwise in the "overflow" file.
*
*		void RWWriteString(MSGDIR *dir, char *str)
*		Copy the string of addresses (str) into the Msgdir.
*
*		void RWReadString(MSGDIR *dir, char *str, int len)
*		Read the addresses for the Msgdir and return them in
*		a string.
*
*		void RWReadList(MSGDIR *dir, char *addr[][10],
*					int maxlist, int *num_addr);
*		Read the addresses for the Msgdir and return them in
*		an array (addr)
*
*		void RWcopyaddr(MSGDIR *dir)
*		Used for AIFS "overflow" addresses. Called during
*		"resends" to copy the addresses to the end of the
*		overflow file (to minimise chance of them being 
*		overwritten).
*
*
* DETAILS
* AIFS addresses are up to 9 characters. There can be up to 200 of them.
* Because the CMSS Msgdir can only store 4 of these addresses, they
* must be stored on a separate file if there are more than 4 addresses.
* The separate "extension" file is circular and the record length is
* the same as for Msgdirs.
*
* If there are more than 4 addresses then "msgaddrs" in Msgdir points
* to the "extension" records rather than contains the actual addresses.
*
* NOTE: To ensure that the "extension" record is still valid (hasn't
*       been overwritten), both the Msgdir and the extension record
*	contain a matching time stamp (msgtmaddr).
*
*
* V0.00 14/02/95 Ian Senior	Original
* V1.2  21/02/95 Ian Senior     Add RWReadList
* V1.3  16/03/95 Ian Senior     Add RWWriteString
* V1.4  16/03/95 Ian Senior     Fix bug in ReadExtDirList.
* V1.5  22/03/95 Ian Senior     Add RWcopyaddr.
* V1.6  24/03/95 Ian Senior     Fix bug in "write_rec".
* v1.7  13/07/98 Ian Senior     Add RWwriteAddr.
* v1.10 09/03/01 Ian Senior     1. Use AIFS_ADDR_LEN+1 (strings)
*				2. Allow for AFTN addresses in RWwriteAddr
* v1.11 15/05/01 Ian Senior     Fix bug in length of temporary string.
* v1.12 07/12/01 Ian Senior     Add new read list function RWReadList2
* v1.13 23/02/05 Ian Senior     Change RWwriteAddr to allow CMSS 
*				to use "overflow directories" too.
* v1.14 24/02/05 Ian Senior     Add RWappendAddr to append an address.
* v1.15 25/02/05 Ian Senior     Make sure AD_xxx flag correctly set.
* v1.16 04/03/05 Ian Senior     Set AD_AFTN in RWWriteString if required
* v1.17 05/05/05 Ian Senior     Add RWwriteOneAddr
* v1.18 19/11/13 Ian Senior     Remove xcmss.h and outq.h
* v1.19 28/11/14 Ian Senior     Use strtok_r (reentrant) for the new
*				threads based GUI
$Header: /home/commsa/cmdv/cmss/src/RCS/RWaddresses.cc,v 1.20 2014/12/01 04:08:11 ies Exp $
***********************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "msgdir.h"
#include "cirfile.h"
#include "alarm.h"
#include "AifsExtAddr.h"
#include "RWaddresses.h"

static int  store_on_disc(MSGDIR *dir,char addrs[][AIFS_ADDR_LEN+1], int num_addr);
static int  write_rec(MSGDIR *dir,char addrs[][AIFS_ADDR_LEN+1], int *pos, int num_addr);
static void ReadDirString(MSGDIR *dir, char *str);
static void ReadExtDirString(MSGDIR *dir, char *str, int len);
static int  ReadRec(ADDR_DIR *ext_dir, int recno, MSGDIR *dir);
static int ProcessExtDir(ADDR_DIR *ext_dir, char *str, int len);
static int ReadExtDirList(MSGDIR *dir,char addr[][AIFS_ADDR_LEN+1], int maxaddr);
static int ReadDirList(MSGDIR *dir, char addr[][AIFS_ADDR_LEN+1]);
static void RWcmssWrite(MSGDIR *dir, char addrs[][AIFS_ADDR_LEN+1], int num_addr);
static void RWaftnWrite(MSGDIR *dir, char addrs[][AIFS_ADDR_LEN+1], int num_addr);
static void RWaifsWrite(MSGDIR *dir,char addrs[][AIFS_ADDR_LEN+1], int num_addr);

static CirFile  DirWrite, DirRead;
static int	open_write = FALSE;
static int	open_read = FALSE;

/**********************************************************************
*			RWwriteAddr
* ** Externally callable ***
* Store addresses in the Msgdir.
* NOTE: assumes dir field "msgaddrs_type" is already set.
**********************************************************************/
int RWwriteAddr(MSGDIR *dir,char addrs[][AIFS_ADDR_LEN+1], int num_addr)
{
int	rc = 0;

switch (dir->msgaddrs_type) {
    case AD_CMSS:
    case AD_EXT_CMSS:
      if (num_addr <= MAXNOADS)
	 RWcmssWrite(dir,addrs,num_addr);
      else 
	 rc = store_on_disc(dir,addrs,num_addr);
      break;
    case AD_AIFS:
    case AD_EXT_AIFS:
      if (num_addr <= MAX_AIFS_DIRADDR)
         RWaifsWrite(dir,addrs,num_addr);
      else 
	 rc = store_on_disc(dir,addrs,num_addr);
      break;
    case AD_AFTN:
    case AD_EXT_AFTN:
      if (num_addr <= MAXNOADS/2) 
	 RWaftnWrite(dir,addrs,num_addr);
      else 
	 rc = store_on_disc(dir,addrs,num_addr);
      break;
    }

return(rc);
}

/**********************************************************************
*			RWwriteOneAddr
* ** Externally callable ***
* Store addresses in the Msgdir.
* NOTE: assumes dir field "msgaddrs_type" is already set.
**********************************************************************/
int RWwriteOneAddr(MSGDIR *dir,char *addr)
{
char    addrlist[1][AIFS_ADDR_LEN+1];
int	rc;

strcpy(addrlist[0],addr);
rc = RWwriteAddr(dir,addrlist,1);
return(rc);
}

/**********************************************************************
*			RWappendAddr
* ** Externally callable ***
* Append address to the Msgdir list.
*
* If this list is "extended" then use the brutal method of reading
* the list and writing it to a new location (hopefully only happens
* rarely)
**********************************************************************/
int RWappendAddr(MSGDIR *dir,char *addr)
{
int	numaddr, rc = 0;
int	extended_addr = FALSE;
char    addrlist[MAX_AIFS_ADDR][AIFS_ADDR_LEN+1];

switch (dir->msgaddrs_type) {
    case AD_CMSS:
      if (dir->msgnoads < MAXNOADS) {
	  strncpy(dir->msgaddrs.cmss[dir->msgnoads],addr,4);
	  dir->msgnoads++;
	  }
      else 
	 extended_addr = TRUE;
      break;
    case AD_AIFS:
      if (dir->msgnoads < MAX_AIFS_DIRADDR) {
	  strncpy(dir->msgaddrs.aifs[dir->msgnoads],addr,AIFS_ADDR_LEN);
	  dir->msgnoads++;
	  }
      else 
	 extended_addr = TRUE;
      break;
    case AD_AFTN:
      if (dir->msgnoads < MAXNOADS/2) {
	  strncpy(dir->msgaddrs.aftn[dir->msgnoads],addr,8);
	  dir->msgnoads++;
	  }
      else 
	 extended_addr = TRUE;
      break;
    default:
	 extended_addr = TRUE;
	 break;
    }

/* IF EXTENDED ADDRESSING REQUIRED THEN READ OLD LIST, */
/* APPEND NEW ADDRESS AND THEN WRITE UPDATED LIST */
if (extended_addr) {
   RWReadList(dir,addrlist,MAX_AIFS_ADDR,&numaddr);
   if (numaddr < MAX_AIFS_ADDR) {
      strcpy(addrlist[numaddr++],addr);
      rc = RWwriteAddr(dir,addrlist,numaddr);
      }
   }

return(rc);
}

/**********************************************************************
*			RWaifsWrite
* Copy the given AIFS addresses into the Msgdir.
**********************************************************************/
static void RWaifsWrite(MSGDIR *dir,char addrs[][AIFS_ADDR_LEN+1], int num_addr)
{
int	i;

for (i=0;i<num_addr;i++) 
    strncpy(dir->msgaddrs.aifs[i],addrs[i],AIFS_ADDR_LEN);

dir->msgnoads = num_addr;
dir->msgaddrs_type = AD_AIFS;
}

/**********************************************************************
*			STORE ON DISC
* There are too many addresses to store in the Msgdir. Instead store
* them in a separate circular file and point the Msgdir to them.
* 1. Open the "extension" address file (if not already open).
* 2. Copy the addresses into this file and update the original Msgdir.
**********************************************************************/
static int store_on_disc(MSGDIR *dir,char addrs[][AIFS_ADDR_LEN+1], int num_addr)
{
int	retcode = 0;
int	pos;

dir->msgnoads = 0;

;// OPEN EXTENSION FILE
if ( !open_write && DirWrite.OpenWriteOnly (ADDRDIR) < 0 ) {
   Alarm("EXT000A: UNABLE TO OPEN MESSAGE ADDRESS FILE");
   return(-1);
   }

open_write = TRUE;
time(&dir->msgtmaddr);

if (dir->msgaddrs_type == AD_AIFS)
   dir->msgaddrs_type = AD_EXT_AIFS;
else if (dir->msgaddrs_type == AD_CMSS)
   dir->msgaddrs_type = AD_EXT_CMSS;
else if (dir->msgaddrs_type == AD_AFTN)
   dir->msgaddrs_type = AD_EXT_AFTN;

/* COPY ADDRESSES INTO SEPARATE FILE */
pos = 0;
while ( pos < num_addr && retcode==0) {
     if (dir->msgnoads == MAXNOADS) {
	Alarm("EXT000A: TOO MANY EXTENDED ADDRESSES");
	retcode = -1;
	}
     else
	retcode = write_rec(dir,addrs,&pos, num_addr);
     }

if (retcode != 0) dir->msgnoads = 0;

return(retcode);
}

/**********************************************************************
*			WRITE REC
* Copy the next group of addresses into an "extension record" and point
* the Msgdir to this record.
* RETURN:  "pos" - position in "addrs" list of next address to be
* processed.
**********************************************************************/
static int write_rec(MSGDIR *dir,char addrs[][AIFS_ADDR_LEN+1], int *pos, int num_addr)
{
ADDR_DIR	ext_dir;
int		cnt = 0;
int		spos, epos, retcode, numw;

;// STORE INFO IN EXTENSION DIR
ext_dir.msgtmaddr = dir->msgtmaddr;

;// COPY ADDRESSES INTO EXTENSION DIR
spos = *pos;
if (num_addr - spos > MAX_AIFS_EXTADDR)
   epos = spos + MAX_AIFS_EXTADDR;
else
   epos = num_addr;

for ( ; spos < epos; spos++) {
    strncpy(ext_dir.aifs[cnt],addrs[spos],AIFS_ADDR_LEN);
    cnt++;
    }

ext_dir.msgnoads = cnt;
*pos = epos;

;// WRITE TO DISC
retcode = DirWrite.WriteUpdate(&ext_dir);
if (retcode != 0) {
   Alarm("EXT000A: WRITE TO MESSAGE ADDRESS FILE FAILED");
   return(-1);
   }

;// UPDATE MSGDIR
numw = DirWrite.qLrnw();
memcpy( dir->msgaddrs.ext_aifs[dir->msgnoads],(char *)&numw,4);
dir->msgnoads++;

return(0);
}

/**********************************************************************
*			RWReadString
* ** Externally callable ***
* Copies the Msgdir addresses into the given string (str).
* Normally these addresses are in the Msgdir however for 
* "Extended" the Msgdir contains pointers to the addresses which
* are stored on a separate file.
**********************************************************************/
void RWReadString(MSGDIR *dir, char *str, int len)
{

str[0] = '\0';

if (dir->msgaddrs_type == AD_EXT_AIFS || dir->msgaddrs_type == AD_EXT_CMSS || dir->msgaddrs_type == AD_EXT_AFTN)
   ReadExtDirString(dir,str,len);
else
   ReadDirString(dir,str);

}

/**********************************************************************
*			READ DIR STRING
* Copy the addresses from the Msgdir into the given string.
**********************************************************************/
static void ReadDirString(MSGDIR *dir, char *str)
{
int	i;

if (dir->msgaftn) dir->msgaddrs_type = AD_AFTN;

for (i=0;i<dir->msgnoads;i++) {
    strcat(str," ");
    switch (dir->msgaddrs_type) {
	case AD_AFTN:
	      strncat(str,dir->msgaddrs.aftn[i],8);
	      break;
	case AD_AIFS:
	      strncat(str,dir->msgaddrs.aifs[i],AIFS_ADDR_LEN);
	      break;
	default:
	      strncat(str,dir->msgaddrs.cmss[i],4);
	      break;
        }
   }
}

/**********************************************************************
*			READ EXTENDED DIR STRING
* Copy the destination addresses into the given string. These
* addresses are stored in a separate file pointed to by the Msgdir.
* 1. For each address pointer in the Msgdir 
*    - Read the actual address record (ReadRec).
*    - Copy the addresses into the string(ProcessExtDir).
**********************************************************************/
static void ReadExtDirString(MSGDIR *dir, char *str, int len)
{
int		i, recno;
int		error = FALSE;
ADDR_DIR	ext_dir;

for (i=0;i<dir->msgnoads && !error;i++) {
    memcpy((char *)&recno,dir->msgaddrs.ext_aifs[i],4);
    error = ReadRec(&ext_dir,recno,dir);
    if (!error) error = ProcessExtDir(&ext_dir,str,len);
    }

}

/**********************************************************************
*			PROCESS EXT DIR
* Copy the addresses from the extension directory into a string (str) 
* separated by spaces.
* If there are too many addresses to fit in the string then store
* "++++" at the end of the string to signify "more data".
**********************************************************************/
static int ProcessExtDir(ADDR_DIR *ext_dir, char *str, int len)
{
int	error = FALSE;
int	slen, i, alen;
char	addr[AIFS_ADDR_LEN+1];

slen = strlen(str);

for (i=0; i<ext_dir->msgnoads && !error; i++) {

    strncpy(addr,ext_dir->aifs[i],AIFS_ADDR_LEN);
    addr[AIFS_ADDR_LEN] = '\0';
    alen = strlen(addr);
    if (slen+alen+1 < len) {
       strcat(str," ");
       strcat(str,addr);
       slen += alen+1;
       }
    else
       error = TRUE;
   
    }

;// ADD "++++" IF STRING FULL
if (error) {
   if (len - slen < 5) {
      for (i=slen-1; i>0 && str[i] != ' '; i--);
      str[i] = '\0';
      }
    strcat(str," ");
    strcat(str,"++++");
    }

return(error);
}

/**********************************************************************
*			RWReadList
* ** Externally callable ***
* Copies the Msgdir addresses into the given address list.
* Normally these addresses are in the Msgdir however for 
* "Extended" the Msgdir contains pointers to the addresses which
* are stored on a separate file.
**********************************************************************/
void RWReadList(MSGDIR *dir, char addr[][AIFS_ADDR_LEN+1], int maxlist, int *num_addr)
{

if (dir->msgaddrs_type == AD_EXT_AIFS || dir->msgaddrs_type == AD_EXT_CMSS || dir->msgaddrs_type == AD_EXT_AFTN) {
   *num_addr = ReadExtDirList(dir,addr,maxlist);
   if (*num_addr < dir->msgnoads)
      Alarm("EXT000A WARNING: ADDRESS LIST TOO LONG");
   }
else
   *num_addr = ReadDirList(dir,addr);

}

/**********************************************************************
*			RWReadList2
* ** Externally callable ***
* Copies the Msgdir addresses into the given address list.
* Normally these addresses are in the Msgdir however for 
* "Extended" the Msgdir contains pointers to the addresses which
* are stored on a separate file.
**********************************************************************/
void RWReadList2(MSGDIR *dir, char addr[][AIFS_ADDR_LEN+1], int maxlist, int *num_addr)
{

if (dir->msgaddrs_type == AD_EXT_AIFS || dir->msgaddrs_type == AD_EXT_CMSS || dir->msgaddrs_type == AD_EXT_AFTN) 
   *num_addr = ReadExtDirList(dir,addr,maxlist);
else
   *num_addr = ReadDirList(dir,addr);

}

/**********************************************************************
*			READ DIR LIST
* Copy the addresses from the Msgdir into the given array.
**********************************************************************/
static int ReadDirList(MSGDIR *dir, char addr[][AIFS_ADDR_LEN+1])
{
int	i;

for (i=0;i<dir->msgnoads;i++) {
    switch (dir->msgaddrs_type) {
	case AD_AFTN:
	      strncpy(addr[i],dir->msgaddrs.aftn[i],8);
	      addr[i][8] = '\0';
	      break;
	case AD_AIFS:
	      strncpy(addr[i],dir->msgaddrs.aifs[i],AIFS_ADDR_LEN);
	      addr[i][AIFS_ADDR_LEN] = '\0';
	      break;
	default:
	      strncpy(addr[i],dir->msgaddrs.cmss[i],4);
	      addr[i][4] = '\0';
	      break;
        }
   }

return(dir->msgnoads);
}

/**********************************************************************
*			READ EXTENDED DIR LIST
* Copy the destination addresses into the given array. These
* addresses are stored in a separate file pointed to by the Msgdir.
* 1. For each address pointer in the Msgdir 
*    - Read the actual address record (ReadRec).
*    - Copy the addresses into the array(ProcessExtDir).
**********************************************************************/
static int ReadExtDirList(MSGDIR *dir,char addr[][AIFS_ADDR_LEN+1], int maxaddr)
{
int		i, j, recno;
int		num_addr = 0;
int		error = FALSE;
ADDR_DIR	ext_dir;

for (i=0;i<dir->msgnoads && !error;i++) {
    memcpy((char *)&recno,dir->msgaddrs.ext_aifs[i],4);
    error = ReadRec(&ext_dir,recno,dir);
    if (!error) {
       for (j=0;j<ext_dir.msgnoads && num_addr<maxaddr; j++) {
	   strncpy(addr[num_addr],ext_dir.aifs[j],AIFS_ADDR_LEN);
	   addr[num_addr][AIFS_ADDR_LEN] = '\0';
	   num_addr++;
	   }
       }
    }

return(num_addr);
}

/**********************************************************************
*			READ REC
* Read the required address extension record (recno).
* If the time stamp in the extension record doesn't match that in the
* Msgdir then raise an alarm - the record has been overwritten.
**********************************************************************/
static int ReadRec(ADDR_DIR *ext_dir, int recno, MSGDIR *dir)
{
int	retcode;

;// OPEN EXTENSION FILE
if ( !open_read && DirRead.OpenLooker(ADDRDIR) < 0 ) {
   Alarm("EXT000A: UNABLE TO OPEN MESSAGE ADDRESS FILE");
   return(-1);
   }

open_read = TRUE;

;// READ RECORD
retcode = DirRead.ReadRecord(ext_dir,recno);
if (retcode != 0) {
   Alarm("EXT000A: READ FROM MESSAGE ADDRESS FILE FAILED");
   return(-1);
   }

;// CHECK THAT RECORD IS STILL VALID
if (ext_dir->msgtmaddr != dir->msgtmaddr) {
   Alarm("EXT001A ERROR: ADDRESS LIST OVERWRITTEN");
   return(-1);
   }

return(0);
}

/**********************************************************************
*			RWWriteString
* Store the string of addresses (str) in the Msgdir (dir).
* 1. Extract the addresses from the string
* 2. Copy the addresses into the Msgdir
*
* NOTE: This function assumes that the MSGDIR field "msgaddrs_type"
* is set correctly on entry. This determines the type of addresses
* - AIFS, CMSS, AFTN.
**********************************************************************/
void RWWriteString(MSGDIR *dir, char *str)
{
char	*p, temp[(MAX_AIFS_ADDR*(MAX_ADDR_LEN))+1];
char	addrs[MAX_AIFS_ADDR][AIFS_ADDR_LEN+1];
char    *saveptr;
int	len, num_addr = 0;

/* EXTRACT THE STRING ADDRESSES */
strcpy(temp,str);
p = strtok_r(temp," \0",&saveptr);
while (p && num_addr < MAX_AIFS_ADDR) {
      len = strlen(p);
      if (len <= AIFS_ADDR_LEN) {
	 strcpy(addrs[num_addr],p);
	 num_addr++;
	 /* CMSS/AFTN/EXTENDED ADDRESSES CAN BE USED TOGETHER */
	 if (len > 4 && dir->msgaddrs_type == AD_CMSS) 
	    dir->msgaddrs_type = AD_AFTN;
	 }
      else 
	 Alarmv("EXT002A ERROR: ADDRESS TOO LONG (%s)",p);
      p = strtok_r(NULL," \0",&saveptr);
      }

/* STORE THE ADDRESSES IN THE MSGDIR */
RWwriteAddr(dir,addrs,num_addr);

}

/**********************************************************************
*		RWcmssWrite
* Copy the list of CMSS addresses into the Msgdir.
**********************************************************************/
static void RWcmssWrite(MSGDIR *dir, char addrs[][AIFS_ADDR_LEN+1], int num_addr)
{
int	i;

for (i=0; i<num_addr && i<MAXNOADS; i++) 
    strncpy(dir->msgaddrs.cmss[i],addrs[i],4);

dir->msgnoads = i;
dir->msgaddrs_type = AD_CMSS;
}

/**********************************************************************
*			RWaftnWrite
* Copy the list of AFTN addresses into the Msgdir.
**********************************************************************/
static void RWaftnWrite(MSGDIR *dir, char addrs[][AIFS_ADDR_LEN+1], int num_addr)
{
int	i;
int	max = MAXNOADS/2;

for (i=0; i<num_addr && i<max; i++) 
    strncpy(dir->msgaddrs.aftn[i],addrs[i],8);

dir->msgnoads = i;
dir->msgaddrs_type = AD_AFTN;
dir->msgaftn = TRUE;
}

/**********************************************************************
*			RWcopyaddr
* Rewrite the AIFS "overflow" addresses. This function is called
* during "resends" and is supposed to ensure that the "overflow"
* record isn't overwritten before the Msgdir that points to it.
**********************************************************************/
void RWcopyaddr(MSGDIR *dir)
{
char	addr[MAX_AIFS_ADDR][AIFS_ADDR_LEN+1];
int	numaddr;

if (dir->msgaddrs_type == AD_EXT_AIFS || dir->msgaddrs_type == AD_EXT_CMSS || dir->msgaddrs_type == AD_EXT_AFTN) {
   RWReadList(dir,addr,MAX_AIFS_ADDR,&numaddr);
   store_on_disc(dir,addr,numaddr);
   }

}

/**********************************************************************
*			MAINLINE
**********************************************************************/
//#define TEST_MAINLINE 
#ifdef TEST_MAINLINE

main()
{
MSGDIR	dir;
char	addrs[10][10], addr2[10][10];
char	str[40];
int	num_addr, i;

dir.msgaddrs_type = AD_CMSS;

RWWriteString(&dir, "AAAA BBBB CCCC DDDD EEEE");
printf("IES: num addrs = %d\n",dir.msgnoads);
printf("reading list\n");
RWReadList(&dir,addr2,10, &num_addr);
for (i=0;i<num_addr;i++)
    printf("%s\n",addr2[i]);

RWWriteString(&dir, "AAAA BBBB CCCC DDDD EEEE FFFF GGGG HHHH IIII JJJJ KKKKK LLLL");
printf("IES: num addrs = %d\n",dir.msgnoads);

exit(0);

strcpy(addrs[0],"AAAAAA");
strcpy(addrs[1],"BBBBBB");
strcpy(addrs[2],"CCCCCC");
strcpy(addrs[3],"DDDDDD");
strcpy(addrs[4],"EEEEEE");
strcpy(addrs[5],"FFFFFF");
strcpy(addrs[6],"GGGGGG");
strcpy(addrs[7],"HHHHHH");
strcpy(addrs[8],"IIIIII");
strcpy(addrs[9],"JJJJJJ");

chdir ("/users/ies/cmss");
RWaifsWrite(&dir,addrs,10);

printf("reading\n");
RWReadString(&dir,str,40);
printf("str = %s\n",str);

printf("reading list\n");
RWReadList(&dir,addr2,10, &num_addr);
for (i=0;i<num_addr;i++)
    printf("%s\n",addr2[i]);

}

#endif
