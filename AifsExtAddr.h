/*********************************************************************
*                       AifsExtAddr.H
* Defines the "Directory" structure used for overflow addresses
* (primarily for AIFS).
*
* v00  16/02/95	Ian Senior      Original
* v1.1 23/03/95	Ian Senior      Replace msgtmrcv with msgtmaddr.
* v1.2 07/02/96	Ian Senior      Change typedef ADDR_DIR
* v1.3 12/02/96	Ian Senior      Increase size of structure/file.
*********************************************************************/

#ifndef _AIFSEXTADDR_
#define _AIFSEXTADDR_

#include <time.h>
#include "msgdir.h"

#define MAX_AIFS_EXTADDR	20

typedef struct ADDR_DIR_S {
     time_t	msgtmaddr;
     short	msgnoads;
     char	spare[6];
     char	aifs[MAX_AIFS_EXTADDR][AIFS_ADDR_LEN];
     char	spare2[28];
     } ADDR_DIR;

#endif
