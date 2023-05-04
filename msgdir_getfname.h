#ifndef _MSGDIR_FNAME_H_
#define _MSGDIR_FNAME_H_

#include "msgdir.h"

    char *msgdir_getfname(MSGDIR *);
    char *msgdir_getfname_r(MSGDIR *,char *filename);
    char *msgdir_getfext(MSGDIR *);
    char *msgdir_getfext_r(MSGDIR *,char *fext);
    char *msgdir_fullfilename(MSGDIR *dir);
    char *msgdir_fullfilename_r(MSGDIR *dir,char *full_filename);

#endif
