#ifndef _GETGROUP_H
#define _GETGROUP_H

/* v1.1 16/03/94 T Crawford  AMDAR - increase MAX_GROUP_LENGTH  */
/* v1.2 01/02/96 T Crawford  change MAX to MAXCH                */
/* v1.3 04/06/96 T Crawford  add PLUS                           */
/* v1.4 06/06/96 T Crawford  add DOT                            */
/* v1.5 15/01/98 T Crawford  add SOLIDI                         */
/* v1.6 08/02/01 T Crawford  increase MAX_GROUP_LENGTH          */
/* v1.7 05/12/14 Ian Senior  Add getngrp_r()                    */


#define	ALPHA	1
#define	NUMERIC	2
#define	NEWLINE	3
#define	MIXED	4
#define	EOM	5 
#define	EQUALS	6
#define	MAXCH	7
#define	PLUS	8
#define	DOT	9
#define	SOLIDI	10

#define	MAX_GROUP_LENGTH	69

char * getgrp (char *text,int textlen,int *start,int *len,int *type);
char * getngrp (char *text,int textlen,int *start,int *len,int *type);
void getngrp_r(char *txt,int len,int *start,int *num,int *type, 
				char *group, int max_group_length);

void get_next_group (char *group, char **text, int *len, int *type);
#endif
