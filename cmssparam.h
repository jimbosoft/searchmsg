/****************************************************************
* v1.4  12/01/96 Ian Senior     Allow for non-Ansi calling function.
* v1.5  02/02/05 Ian Senior     Add cmssparamn
* v1.6  25/11/05 Ian Senior     Add cmssparamgCache
* v1.7  08/05/08 Ian Senior     Add cmssparamString, cmssparamgString
* v1.8  11/08/08 Ian Senior     Add <time.h> for linux
* v1.9  30/07/20 Ian Senior     Add cmssparam_ReadTokens/GetParm
****************************************************************/

#ifndef _CMSSPARAM_H_
#define _CMSSPARAM_H_

#include <time.h>
#include "tokens.h"

#ifdef __cplusplus
    extern "C" { int	cmssparam (char *param, char *result);};
    extern "C" { int  cmssparamn (const char *param, char values[][80], int max);};
    extern "C" { int	cmssparam2 (char *param, char *result);};
    extern "C" { int cmssparamString(char *param, char *str);};
    extern "C" { int cmssparamg (char *fname, char *param, char *result);};
    extern "C" { int cmssparamgn (char *fname, char *param, char values[][80], int max);};
    extern "C" { int cmssparamgString(char *fname, char *param, char *str);};
    extern "C" { int cmssparamgt (TOKENS *t, char *fname, time_t *read_time, char *param, char *result);};
    extern "C" { int cmssparam_ReadTokens(char *fname, TOKENS *t);};
    extern "C" { int cmssparam_GetParm(TOKENS *t, char *parm, char *value);};
#else
    int cmssparam();
    int cmssparam2();
    int cmssparamString();
    int cmssparamg();
    int cmssparamgn();
    int cmssparamgString();
    int cmssparam_ReadTokens();
    int cmssparam_GetParm();
#endif

#endif
