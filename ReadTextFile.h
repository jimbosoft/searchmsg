#ifndef _READTEXTFILE_H
#define _READTEXTFILE_H

    int	ReadTextFile (int, int, int *, short *, char *, char *, char *, int flag);
    int ReadTextFile2(int index_text, int off, int msglen, int *len, char *msg, int flag);
    int ReadTextFile_r(int index_text, int off, int msglen, int *len, char *msg, int flag);

#endif
