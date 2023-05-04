#include "logc.h"

int logc      (char *text) { printf("%s\n",text); return 0;}
int logc_r    (char *label,char *text) { printf("%s\n",label); printf("%s\n",text); return 0;}
int logcv     (const char *text, ...) { printf("%s\n",text); return 0;}
int logcv_r   (char *label,const char *text, ...) { printf("%s\n",label); printf("%s\n",text); return 0;}
