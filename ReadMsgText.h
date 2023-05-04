#ifndef _READMSGTEXT_H2
#define _READMSGTEXT_H2

#ifdef __cplusplus
    extern "C" {int ReadMsgText (struct MSGDIR *, int *, char *);};
    extern "C" {int ReadMsgText2 (struct MSGDIR *, int *, char *, int alarmit);};
    int ReadMsgText_r(struct MSGDIR *, int *, char *);
#else
    int ReadMsgText();
#endif

#endif
