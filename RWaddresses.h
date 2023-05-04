/*********************************************************************
*                       AifsExtAddr.H
* Defines the functions used to read/write "addresses to Msgdirs.
*
* v00  16/02/95 Ian Senior      Original
* v01  21/02/95 Ian Senior      Add RWReadList
* v02  16/03/95 Ian Senior      Add RWWriteString
* v03  22/03/95 Ian Senior      Add RWcopyaddr
* v04  13/07/98 Ian Senior      Add RWwriteAddr
* v05  09/03/01 Ian Senior      Change to AIFS_ADDR_LEN+1
* v06  07/12/01 Ian Senior      Add RWReadList2
* v07  23/02/05 Ian Senior      Remove RWaifsWrite 
* v08  24/02/05 Ian Senior      Add RWappendAddr.
*********************************************************************/

#ifndef _RWaddresses_H_
#define _RWaddresses_H_

#include "msgdir.h"

int RWwriteAddr(MSGDIR *dir,char addrs[][AIFS_ADDR_LEN+1], int num_addr);
int RWwriteOneAddr(MSGDIR *dir,char *addr);
int RWappendAddr(MSGDIR *dir,char *addr);
void RWWriteString(MSGDIR *dir, char *str);
void RWReadString(MSGDIR *dir, char *str, int len);
void RWReadList(MSGDIR *dir, char addr[][AIFS_ADDR_LEN+1], int maxlist, int *num_addr);
void RWReadList2(MSGDIR *dir, char addr[][AIFS_ADDR_LEN+1], int maxlist, int *num_addr);
void RWcopyaddr(MSGDIR *dir);

#endif
