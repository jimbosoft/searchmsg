/*********************************************************************
*                       MSGINTXT.H
* v1.1 22/04/02 Ian Senior      Create from cmss.h
*********************************************************************/
#ifndef _MSGINTXT_H
#define _MSGINTXT_H

#define NTBR_OFFSET     0
#define NTBW_OFFSET     8
#define NTBW_OFFSET_OLD 4

/*  MSGINTXT.DAT declarations */
#define MSGINTXT_INFO_LEN       27
typedef struct MSGIN_TXT {        /* MSGINTXT.DAT "record" overlay */
        unsigned int msglng;
        short        format;
        char         rrrcc [5];
        char         dtead [16];
        char         msg [MAX_MSGSIZE];
        } MSGIN_TXT;

typedef struct MSGIN_PTR {        /* MSGINTXT.PTR file overlay */
        unsigned int index_ntbr;
        unsigned int ntbr;
        unsigned int index_ntbw;
        unsigned int ntbw;
        unsigned int index_max;
        unsigned int max_msgintxt;
        unsigned int max_text_last;
        } MSGIN_PTR;

typedef struct MSGIN_PTR_OLD {      
        unsigned int ntbr;
        unsigned int ntbw;
        unsigned int max_msgintxt;
        } MSGIN_PTR_OLD;


#endif
