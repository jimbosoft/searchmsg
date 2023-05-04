static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/makestring.cc,v 1.5 2001/03/05 03:01:58 ace Exp $" ;
//
//  Program: makestring.cc
//
//  Prototype in: makestring.h
//
//  Usage:		makestring (text_len, text);
//
//  Return code:	none
//
//  Description:	This mammoth program was developed over time,
//			and is the result of careful planning.  The idea
//			for this useful tool first germinated in the
//			of I.S..  This sprouted, grew and bore wonderful
//			fruits that tumbled from his mouth. And I.S.
//			spake to his humble doer of all things nasty 
//			"D.A. - take this fruit and turneth it into a
//			wonderous thing that all can use and enjoy". 
//			And D.A. did this thing, and I.S. saw that it 
//			was good.  (Remove null characters from a 
//			passed string out to the passed length).
//
//  History
//  v1.1  17/09/92  Doug Adams  Original Code.
//  v1.3  08/07/96  Ian Senior	Change way scan string for nulls.
//  v1.5  05/03/01  C.Edington	Fixed '// *' to not look like comments.
//
#include <stdio.h>
#include <string.h>

void makestring (int num_chars, char *text) 
{
   int	i;

   text[num_chars] = '\0';

   // REMOVE INTERNAL NULLS FROM STRING
   for (i=0; i<num_chars; i++) {
       if (text[i] == '\0') text[i] = ' ';
       }
}
