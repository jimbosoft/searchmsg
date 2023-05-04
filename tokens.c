static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/tokens.c,v 1.16 2006/01/11 02:41:21 ies Exp $" ;
/**
*	tokens.c -- subroutine-toolkit to handle "Token-Structures".
*
* Definitions within these subroutines:
*
*  word:	1 word of characters trimmed of leading and trailing whitespace
*  keyword:	a word with a trailing "=", e.g. "keyword="
*  string:	a zero-delimited sentence of words, with arbitrary whitespace,
*		and may contain a (trailing) comment starting with a "#".
*  token-structure:	a structure that stores many words/keywords,
*			and is operated on by these subroutines.
* Status: 940729 Clive. This code is heavily used in production by Mcidas.
* Future: It may be used in Difacs too.
*	  It would benefit from a rationalisation of error codes,
*	  and from a "man" page to document it,
*	  and from extra macros to allow "CopyString" vs "AddString" etc.,
*		(i.e. to zero the destination before adding to it)
* $Header: /home/commsa/cmdv/cmss/src/RCS/tokens.c,v 1.16 2006/01/11 02:41:21 ies Exp $
*
* v1.16 2006-01-16 C.Edington, linux type change
* v1.15 2003-10-02 C.Edington, fix delete-keyword bug.
* v1.14 2002-07-01 C.Edington, added DeleteThisKeyword.
* v1.13 010209 C.Edington, update multiple kw=value in AllIn1Rewrite.
* v1.12 000808 C.Edington, fix bug in GetNextKeywordsWords.
* v1.11 000714 C.Edington, preserve comments in AllIn1Rewrite,
*				put spaces around '=' in getstrings and writes.
* v1.10 000626 C.Edington, added Read/Write/RewriteAllIn1, GetNextKeywordsWords.
* v1.9  980602 cmdv,       added rcs-id string.
* v1.8  960223 C.Edington, fix an unterminated comment in tokens filter.
* v1.5  960117 I.Senior	 Fix minor problem in tokens_FileRead1Token
*			 (uninitialized memory read - found by purify)
* v1.4  950812 C.Edington, use our own tokeniser, hence allow:
*			. dont need spaces around "=" signs,
*			. does not clobber input string,
*			. uses "quoted" strings to hide characters,
*			. can use single (') or double (") quotes around strings.
* v1.3  950406 ACE, fixed PrintfTokens, added ToValueString.
* v1.2  950404 ACE, added tokens_Filter.
*			Removed unused WildcardMatch.
* v0.6  940715 ACE, fixed detection of I/O errors.
* v0.5  940130 ACE, added GetThisKeywordsWords, RestartNextKeywords,
*			SearchForKeyword.
* v0.4  931221 ACE, renamed qNtokens to qNwords, added GetWords,
*			Write1token, WriteAllTokens.
* v0.3  931115 ACE, added AddTokens, GetNextKeyword..,
*			allow blanks around "=".
* v0.2  931112 ACE, improved error test, bigger array sizes.
* v0.1  931111 ACE, initial code.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "tokens.h"
#ifdef MAIN_tokens
int logc (char *msg);
#else
#include "logc.h"
#endif

#define  OK  0
#define  TRUE  1
#define  FALSE 0

typedef struct TOKENS_STRUCT *T;

/* FileRead1Token saves the string it has just read in this global variable.*/
/* It is used by AllIn1Rewrite to retrieve lines that are comments.*/
/* Note: continuation lines are joined into a single line.*/
static char g_read_string [TOKENS_MAX_TMP_STRING_LENGTH];

static int NotInitYet (char *routine_name);
static int SearchForKeyword (TOKENS *t, char *keyword);

/**
*--------------------------------------------------------------------------
*						(internal)	NotInitYet
*--------------------------------------------------------------------------
* Called internally (when Tokens-structure is not initialised)
* to tell the user that an input Tokens-structure is
* not initialised yet.
* Returns:  -9   Tokens-structure not initialised.
*/
static int NotInitYet (char *routine_name) {
     char msg[80];
     strcpy (msg, "TOK001A UNINITIALISED TOKENS IN tokens_");
     strcat (msg, routine_name);
     logc (msg);
     return -9;
}

/**
*--------------------------------------------------------------------------
*								Init
*--------------------------------------------------------------------------
* Every TOKENS structure must be initialised before use.
*/
int tokens_Init       (TOKENS *t) {
    int i;

    t->Totwords = 0;
    t->Nextword = 0;
    t->Totchars = 0;
    t->String[0] = '\0';
    for (i=0; i<TOKENS_MAX_WORDS; i++ ) {
	t->Keyword [i] = 0;
	t->Wordlength [i] = 0;
	t->Word [i] = NULL;
    }
    t->Magic = TOKENS_MAGIC;  /* This is now a correct structure.*/
    return OK;
}

/**
*--------------------------------------------------------------------------
*								AddWord
*--------------------------------------------------------------------------
* Add (append) a word-token to this existing tokens-structure.
* Note: This routine is usually only used by internal routines to transfer
*	words into a token-structure.
* Warning: the "word" must already be a correct token-word,
*	free of leading and trailing blanks, and non-null.
* Returns:  0 = OK,
*	   -1 = error, full, cannot add word.
*	   -2 = error, illegal word.
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_AddWord (TOKENS *t, char *word, int wordlen) {

    int wordno;

    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("AddWord");

    wordno=t->Totwords;
    if (wordno >= TOKENS_MAX_WORDS)  return (-1);  /* Full */
    if (wordlen < 1)  return (-2); /* illegal word */
    if ( (t->Totchars+wordlen+1) > TOKENS_MAX_STRING_LENGTH) {
	return (-1); /* no room */
    }

    /* If this is an "=" sign by itself, */
    /* and the last word was not a keyword, */
    /* then make the last word a keyword now by appending the "=" to it.*/
    if ((wordlen==1) && (word[0]=='=')
    && (wordno>0) && (t->Keyword[wordno-1]==0)) {
	/* Add "=" to the end of the last word.*/
	t->Wordlength [wordno-1] += 1;
	t->String [t->Totchars-1] = '=';
	t->Totchars++;
	t->String [t->Totchars-1] = '\x0';
	t->Keyword [wordno-1] = 1;
	return (OK);
    }

    /* Copy the normal word/keyword to safe storage.*/
    /* (Each word is stored with its own trailing-zero-delimiter.) */
    strncpy (&t->String [t->Totchars], word, wordlen);
    t->Word [wordno] = &t->String [t->Totchars];
    t->Wordlength [wordno] = wordlen;
    if (word[wordlen-1] == '=')	t->Keyword [wordno] = 1;
    else			t->Keyword [wordno] = 0;
    t->Totchars += wordlen+1;
    t->String [t->Totchars-1] = '\x0';
    t->Totwords++;
    return (OK);
}

/**
*--------------------------------------------------------------------------
*								AddString
*--------------------------------------------------------------------------
* Add a character string of token(s) to this existing tokens-structure.
* (The input string is left unchanged).
* Returns:  0 = OK,
*	   -1 = error, full, cannot add all words.
*	   -2 = error, illegal word.  (-ve length word. Program error)
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_AddString (TOKENS *t, char *instring) {

    char *instringptr, *outword;
    int wordno, len, reply;

    if (t->Magic != TOKENS_MAGIC)
	return NotInitYet("AddString");  /* Not initialised yet.*/
    instringptr = instring;
    for (wordno=t->Totwords; ; wordno++) {
	reply = tokens_TokeniserNext (instringptr, &outword, &len);
	if (reply < 0)  return (-2);  /* bug ? */

	if (reply == 0)  return (OK);  /* End of input string */

	instringptr = NULL;  /* (for subsequent calls to strtok).*/

	/* Now just have a normal word.*/
	/* Copy it to safe storage.*/
	reply = tokens_AddWord (t, outword, len);
	if (reply)  return (reply);
    }
    return (-1);  /* Should not get this far.*/
}

/* future tokeniser:  start (set policy), then next-token.*/

/**
*--------------------------------------------------------------------------
*							TokeniserNext
*--------------------------------------------------------------------------
* Scan the input text string and return a pointer+length of the next "word".
* . Leading and trailing blanks are stripped..
* . Imbedded "=" are recognised.
* . trailing comments are discarded.
* . "quoted" strings can be used to enclose sensitive characters such as
*	blanks and "=".
* . use either single-quotes or double-quotes to enclose quoted strings.
* . Quoting characters are preserved around the string. Strings with no
*	matching trailing quote are ... (? maybe we add a trailing quote)
* (This behaves like the system's "strtok", but does not clobber the string)
* The first call to this routine should provide the address of the string
* to be tokenised. Subsequent calls with address NULL will retrieve the next
* tokens from the original string.
* Returns:  1 = success, next token-word returned. Save it as is.
*	    0 = end-of-string reached. (or no initial string was set up)
*	   -1 = error
*/
int tokens_TokeniserNext  (char *instring,
				char **tokenpointer, int *tokenlength) {

    static char *partialstring=NULL;	/* points to original string.*/
    static int  startcounter=0;		/* number of next char to process.*/
    int		endcounter;		/* number of char after word.*/
    int		wordlength;		/* length of word just found.*/
    int		noclosingquote;
    int		reply;

    /* Set outputs to null */
    *tokenpointer = NULL;
    *tokenlength = 0;
    noclosingquote = FALSE;

    if (instring != NULL) {
	/* First call with a new string.*/
	/* Setup our pointers. */
	partialstring = instring;
	startcounter = 0;
    }
    else {
	/* i.e. instring=NULL, follow on call.*/
	/* Check that we have already started a string. */
	if (partialstring==NULL)  return (-1);  /* bug. bad call. */
	/* Check that the startcounter is not corrupted.*/
	if (startcounter < 1) return (-1); /* my bug ? */
    }

    /* Start processing a partial string.*/

    /* 1. String leading whitespace, leave the startcounter on a non-space.*/
    reply = tokens_StripLeadingWhiteSpace (partialstring, &startcounter);
    if (reply<0) return (-1);  /* my bug ? */

    /* 2. Process characters until separator or comment or end-of-string.*/
    endcounter = startcounter;
    for (;;) {  /* dummy loop so that logic can exit whenever it needs to.*/

	/* exit on end-of-string */
	if (partialstring [endcounter]=='\x0')  break;

	/* exit on start of comment string */
	/*  For the moment comments are "#" (unix). */
	/*  Later our policy could include ";" (windows). */
	if (partialstring [endcounter]=='#')  break;

	/* exit on "=" sign (the end of a keyword) */
	if (partialstring [endcounter]=='=')  {
	    /* include the "=" within the word.*/
	    endcounter++;
	    break;
	}

	/* exit on a separator (end-of-a-word).*/
	/* currently only use whitespace separators, "space" or "tab" */
	/*  later allow comma,etc.*/
	if (partialstring [endcounter]==' ')  break;
	if (partialstring [endcounter]=='\t')  break;

	/* perhaps swallow a whole quoted string. (leave quotes in).*/
	/* currently use double quotes. later can use other chars too.*/
	if ( (partialstring [endcounter]=='"')
	      || (partialstring [endcounter]=='\'') ) {
	    /* Start of a quoted string. */
	    /* Look for the closing quote, and update the endpointer to */
	    /* be beyond it.*/
	    reply = tokens_SwallowQuotedString (partialstring, &endcounter);
	    if (reply<0) return (-1);  /* my bug ? */

	    /* if there was a closing quote, then it is in the string and */
	    /* we can continue scanning characters beyond it.*/
	    if (reply==0) continue;

	    /* Woops, got to end of string and no closing quote.*/
	    noclosingquote = TRUE;
	    break;	/* end of string */
	}

	/* We get here when we have a non-sensitive character */
	/* halfway through a word. */
	/* Continue looking at up-coming characters.*/
	endcounter++;
	continue;
    }

    /* 3. Got a whole word. Return it to the caller. */

    /* This word is up to, but not including, the last character scanned.*/
    wordlength = endcounter - startcounter;

    /* end-of-string, but no word ? */
    /* if so, closedown this string. */
    if (wordlength<=0) {
	/* No more words in string. Reset ourselves to null.*/
	partialstring = NULL;
	startcounter = 0;
	return (0);   /* no more words.*/
    }

    /* Record this word. */
    *tokenpointer = &partialstring [startcounter];
    *tokenlength = wordlength;

    /* then update our pointers.*/
    startcounter = endcounter;

    /* tell caller no closing quote.*/
    if (noclosingquote) return (2); /* Success, 1 word, no closing quote.*/

    return (1);  /* Success, 1 word returned.*/
}

/**
*--------------------------------------------------------------------------
*						StripLeadingWhiteSpace
*--------------------------------------------------------------------------
* Given a character string and a position within that string,
* skip over leading whitespace (spaces or tabs),
* and update the string-position to point to the first non-white character.
* Returns:  0 = OK, "position" has been updated.
*	   -1 = failed. bad inputs.
*/
int tokens_StripLeadingWhiteSpace (char *partialstring, int *position) {

    if (*position < 0) return (-1);  /* bug ? */

    for (;;) {
	/* currently only use whitespace separators, "space" or "tab" */
	/*  later allow comma,etc.*/
	if (partialstring [*position]==' ')  {(*position)++; continue;}
	if (partialstring [*position]=='\t') {(*position)++; continue;}
	/* Got a non-space or non-tab. so exit.*/
	break;
    }
    return (0);
}

/**
*--------------------------------------------------------------------------
*							SwallowQuotedString
*--------------------------------------------------------------------------
* Given a character string and the position within that string where a
* quoted string starts, search for the matching end-of-quote,
* and update the string-position to point to the first character after the
* closing quotes.
* If the end of string is reached before a closing quote, then tell the
* caller (who may add a closing quote for us).
*
* Currently only uses a dumb algorithm of allowing either single- or
* double-quotes around a string.
* (later we could add backslash-processing, or other logic,
* e.g. curly or square brackets.)
*
* Returns:  1 = OK, "position" has been updated, but no closing quote.
*	    0 = OK, "position" has been updated.
*	   -1 = failed. bad inputs.
*/
int tokens_SwallowQuotedString (char *partialstring, int *position) {

    char quotechar;

    if (*position < 0) return (-1);  /* bug ? */

    /* Grab the first quote char, and use it to find a closing quote.*/
    quotechar = partialstring [*position];
    (*position)++;

    for (;;) {
	/* If end-of-string, tell caller "no closing quote".*/
	if (partialstring [*position]=='\x0')  return (1);

	/* Closing quote, jump over it, then return OK.*/
	if (partialstring [*position]==quotechar) {
	    (*position)++;
	    return (0);
	}

	/* still going. Try next character. */
	(*position)++;
    }
    return (-1);  /* Should not get this far.*/
}

/**
*--------------------------------------------------------------------------
*								AddTokens
*--------------------------------------------------------------------------
* Add one tokens-structure (intokens) into another tokens-structure (t).
* Returns:  0 = OK,
*	   -1 = error, full, cannot add all words.
*	   -2 = error, illegal word.
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_AddTokens  (TOKENS *t, TOKENS *intokens) {

    int i, reply;

    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("AddTokens");
    if (intokens->Magic != TOKENS_MAGIC)  return NotInitYet("AddTokens");

    /* Copy the words until end-of-list or error.*/
    for (i=0; i < intokens->Totwords; i++) {
	reply = tokens_AddWord (t, intokens->Word[i], intokens->Wordlength[i]);
	if (reply) return (reply);  /* Error while copying. */
    }
    return (OK);
}

/**
*--------------------------------------------------------------------------
*							FileRead1Token
*--------------------------------------------------------------------------
* Read the given stream-file, picking up one string of tokens.
* (which may be spread over several text lines).
* The string (1) is saved in g_read_string (for AllIn1Rewrite to use),
* and (2) is converted into a token structure.
* Returns:  0 = success (tokens-structure returned, g_read_string is set.),
*	   -1 = end-of-file seen, no new data read in.
*	   -2 = error, could not store all tokens. (Structure filled,
*			or other error).
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_FileRead1Token    (TOKENS *t, FILE *stream) {

    char *next_start, *next_string;
    int length, room_left, next, reply;

    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("FileRead1Token");

    /* Read one line at a time, */
    /* checking to see if there is a backslash-continuation-character.*/
    room_left = TOKENS_MAX_TMP_STRING_LENGTH;
    next = 0;
    g_read_string [0] = '\x0';
    reply = 0;
    for (;;) {
	/* Read the next string, and trim off the trailing cr,lf */
	next_start = &g_read_string[next];

	next_string = fgets (next_start, room_left, stream);

	if (next_string == NULL) {
	    reply = -1;   /* some sort of read error */
	    break;
	}
	length = strlen (next_start);
	if (length>0 && next_string[length-1]=='\n') length--;  /* skip over NL */
	if (length>0 && next_string[length-1]=='\r') length--;  /* skip over DOS CR */

	/* Exit if there are no more backslash-continuation lines.*/
	if (length == 0 || next_string [length-1] != '\\') {
	    next_string [length] = '\x0';
	    break;
	}

	/* Next continuation line...*/
	length--;        /* remove backslash */
	next += length;
	g_read_string [next] = '\x0';
	room_left -= length;
	if (room_left <= 0)  break;
    }

    if (reply) return (-1);   /* EOF, or Error during read.*/

    /* Got a long string. */
    /* Now make it into a token. */
    reply = tokens_AddString (t, g_read_string);
    if (reply) return (-2);  /* Cannot add tokens. */
    return (OK);
}

/**
*--------------------------------------------------------------------------
*							FileWrite1Token
*--------------------------------------------------------------------------
* Write 1 token-structure as a string on the given stream-file
* Returns:  0 = success
*	   -1 = I/O error.
*	   -2 = internal string error.
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_FileWrite1Token    (TOKENS *t, FILE *stream) {

    int reply;
    char outstring [TOKENS_MAX_STRING_LENGTH];

    if (t->Magic != TOKENS_MAGIC)  return (NotInitYet("FileWrite1Token"));
    strcpy (outstring,"");
    reply = tokens_ToString (t, outstring, TOKENS_MAX_STRING_LENGTH);
    if (reply) return (-2);
    reply = fprintf (stream, "%s\n", outstring);
    if (reply < 0)  return (-1);  /* I/O error */
    return (OK);
}

/**
*--------------------------------------------------------------------------
*							FileReadAllTokens
*--------------------------------------------------------------------------
* Read the given file, and pick up an array of Token strings (t[]).
* (Each token-string may be spread over several text lines).
* Ignores empty token strings (e.g blank lines).
* Returns:  n = success, n tokens read. (tokens-structure-array returned),
*	   -1 = cannot open file.
*	   -2 = error, could not store all tokens. (Array or structure
*		filled, or other error).
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_FileReadAllTokens (TOKENS *t, int maxtokens, char *filename) {

    FILE *stream;
    int reply, ntokens;

    stream = fopen (filename, "r");
    if (stream == NULL)   return (-1);  /* Cannot open file */

    /* Read the token-string one at a time until error or end-of-file.*/
    /* Skip over comment strings (that begin with "#").*/
    ntokens = 0;
    for (;;) {
	if (ntokens >= maxtokens) {
	    fclose (stream);
	    return (-2);    /* Array overflow. maxtokens returned.*/
	}
	if (t[ntokens].Magic != TOKENS_MAGIC)  {
	    fclose (stream);
	    return NotInitYet("FileReadAllTokens");
	}
	reply = tokens_FileRead1Token (&t [ntokens], stream);
	if (reply)  break;

	/* Only keep non-empty tokens */
	if (tokens_qNwords(&t [ntokens]) != 0)  ntokens++;
    }
    fclose (stream);
    if (reply <= -2)  return (-2);  /* some internal error */
    /* reply=-1 means normal eof */
    return (ntokens);
}

/**
*--------------------------------------------------------------------------
*							FileWriteAllTokens
*--------------------------------------------------------------------------
* Write an array of tokens-structures as a series of text lines in this file.
* Returns:  n = success, n tokens written, file updated.
*	   -1 = cannot open file.
*	   -2 = error, could not store all tokens. (I/O error or
*		other internal error).
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_FileWriteAllTokens (TOKENS *t, int ntokens, char *filename) {

    FILE *stream;
    int i, reply, reply2;

    stream = fopen (filename, "w");
    if (stream == NULL)   return (-1);  /* Cannot open file */
    
    reply = 0;
    for (i=0; i<ntokens; i++) {
	if (t[i].Magic != TOKENS_MAGIC)  {
	    reply = NotInitYet("FileWriteAllTokens");
	    break;
	}
	reply = tokens_FileWrite1Token (&t [i], stream);
	if (reply)  break;
    }
    reply2 = fclose (stream);

    if (reply || reply2) return (-2);  /* Some error.*/
    return (ntokens);   /* OK, file updated. */
}

/**
*--------------------------------------------------------------------------
*							FileAllIn1ReadToken
*--------------------------------------------------------------------------
* Read the given file, and put all of its tokens into one Token structure.
* (Each token-string may be spread over several text lines).
* Ignores empty token strings (e.g blank lines).
* Returns:  n = success, n words read. (tokens-structure returned),
*	   -1 = cannot open file.
*	   -2 = error, could not store all tokens. (Structure
*		filled, or other error).
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_FileAllIn1ReadToken (TOKENS *t, char *filename) {

    FILE *stream;
    int reply, ntokens;

    if (t->Magic != TOKENS_MAGIC)  {
	return NotInitYet("FileAllIn1ReadToken");
    }

    stream = fopen (filename, "r");
    if (stream == NULL)   return (-1);  /* Cannot open file */

    /* Read the token-strings one at a time until error or end-of-file.*/
    /* Skip over comment strings (that begin with "#").*/
    for (;;) {
	reply = tokens_FileRead1Token (t, stream);
	if (reply)  break;
    }
    fclose (stream);
    /* reply=-1 means normal eof */
    if (reply <= -2)  return (-2);  /* some internal error */

    /* total words = Number of key-words plus number of value-words.*/
    ntokens = t->Totwords;
    return (ntokens);
}

/**
*--------------------------------------------------------------------------
*                                                      FileAllIn1WriteToken
*--------------------------------------------------------------------------
* Write one tokens-structure as a series of text lines in this file,
* one "keyword= value" per line.
* Returns:  n = success, n keywords written.,
*          -1 = cannot open file.
*          -2 = error, could not store all tokens. (I/O error or
*               other internal error).
*          -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_FileAllIn1WriteToken (TOKENS *t, char *filename) {

    FILE *stream;
    int nkeywords, save_nextword, reply, reply2;
    TOKENS t2;

    if (t->Magic != TOKENS_MAGIC)  {
        return NotInitYet("FileAllIn1WriteToken");
    }

    stream = fopen (filename, "w");
    if (stream == NULL)   return (-1);  /* Cannot open file */

    /* Save NEXT pointer. Restore it later.*/
    save_nextword = t->Nextword;
    reply = tokens_RestartNextKeywords (t);
 
    /* Get each keyword, and write it on its own line.*/
    nkeywords = 0;
    for (;;) {
	tokens_Init (&t2);
	reply = tokens_GetNextKeywordsTokens (t, &t2);
        if (reply)  break;

	/* Any more left ? */
	if (t2.Totwords == 0) break;
        
	reply = tokens_FileWrite1Token (&t2, stream);
        if (reply)  break;
	nkeywords++;
    }

    /* Restore.*/
    t->Nextword = save_nextword;
    
    reply2 = fclose (stream);
    
    if (reply || reply2) return (-2);  /* Some error.*/

    /* OK. */
    return (nkeywords);
}

/**
*--------------------------------------------------------------------------
*						    FileAllIn1RewriteString
*--------------------------------------------------------------------------
* Given a filename, and a string with new multiple "keyword = value"
* settings (separated by blanks),
* read the given file, and rewrite its kewords and values, updating the 
* the new "keyword = value"s, 
* and preserving comment lines.
* If new keywords do not exist on the old file, add them to the new file now.
* If the old file did not exist, then write a new file now.
* If any errors occur while the file is being rewitten, then a partial file
* may result. (Hopefully this fault should never occur).
* Side-effects:
* (1) trailing comments are lost (e.g. keyword = value # trailing comment)
* (2) tabs between keywords & value-words are converted to spaces.
* Returns:  0 = success (file updated and rewritten.),
*	   -n = any error.  file not updated fully.
*/
int tokens_FileAllIn1RewriteString (char *newstring, char *filename) {

    TOKENS oldtoken, newtoken, giventokens, alloldtokens;
    int reply, reply2, replaced;
    FILE *instream, *outstream;
    char *thiskeyword;
    char tmpfilename[1025], cmd[2049];

    /* Turn the given string into a tokens structure (multiple kw=value).*/
    tokens_Init (&giventokens);
    reply = tokens_AddString (&giventokens, newstring);
    if (reply) {
	return (-1);
    }

    /* Read the whole input file into one tokens array,*/
    /* so later we can check for new tokens not in this old file.*/
    tokens_Init (&alloldtokens);
    reply = tokens_FileAllIn1ReadToken (&alloldtokens, filename);
    /* It is OK if the old file does not exit yet.*/
    if (reply < -1) {
	return (-1);  /* error other than no-file.*/
    }
    
    /* Copy the input file to a temporary file,*/
    /* then read the temporary file while rewriting the original file.*/
    sprintf (tmpfilename,"/tmp/tokens.%i", getpid());
    sprintf (cmd,"if [ -f %s ];then cp %s %s;fi", 
    			filename, filename, tmpfilename);
    /*printf ("Rewrite.cmd=%s\n", cmd); */
    system (cmd);
    instream = fopen (tmpfilename, "r");
    /* After fopen above, Allow the input file to be missing.*/

    /* Open Output file.*/
    outstream = fopen (filename, "w");
    if (outstream == NULL) {
	return (-1);  /* Cannot open file */
    }

    /* Read and Write keywords one at a time, */
    /* substituting the given token, copying the rest*/
    /* If a comment is seen, copy it unchanged to the output file.*/
    replaced = 0;
    reply = 0;  /* Assume success */

    for (;;) {
        if (instream==NULL)  break;

	/* Get just one keyword and its values.*/
	tokens_Init (&oldtoken);
	reply = tokens_FileRead1Token (&oldtoken, instream);
	/* printf ("Rewrite.read.  reply=%i, string=%s\n", reply, g_read_string); */
        if (reply) {
            /* Cannot read any more.*/
            /*  Ignore errors and finish writing file.*/
            reply = 0;
            break;
        }

        /* Is there a keyword on the start of this line ? */
        if (oldtoken.Keyword[0] != 0) {
	    /* Yes.*/
	    thiskeyword = oldtoken.Word[0];
	    /* Is this token about to be replaced ? */
	    tokens_Init (&newtoken);
	    reply = tokens_GetThisKeywordsTokens (&giventokens, thiskeyword,
	    						&newtoken);
	    if (reply == 0) {
                /* Same keyword.  Replace its value.*/
		reply = tokens_FileWrite1Token (&newtoken, outstream);
	    } else {
                /* Not the same keyword. Just copy it.*/
		reply = tokens_FileWrite1Token (&oldtoken, outstream);
	    }
        } else {
            /* No keyword. Must be a comment line. Copy it.*/
            /* (original string was saved in global variable).*/
            reply = fprintf (outstream, "%s\n", g_read_string);
	    if (reply > 0) reply = 0;
        }
	if (reply) { break; } /* Any write-error will force an abort.*/
    }

    /* Have finished copying keywords.*/
    /* (but there may be an error) */

    /* If there are new keywords, add them to the file now. */
    if (reply==0) {
    	/* For each new keyword, check if an old keyword existed, */
    	/* and if not, write out the new keyword now.*/
    	
    	for (;;) {
    	    tokens_Init (&newtoken);
            reply = tokens_GetNextKeywordsTokens (&giventokens, &newtoken);
            if (reply)  break;

            /* Any more keywords left ? */
            if (newtoken.Totwords == 0) break;

	    /* Yes.*/
	    thiskeyword = newtoken.Word[0];
	    
	    /* Is this token missing from the original file ? */
	    tokens_Init (&oldtoken);
	    reply = tokens_GetThisKeywordsTokens (&alloldtokens, thiskeyword,
	    						&oldtoken);
	    if (reply != 0) {
                /* New keyword.  Store its value.*/
		reply = tokens_FileWrite1Token (&newtoken, outstream);
	    }
            if (reply)  break;
    	}
    }

    if (instream != NULL) fclose (instream);
    reply2 = fclose (outstream);
    unlink (tmpfilename);
    if (reply || reply2) return (-2);  /* Some error.*/
    return (0);  /* File rewritten OK.*/
}

/**
*--------------------------------------------------------------------------
*								qNwords
*--------------------------------------------------------------------------
* Query total number of words in this structure.
* Returns:  n = success, n words in this structure
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_qNwords   (TOKENS *t) {
    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("qNwords");
    return t->Totwords;
}

/**
*--------------------------------------------------------------------------
*					(internal)	SearchForKeyword
*--------------------------------------------------------------------------
* Search the structure (t) for the given keyword, and return its index
* within the tokens-structure.
* Returns: +n = success (n = index within array within tokens-structure.),
*	   -1 = failed, cannot find keyword.
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
static int SearchForKeyword (TOKENS *t, char *keyword) {

    int i, keyword_index, keylength;

    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("GetThisKeywordsTokens");

    /* Search for the beginning of this keyword.*/
    keylength = strlen (keyword);
    keyword_index = -1;
    for (i=0; i<t->Totwords; i++) {
	if (!t->Keyword[i]) continue;
	/*printf ("got '%s', looking for '%s'\n",t->Word[i],keyword);*/
	if (t->Wordlength[i] != keylength) continue;
	if (strncmp (t->Word[i], keyword, keylength) != 0) continue;
	/* Got equal keywords.*/
	keyword_index = i;
	break;
    }
    return (keyword_index);  /* -1 or +n */
}

/**
*--------------------------------------------------------------------------
*								ToString
*--------------------------------------------------------------------------
* Copy all tokens into one long character string.
* Put a space-character before '=' characters.
* Returns:  0 = success, character-string stored.
*	   -1 = error, string filled up, some tokens not stored.
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_ToString   (TOKENS *t, char *outstring, int maxlen) 
{
    int i, len, outlen, keyword, previous_output;

    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("ToString");
    if (maxlen < 1)  return (-1);  /* outstring too short */
    outstring [0] = '\0';
    outlen = 0;
    previous_output = 0;
    for (i=0; i<t->Totwords; i++) {
	len = t->Wordlength [i];
	if (t->Word [i][len-1] == '=') {
	    /* We have a 'keyword=' word.*/
	    /* Copy an extra character 'keyword=?', then later overwrite ' ='.*/
	    len++;
	    keyword = 1;
	} else {
	    keyword = 0;
	}
	
	if ( (outlen+len+1) > maxlen)  return (-1);  /* outstring too short */
	if (previous_output != 0) {
	    /* After the first word, */
	    /*  put an extra space before putting extra words.*/
	    outstring[outlen] = ' ';
	    outlen++;
	}
	previous_output = 1;
	strncpy (&outstring[outlen], t->Word [i], len);
	outlen += len;
	/* Only for words 'keyword=', overwrite trailing '=?' with ' ='.*/
	if (keyword) strncpy (&outstring[outlen-2], " =", 2);
	outstring[outlen] = '\0';
    }
    return OK;
}

/**
*----------------------------------------------------------------------------
*								ToValueString
*----------------------------------------------------------------------------
* From a 'keyword= value' tokens structure, return the 'value' part
* as a string. (i.e. without its keyword and without any quotes etc.)
* Only returns the value string for the first keyword in the structure.
* Returns:  0 = success, character-string stored.
*	   -1 = error, string filled up, some tokens not stored.
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_ToValueString   (TOKENS *t, char *outstring, int maxlen) {
    int i, len, outlen, nwordscopied;
    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("ToValueString");
    if (maxlen < 1)  return (-1);  /* outstring too short */
    outstring [0] = '\0';
    outlen = 0;

    /* skip (optional) initial keyword.*/
    i=0;
    if (t->Keyword[i]) i++;
    nwordscopied = 0;
    for (; i<t->Totwords; i++) {
	if (t->Keyword[i]) break;  /* stop at the next keyword */
	len = t->Wordlength [i];
	if ( (outlen+len+1) > maxlen)  return (-1);  /* outstring too short */
	if (nwordscopied != 0) {
	    outstring[outlen] = ' ';
	    outlen++;
	}
	strncpy (&outstring[outlen], t->Word [i], len);
	outlen += len;
	outstring[outlen] = '\0';
	nwordscopied++;
    }
    return OK;
}

/**
*--------------------------------------------------------------------------
*							GetWords
*--------------------------------------------------------------------------
* Return an array of pointers to words in this tokens-structure.
* (Each word is a normal character string with a zero-terminator).
* Returns: +n = success (found tokens-structure returned),
*	   -2 = error, could not store all words, however the 1st maxwords
*			are stored.
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_GetWords (TOKENS *t, char *words[], int maxwords) {

    int i, nwords, reply;

    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("GetWords");

    if (t->Totwords > maxwords)
		{nwords=maxwords;    reply=-2;} /* not enough room.*/
    else	{nwords=t->Totwords; reply=nwords;}

    /* Copy pointers to each word-string.*/
    for (i=0; i < nwords; i++) {
	words[i] = t->Word[i];
    }

    return (reply);
}

/**
*--------------------------------------------------------------------------
*							DeleteThisKeyword
*--------------------------------------------------------------------------
* Search the structure (t) for the given keyword, and then delete it
* and its extra words.
* Note:	The keyword within the structure is disabled by giving it zero-length.
*	This does not release any storage within the structure.
*	So if you are adding new keywords, be careful not to overflow the 
*	structure.
* Returns:  0 = success (keyword found and deleted),
*	   -1 = failed, cannot find keyword.
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_DeleteThisKeyword (TOKENS *t, char *keyword) {

    int i, got_keyword, last_word, nwords;

    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("DeleteThisKeyword");

    /* Search for the beginning of this keyword.*/
    got_keyword = SearchForKeyword (t, keyword);

    if (got_keyword == -1) return (-1);  /* Keyword not found */
    if (got_keyword < 0)   return (-9);  /* not initialised. */

    /* Find out all 'words' up to next keyword.*/
    last_word = got_keyword;
    for (i=got_keyword; i<t->Totwords; i++) {
	/* Exit if the next word is a keyword.*/
	if ( ((i+1)>=t->Totwords) || (t->Keyword[i+1])) break;
	last_word++;
    }
    
    /* Copy remaining words up into the place where the deleted keyword was.*/
    nwords = last_word - got_keyword + 1;
    for (i=last_word+1; i<t->Totwords; i++) {
	t->Keyword    [i-nwords] = t->Keyword    [i];
	t->Wordlength [i-nwords] = t->Wordlength [i];
	t->Word       [i-nwords] = t->Word       [i];
    } 
    t->Totwords = t->Totwords - nwords;
    return (OK);
}

/**
*--------------------------------------------------------------------------
*						GetThisKeywordsWords
*--------------------------------------------------------------------------
* Search the structure (t) for the given keyword, and return, in the given
* array, pointers to each of the words in this keyword sequence.
* Note (1) The pointers point to zero-delimited-strings held within the
*	   tokens-structure, and hence the pointers are only valid while the
*	   structure is still in storage.
* Note (2) If the given word-array is filled, no indication is given that
*	   there may be further words yet to be returned.
* Returns: +n = success (n words returned),
*	   -1 = failed, cannot find keyword.
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_GetThisKeywordsWords (TOKENS *t, char *keyword, char *words[],
					int maxwords) {

    int i, got_keyword, nwords;

    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("GetThisKeywordsWords");

    /* Search for the beginning of this keyword.*/
    got_keyword = SearchForKeyword (t, keyword);

    if (got_keyword == -1) return (-1);  /* Keyword not found */
    if (got_keyword < 0)   return (-9);  /* not initialised. */

    /* Now copy the words until next-keyword or end-of-list or error.*/
    nwords = 0;
    for (i=got_keyword; i<t->Totwords; i++) {
	nwords++;
	if (nwords > maxwords) return (maxwords);
	words[nwords-1] = t->Word[i];
	/* Exit if the next word is a keyword.*/
	if ( ((i+1)>=t->Totwords) || (t->Keyword[i+1])) break;
    }
    return (nwords);
}

/**
*--------------------------------------------------------------------------
*                                                      GetNextKeywordsWords
*--------------------------------------------------------------------------
* From the structure (t) get the next keyword, and return, in the given
* array, pointers to each of the words in this keyword sequence.
* Note (1) The pointers point to zero-delimited-strings held within the
*          tokens-structure, and hence the pointers are only valid while the
*          structure is still in storage.
* Note (2) If the given word-array is filled, no indication is given that
*          there may be further words yet to be returned.
* Returns: +n = success (n words returned),
*          -1 = failed.
*          -9 = error, not initialised yet. (See log msg TOK001)
* Note: when n-words-returned becomes zero, it is the end of the structure.
*/
int tokens_GetNextKeywordsWords (TOKENS *t, char *words[], int maxwords) 
{
    int nwords, i;
    
    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("GetNextKeywordsWords");

    /* Start from the next-keyword position, and */
    /* copy the words until another keyword or end-of-list or error.*/
    nwords = 0;
    for (i=t->Nextword; i<t->Totwords; i++) {
        nwords++;
        if (nwords > maxwords) return (maxwords);
        words[nwords-1] = t->Word[i];
        t->Nextword += 1;
        /* Exit if the next word is a keyword.*/
        if ( ((i+1)>=t->Totwords) || (t->Keyword[i+1])) break;
    }
    return (nwords);
}

/**
*--------------------------------------------------------------------------
*						GetThisKeywordsTokens
*--------------------------------------------------------------------------
* Search the structure (t) for the given keyword, and return its tokens in a
* second (smaller) structure (kwtokens).
* Returns:  0 = success (found tokens-structure returned),
*	   -1 = failed, cannot find keyword.
*	   -2 = error, could not store all tokens. (Structure filled,
*			or other error).
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_GetThisKeywordsTokens (TOKENS *t, char *keyword, TOKENS *kwtokens) {

    int i, got_keyword, reply;

    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("GetThisKeywordsTokens");
    if (kwtokens->Magic != TOKENS_MAGIC)  return NotInitYet("GetThisKeywordsTokens");

    /* Search for the beginning of this keyword.*/
    got_keyword = SearchForKeyword (t, keyword);

    if (got_keyword == -1) return (-1);  /* Keyword not found */
    if (got_keyword < 0)   return (-9);  /* not initialised. */

    /* Now copy the words until next-keyword or end-of-list or error.*/
    for (i=got_keyword; i<t->Totwords; i++) {
	reply = tokens_AddWord (kwtokens, t->Word[i], t->Wordlength[i]);
	if (reply) return (reply);  /* Error while copying. */
	/* Exit if the next word is a keyword.*/
	if ( ((i+1)>=t->Totwords) || (t->Keyword[i+1])) break;
    }
    return (OK);
}

/**
*--------------------------------------------------------------------------
*						GetNextKeywordsTokens
*--------------------------------------------------------------------------
* Search the structure for the next keyword, and return its tokens in a
* second (smaller) structure.
* Returns:  0 = success (found tokens-structure returned),
*	   -1 = error, could not store all tokens. (Structure filled)
*	   -2 = error, other error.
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_GetNextKeywordsTokens (TOKENS *t, TOKENS *kwtokens) {

    int i, reply;

    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("GetNextKeywordsTokens");
    if (kwtokens->Magic != TOKENS_MAGIC)  return NotInitYet("GetNextKeywordsTokens");

    /* Copy the words until next-keyword or end-of-list or error.*/
    for (i=t->Nextword; i<t->Totwords; i++) {
	reply = tokens_AddWord (kwtokens, t->Word[i], t->Wordlength[i]);
	if (reply) return (reply);  /* Error while copying. */
	t->Nextword += 1;
	/* Exit if the next word is a keyword.*/
	if ( ((i+1)>=t->Totwords) || (t->Keyword[i+1])) break;
    }
    return (OK);
}

/**
*--------------------------------------------------------------------------
*						RestartNextKeywords
*--------------------------------------------------------------------------
* Restart the counter so that future calls to "GetNextKeywordsTokens"
* will begin at the first keyword in the tokens-structure.
* Returns:  0 = success.
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_RestartNextKeywords (TOKENS *t) {

    if (t->Magic != TOKENS_MAGIC)  return NotInitYet("RestartNextKeywords");

    t->Nextword = 0;
    return (OK);
}

/**
*--------------------------------------------------------------------------
*							KeywordIs
*--------------------------------------------------------------------------
* Test if the first word of this structure is equal to this keyword.
* Returns:  1 = TRUE = yes, keywords are equal;
*	    0 = FALSE = no, not-equal.
*/
int tokens_KeywordIs (TOKENS *t, char *keyword) {

    int keylength;

    if (t->Magic != TOKENS_MAGIC)  return (FALSE);  /* Not initialised yet.*/

    if (keyword==NULL)  return (FALSE);
    keylength = strlen (keyword);
    if (!t->Keyword[0])  return (FALSE);
    if (t->Wordlength[0] != keylength)  return (FALSE);
    if (strncmp (t->Word[0], keyword, keylength) != 0)  return (FALSE);
    /* Got equal keywords.*/
    return (TRUE);
}

/**
*---------------------------------------------------------------------------
*							tokens_Filter
*---------------------------------------------------------------------------
* Compare a given tokens structure (RAWDATA) with a filter token structure,
* (FILTER) and return whether the given data passes the filter.
*
* For each keyword=value in the FILTER,
* the RAWDATA tokens are tested to see they also contain these keyword=value
* pairs.
*
* e.g.
*     RAWDATA:  DATE= 950403  ORIGIN= HYDRO  MSG= Rainfall exceeds 10mm
*  and FILTER:  ORIGIN= HYDRO
*
* then this RAWDATA would pass the filter.
*
* Returns:	 1 = success, passes filter,
*		 0 = no match (does not pass filter)
*		-1 = error, bad parameters or see log msg.
*/
int tokens_Filter (TOKENS *rawdata, TOKENS *filter) {

    char *filterkeyword, *filtervalue;
    int nwords, reply, wordno;

    if (filter->Magic != TOKENS_MAGIC)  return NotInitYet("Filter");
    if (rawdata->Magic != TOKENS_MAGIC)  return NotInitYet("Filter");

    /* Initial SHORT implementation: */
    /*   only use 1st keyword=value from filter.*/

    /* Fetch the filter "keyword=" and "value".*/
    nwords = filter->Totwords;
    if (nwords<2) return (-1);  /* bad filter */
    if (!filter->Keyword[0]) return (-1);  /* bad filter, value instead of kw */
    if ( filter->Keyword[1]) return (-1);  /* bad filter, kw instead of value */
    filterkeyword = filter->Word[0];
    filtervalue   = filter->Word[1];

    /* Now look inside the rawdata for this filterkeyword.*/
    wordno = SearchForKeyword (rawdata, filterkeyword);
    if (wordno<0) return (0);  /* no match */

    /* Check value after keyword */
    wordno++;
    if (wordno >= rawdata->Totwords) return (0); /* no more data, so no match */
    if (rawdata->Keyword[wordno]) return (0); /* not a value, so no match */
    reply = strcmp (rawdata->Word[wordno], filtervalue);
    if (reply != 0)  return (0);  /* strings not equal, so no match.*/
    return (1);  /* success, equal keywords and values.*/
}

/**
*---------------------------------------------------------------------------
*								PrintfTokens
*---------------------------------------------------------------------------
* Print out a tokens structure using the supplied printf-format-string,
* which MUST INCLUDE a %s to print the tokens string itself.
* This is a convenience function for debuggers to print out a labelled
* tokens string. 
* e.g.  tokens_PrintfTokens (&tokens,"After Addstring: %s\n");
*
*  ??????yet to be fixed for multiline outputs....?????
* Returns:  0 = OK,
*	   -1 = error, string filled up, some tokens not stored.
*	   -9 = error, not initialised yet. (See log msg TOK001)
*/
int tokens_PrintfTokens (TOKENS *t, char *format) {
    int reply;
    char outstring [TOKENS_MAX_STRING_LENGTH];

    if (t->Magic != TOKENS_MAGIC)  return (NotInitYet("PrintfTokens"));
    strcpy (outstring,"");
    reply = tokens_ToString (t, outstring, TOKENS_MAX_STRING_LENGTH);
    printf (format, outstring);
    return reply;
}

/**
*----------------------------------------------------------------------------
*								PrintfArgs
*----------------------------------------------------------------------------
* A small helper subroutine used to see the effect of input arguments
* given to main programs.
* Returns:  0 = OK,
*	   -1 = error, too many tokens, some tokens not stored.
*	   -9 = internal error, not initialised yet. (See log msg TOK001)
*/
int tokens_PrintfArgs (int argc, char *argv[]) {

    TOKENS t1_real, *t1=&t1_real;
    int reply, reply2, i;

    /* Now print out calling arguments.*/
    tokens_Init (t1);
    printf ("\nInput Auguments (words) are:\n");
    /* Woops, side-effect: if any added-string starts a # comment, */
    /* 	that fact will be lost when further strings are added, */
    /*		any futher "comments words" will be taken as normal tokens.*/
    /* I'm not sure what to do about this...*/
    for (i=0; i<argc; i++) {
	printf ("%s\n", argv[i]);
	reply = tokens_AddString (t1,argv[i]);
	if (reply)  {
	    printf ("Error: too many tokens or other error.\n");
	}
    }
    reply2 = tokens_PrintfTokens (t1, "\nStored tokens are:\n%s\n");
    if (reply2)  {
	printf ("Error: Internal error in PrintfTokens.\n");
	return (reply2);
    }
    if (reply) return (reply);
    return OK;
}

/**
*---------------------------------------------------------------------------
*							tokens_PrintfFile
*----------------------------------------------------------------------------
* Read a file-of-tokens into an array of tokens,
* and then print out all tokens.
* (this is a small helper routine to help display tokens-files for debugging)
* Returns:  0 = OK,
*	   -1 = error, too many tokens, some tokens not stored/printed.
*	   -9 = internal error, not initialised yet. (See log msg TOK001)
*/
#ifdef TurboC
#define TOKENS_FILE_MAXTOKENS 19
#else
#define TOKENS_FILE_MAXTOKENS 1000
#endif
int tokens_PrintfFile (char *filename) {

    TOKENS t[TOKENS_FILE_MAXTOKENS];
    int i, reply, ntokenlines, nerrors;

    printf ("\nTokens in file '%s' are:\n", filename);

    nerrors = 0;
    for (i=0; i<TOKENS_FILE_MAXTOKENS; i++) {
	tokens_Init (&t[i]);
    }

    /* Read the whole file a tokens-array. */
    ntokenlines = tokens_FileReadAllTokens (t, TOKENS_FILE_MAXTOKENS, filename);
    if (ntokenlines < 0)  {
	printf (" Error: too many tokens or lines.\n");
	nerrors++;
	ntokenlines = TOKENS_FILE_MAXTOKENS;
    }
    if (ntokenlines == 0) {
	printf (" Empty file.\n");
	return (0);
    }

    for (i=0; i<ntokenlines; i++) {
	if (tokens_qNwords (&t[i]) <= 0)  break;  /* dont go past EOF */
	reply = tokens_PrintfTokens ( &t[i], "%s\n");
	if (reply) {
	    printf ("Error: Internal error in PrintfTokens.\n");
	    nerrors++;
	}
    }
    if (nerrors)  printf (" %i Errors seen. (e.g. line or token overflows)\n",
				nerrors);
    return OK;
}

/**
*--------------------------------------------------------------------------
*								SelfTest
*--------------------------------------------------------------------------
* Main self-test control routine, which calls others to do individual
* selftests.
* ...?????...need to add error-descriptions-and-line-trace.....????
* Returns:  0 = OK, success.
*          -n = error, failure code.
*/
#define OUTSTRINGLEN 99

int tokens_SelfTest (void) {

    TOKENS t1_real, *t1=&t1_real;
    TOKENS t2_real, *t2=&t2_real;
    int reply, nwords, i;
    char *outwords[9];
    char *expectwords[9], *instring, *instringb, *expectstring;
    char outstring[OUTSTRINGLEN];

    /* Assume normal running, and check normal operations.*/
    /* (A later check will test the boundary conditions.) */

    printf (" Test 1 word in and out.\n");
    instring = "testtokenskqz";
    tokens_Init (t1);
    reply = tokens_AddString (t1,instring);
    if (reply)  return (reply);
    reply = tokens_ToString (t1, outstring, OUTSTRINGLEN);
    if (reply)  return (reply);
    if ( strcmp (instring,outstring) != 0)  return -1;
    reply = tokens_GetWords (t1, outwords, 99);
    if (reply != 1) return -1;
    if ( strcmp (instring,outwords[0]) != 0)  return -1;

    printf (" Test a whole string in and out.\n");
    instring = "  test	 no=blanks tokens= z a separate = sign # comments";
    expectstring = "test no = blanks tokens = z a separate = sign";
    nwords = 8;
    expectwords[0] = "test";
    expectwords[1] = "no=";
    expectwords[2] = "blanks";
    expectwords[3] = "tokens=";
    expectwords[4] = "z";
    expectwords[5] = "a";
    expectwords[6] = "separate=";
    expectwords[7] = "sign";
    tokens_Init (t1);
    reply = tokens_AddString (t1,instring);
    if (reply)  return (reply);
    reply = tokens_ToString (t1, outstring, OUTSTRINGLEN);
    if (reply)  return (reply);
    if ( strcmp (outstring,expectstring) != 0)  return -1;
    reply = tokens_GetWords (t1, outwords, 9);
    if (reply != nwords) return -1;
    for (i=0; i<nwords; i++) {
	if ( strcmp (outwords[i], expectwords[i]) != 0)  return -1;
    }

    printf (" Test a quoted strings in and out.\n");
    instring = " 'test'	 no='b' tokens=\"z= a\" 'c d'=a'=b 'c 'notrailq";
    expectstring = "'test' no = 'b' tokens = \"z= a\" 'c d' = a'=b 'c 'notrailq";
    nwords = 8;
    expectwords[0] = "'test'";
    expectwords[1] = "no=";
    expectwords[2] = "'b'";
    expectwords[3] = "tokens=";
    expectwords[4] = "\"z= a\"";
    expectwords[5] = "'c d'=";
    expectwords[6] = "a'=b 'c";
    expectwords[7] = "'notrailq";
    tokens_Init (t1);
    reply = tokens_AddString (t1,instring);
    if (reply)  return (reply);
    reply = tokens_ToString (t1, outstring, OUTSTRINGLEN);
    if (reply)  return (reply);
    if ( strcmp (outstring,expectstring) != 0)  return -1;
    reply = tokens_GetWords (t1, outwords, 9);
    if (reply != nwords) return -1;
    for (i=0; i<nwords; i++) {
	if ( strcmp (outwords[i], expectwords[i]) != 0)  return -1;
    }

    printf (" Test adding 2 tokens-structures.\n");
    instring = "  test	 tokens =kqz  # comments";
    instringb= "	2nd	kw = tokens string. # more comments.";
    expectstring = "test tokens = kqz 2nd kw = tokens string.";
    tokens_Init (t1);
    reply = tokens_AddString (t1,instring);
    if (reply)  return (reply);
    tokens_Init (t2);
    reply = tokens_AddString (t1,instringb);
    if (reply)  return (reply);
    reply = tokens_ToString (t1, outstring, OUTSTRINGLEN);
    if (reply)  return (reply);
    if ( strcmp (outstring,expectstring) != 0)  return -1;

    printf (" Fetch a keyword.\n");
    instring = "a b c test= kqz cde last= qzk # comments";
    expectstring = "test = kqz cde";
    nwords = 3;
    expectwords[0] = "test=";
    expectwords[1] = "kqz";
    expectwords[2] = "cde";
    tokens_Init (t1);
    reply = tokens_AddString (t1,instring);
	if (reply)  return (reply);

    tokens_Init (t2);
    reply = tokens_GetThisKeywordsTokens (t1, "test=", t2);
	if (reply)  return (reply);
    reply = tokens_ToString (t2, outstring, OUTSTRINGLEN);
	if (reply)  return (reply);
    if ( strcmp (outstring,expectstring) != 0)  return -1;

    reply = tokens_GetThisKeywordsWords (t1, "test=", outwords, 9);
    if (reply != nwords) return -1;
    for (i=0; i<nwords; i++) {
	if ( strcmp (outwords[i], expectwords[i]) != 0)  return -1;
    }

    printf (" Fetch next keywords tokens.\n");
    expectstring = "a b c";
    tokens_Init (t2);
    reply = tokens_GetNextKeywordsTokens (t1, t2);
	if (reply)  return (reply);
    reply = tokens_ToString (t2, outstring, OUTSTRINGLEN);
	if (reply)  return (reply);
    if ( strcmp (outstring,expectstring) != 0)  return -1;

    expectstring = "test = kqz cde";
    tokens_Init (t2);
    reply = tokens_GetNextKeywordsTokens (t1, t2);
	if (reply)  return (reply);
    reply = tokens_ToString (t2, outstring, OUTSTRINGLEN);
	if (reply)  return (reply);
    if ( strcmp (outstring,expectstring) != 0)  return -1;

    /* Restart next keywords */
    reply = tokens_RestartNextKeywords (t1);
	if (reply)  return (reply);
    expectstring = "a b c";
    tokens_Init (t2);
    reply = tokens_GetNextKeywordsTokens (t1, t2);
	if (reply)  return (reply);
    reply = tokens_ToString (t2, outstring, OUTSTRINGLEN);
	if (reply)  return (reply);
    if ( strcmp (outstring,expectstring) != 0)  return -1;

    printf (" Fetch next keywords words.\n");
    nwords = 3;
    expectwords[0] = "test=";
    expectwords[1] = "kqz";
    expectwords[2] = "cde";
    reply = tokens_GetNextKeywordsWords (t1, outwords, 9);
        if (reply != nwords) return -1;
    for (i=0; i<nwords; i++) {
        if ( strcmp (outwords[i], expectwords[i]) != 0)  return -1;
    }

    nwords = 2;
    expectwords[0] = "last=";
    expectwords[1] = "qzk";
    reply = tokens_GetNextKeywordsWords (t1, outwords, 9);
        if (reply != nwords) return -1;
    for (i=0; i<nwords; i++) {
        if ( strcmp (outwords[i], expectwords[i]) != 0)  return -1;
    }

    printf (" Test ToValueString.\n");
    /*  skip keyword, only 1 keyword*/
    tokens_Init (t1);
    expectstring = "a 1 2 3 4 5 6";
    tokens_AddString (t1,"MSG= a 1 2 3 4 5 6 ");
    reply = tokens_ToValueString (t1, outstring, OUTSTRINGLEN);
	if (reply)  return (reply);
    if ( strcmp (outstring,expectstring) != 0)  return -1;

    /*  skip keyword, many keywords*/
    tokens_Init (t1);
    expectstring = "b 1 2 3 4 5 6";
    tokens_AddString (t1,"MSG= b 1 2 3 4 5 6 NEXT= 7");
    reply = tokens_ToValueString (t1, outstring, OUTSTRINGLEN);
	if (reply)  return (reply);
    if ( strcmp (outstring,expectstring) != 0)  return -1;

    /*  no keyword to skip*/
    tokens_Init (t1);
    expectstring = "c 1 2 3 4 5 6";
    tokens_AddString (t1," c 1 2 3 4 5 6 NEXT= 7");
    reply = tokens_ToValueString (t1, outstring,OUTSTRINGLEN);
	if (reply)  return (reply);
    if ( strcmp (outstring,expectstring) != 0)  return -1;

    printf (" Test DeleteKeyword.\n");
    tokens_Init (t1);
    tokens_AddString (t1,"DATE = 1 MSG = 2 ORIGIN = HYDRO MSGTYPE = 6");
    
    /*  unsuccessful deletes */
    reply = tokens_DeleteThisKeyword (t1,"MSG1=");
    if (reply != -1) return (reply);
    reply = tokens_DeleteThisKeyword (t1,"2DATE=");
    if (reply != -1) return (reply);
    reply = tokens_DeleteThisKeyword (t1,"ZZZ=");
    if (reply != -1) return (reply);
   
    /*  successful deletes */
    
    /* middle keyword*/
    reply = tokens_ToString (t1, outstring, OUTSTRINGLEN);
	if (reply)  return (reply);
    expectstring = "DATE = 1 ORIGIN = HYDRO MSGTYPE = 6";
    /*tokens_PrintfTokens (t1,"Before DeleteKeywords: %s\n");*/
    reply = tokens_DeleteThisKeyword (t1,"MSG=");
    /*tokens_PrintfTokens (t1," After DeleteKeywords: %s\n");*/
    if (reply != 0) return (reply);
    reply = tokens_ToString (t1, outstring, OUTSTRINGLEN);
	if (reply)  return (reply);
    if ( strcmp (outstring,expectstring) != 0)  return -1;
   
    /* end keyword*/
    expectstring = "DATE = 1 ORIGIN = HYDRO";
    reply = tokens_DeleteThisKeyword (t1,"MSGTYPE=");
    if (reply != 0) return (reply);
    reply = tokens_ToString (t1, outstring, OUTSTRINGLEN);
	if (reply)  return (reply);
    if ( strcmp (outstring,expectstring) != 0)  return -1;
    /* start keyword*/
    expectstring = "ORIGIN = HYDRO";
    reply = tokens_DeleteThisKeyword (t1,"DATE=");
    if (reply != 0) return (reply);
    reply = tokens_ToString (t1, outstring, OUTSTRINGLEN);
	if (reply)  return (reply);
    if ( strcmp (outstring,expectstring) != 0)  return -1;

    printf (" Test Filter.\n");
    /*  successful filter */
    tokens_Init (t1);
    tokens_Init (t2);
    tokens_AddString (t1,"DATE= 1 MSG= 2 ORIGIN= HYDRO MSGTYPE= 6");
    tokens_AddString (t2,"ORIGIN= HYDRO");
    reply = tokens_Filter (t1,t2);
    if (reply != 1) return (reply);

    /*  unsuccessful filter */
    tokens_Init (t2);
    tokens_AddString (t2,"ORIGIN= NOTHYDRO");
    reply = tokens_Filter (t1,t2);
    if (reply != 0) return (reply);

    /* unsuccessful filter */
    tokens_Init (t1);
    tokens_Init (t2);
    tokens_AddString (t1,"DATE= 1 MSG= 2 ORIGIN= ");
    tokens_AddString (t2,"ORIGIN= HYDRO");
    reply = tokens_Filter (t1,t2);
    if (reply != 0) return (reply);

    /*  bad filter */
    tokens_Init (t2);
    tokens_AddString (t2,"ORIGIN=");
    reply = tokens_Filter (t1,t2);
    if (reply != -1) return (reply);

    /*  bad filter */
    tokens_Init (t2);
    tokens_AddString (t2,"ORIGIN BAD=");
    reply = tokens_Filter (t1,t2);
    if (reply != -1) return (reply);

    /*  bad filter */
    tokens_Init (t2);
    tokens_AddString (t2,"ORIGIN= BAD=");
    reply = tokens_Filter (t1,t2);
    if (reply != -1) return (reply);

    /* Test reading-in a whole file of tokens.*/
    reply = tokens_SelfTestFile ();
    if (reply) return (reply);

    return (OK);
}

/**
*---------------------------------------------------------------------------
*								SelfTestFile
*----------------------------------------------------------------------------
* Selftest reading a file-of-tokens into an array of tokens.
* Returns:  0 = OK, success.
*          -n = error, failure code.
*/
int tokens_SelfTestFile (void) {

    TOKENS t[9], t2;
    int i, reply, nstrings, ntokens, nwords, nwords_allin1;
    int nkeywords, nkeywords_allin1;
    char *filename, *string_out[9], *expect_in[9], *expect_allin1, string[99];
    FILE *stream;

    /* Specify strings to be written out, */
    /* and their expected results when read back in.*/
    string_out[0] = "0= a b \n";
    expect_in [0] = "0 = a b";
    string_out[1] = "1a \\\n=1b 1c\n";
    expect_in [1] = "1a = 1b 1c";    /* see below where this updated. */
    string_out[2] = "2a\\\n2b 2\\\nc \n";
    expect_in [2] = "2a2b 2c";
    string_out[3] = "3a\\\n3b 3\\\nc= # comments\n";
    expect_in [3] = "3a3b 3c =";
    string_out[4] = "# comments\n 4a\n";
    expect_in [4] = "4a";
    nstrings = 4 + 1;
    
    expect_allin1 = "0 = a b 1a = 1b 1c 2a2b 2c 3a3b 3c = 4a";
    nwords_allin1 = 11;
    nkeywords_allin1 = 3;
    for (i=0; i<9; i++) {
	tokens_Init (&t[i]);
    }

    /* First write out a test-file containing all "string_out" strings.*/
    printf (" Write test-file tokens1.tzt\n");
    filename = "tokens1.tzt";
    stream = fopen (filename,"w");
    if (stream == NULL) return (-1); /* Cannot open file.*/
    for (i=0; i<nstrings; i++) {
	reply = fprintf (stream, "%s", string_out[i]);
	if (reply <=0)  {
	    fclose (stream);
	    return (-2);  /* Cannot write file.*/
	}
    }
    fclose (stream);

    /* Now read the whole file back, into a tokens-array. */
    printf (" FileReadAllTokens %s\n", filename);
    ntokens = tokens_FileReadAllTokens (t, 9, filename);

    /* and check that the expected contents of the tokens.*/
    if (ntokens != nstrings)  
	return (-3);  /* Did not read in the right number of strings.*/
    for (i=0; i<nstrings; i++) {
	reply = tokens_ToString ( &t[i], string, 99);
	if (reply) return (-4); /* Woops internal error.*/
	if (strcmp (string, expect_in[i]) != 0)  return (-5); /* Unexpected.*/
    }

    /* Now read the whole file back, into a single tokens-array. */
    printf (" FileAllIn1ReadToken %s\n", filename);
    tokens_Init (&t2);
    nwords = tokens_FileAllIn1ReadToken (&t2, filename);

    /* and check that the expected contents of the token.*/
    if (nwords != nwords_allin1) 
	return (-3);  /* Did not read in the right number of strings.*/
    reply = tokens_ToString (&t2, string, 99);
    /*printf ("tokens: '%s', nwords=%i\n", string, nwords);*/
    /*printf ("expect: '%s', nwords=%i\n", expect_allin1, nwords_allin1);*/
    if (reply) return (-4); /* Woops internal error.*/
    if (strcmp (string, expect_allin1) != 0)  return (-5); /* Unexpected.*/


    /* Now write the whole array into another file. */
    filename = "tokens2.tzt";
    printf (" FileWriteAllTokens %s\n", filename);
    reply = tokens_FileWriteAllTokens (t, nstrings, filename);
    if (reply!=nstrings) return (reply);

    /* Now read the whole file back, into a tokens-array. */
    printf (" FileReadAllTokens %s\n", filename);
    for (i=0; i<9; i++) {
	tokens_Init (&t[i]);
    }
    ntokens = tokens_FileReadAllTokens (t, 9, filename);

    /* and check that the expected contents of the tokens.*/
    if (ntokens != nstrings)  
	return (-3);  /* Did not read in the right number of strings.*/
    for (i=0; i<nstrings; i++) {
	reply = tokens_ToString ( &t[i], string, 99);
	if (reply) return (-4); /* Woops internal error.*/
	if (strcmp (string, expect_in[i]) != 0)  return (-5); /* Unexpected.*/
    }

/*---------------------------------------------*/

    /* Write a single tokens-array. */
    filename = "tokens3.tzt";
    printf (" FileAllIn1WriteToken %s\n", filename);
    nkeywords = tokens_FileAllIn1WriteToken (&t2, filename);

    /* and check that the expected contents of the token.*/
    if (nkeywords != nkeywords_allin1)
        return (-3);  /* Did not read in the right number of strings.*/

    expect_in [0] = "0 = a b";
    expect_in [1] = "1a = 1b 1c 2a2b 2c 3a3b";
    expect_in [2] = "3c = 4a";
    nstrings = 2 + 1;
 
    /* Now read the whole file back, into a tokens-array. */
    printf (" FileReadAllTokens %s\n", filename);
    for (i=0; i<9; i++) {
        tokens_Init (&t[i]);
    }
    ntokens = tokens_FileReadAllTokens (t, 9, filename);

    /* and check that the expected contents of the tokens.*/
    if (ntokens != nstrings)  
	return (-3);  /* Did not read in the right number of strings.*/
    for (i=0; i<nstrings; i++) {
        reply = tokens_ToString ( &t[i], string, 99);
        if (reply) return (-4); /* Woops internal error.*/
        if (strcmp (string, expect_in[i]) != 0)  return (-5); /* Unexpected.*/
    }

    /* Now update the file, changing keyword values. */
    printf (" FileAllIn1RewriteString %s\n", filename);
    reply = tokens_FileAllIn1RewriteString ("1a=YES 4z=new line", filename);
    if (reply) return (reply);
    expect_in [1] = "1a = YES";    /* see above where this was defined. */
    expect_in [3] = "4z = new line";    /* see above where this was defined. */
    nstrings = 3 + 1;

    /* Now read the whole file back, into a tokens-array. */
    printf (" FileReadAllTokens %s\n", filename);
    for (i=0; i<9; i++) {
	tokens_Init (&t[i]);
    }
    ntokens = tokens_FileReadAllTokens (t, 9, filename);

    /* and check that the expected contents of the tokens.*/
    if (ntokens != nstrings)  
	return (-3);  /* Did not read in the right number of strings.*/
    for (i=0; i<nstrings; i++) {
	reply = tokens_ToString ( &t[i], string, 99);
	if (reply) return (-4); /* Woops internal error.*/
	if (strcmp (string, expect_in[i]) != 0)  return (-5); /* Unexpected.*/
    }

    /* Now update a missing file, changing keyword values. */
    filename = "tokens4.tzt";
    unlink (filename);
    printf (" FileAllIn1RewriteString %s\n", filename);
    reply = tokens_FileAllIn1RewriteString ("1a = YES 4z=new line", filename);
    if (reply) return (reply);
    expect_in [0] = "1a = YES";    /* see above where this was defined. */
    expect_in [1] = "4z = new line";    /* see above where this was defined. */
    nstrings = 1 + 1;

    /* Now read the whole file back, into a tokens-array. */
    printf (" FileReadAllTokens %s\n", filename);
    for (i=0; i<9; i++) {
        tokens_Init (&t[i]);
    }
    ntokens = tokens_FileReadAllTokens (t, 9, filename);

    /* and check that the expected contents of the tokens.*/
    if (ntokens != nstrings)
        return (-3);  /* Did not read in the right number of strings.*/
    for (i=0; i<nstrings; i++) {
        reply = tokens_ToString ( &t[i], string, 99);
        if (reply) return (-4); /* Woops internal error.*/
        if (strcmp (string, expect_in[i]) != 0)  return (-5); /* Unexpected.*/
    }

    /* Special run to exercise rewriting a CMSS parameter file.*/
    /* Just do the replacement/addition, then do a manual check by eyeball.*/
    filename = "cmss.params.tzt";
    system ("cp cmss.params cmss.params.tzt");
    printf (" FileAllIn1RewriteString %s\n", filename);
    reply = tokens_FileAllIn1RewriteString ("1a= YES NewKeyword=newvalue", filename);
    if (reply) return (reply);

    return OK;
}

/**
*---------------------------------------------------------------------------
*				(conditional)  MAIN program to do self-test.
*----------------------------------------------------------------------------
*/
#ifdef MAIN_tokens

#ifdef TurboC
extern unsigned _stklen=64000;  /* bigger stack */
#endif

int main (int argc, char *argv[]) 
{
    TOKENS t2;
    int reply;
    printf ("\n---------------------------------\n");
    printf ("\nStarting: tokens_SelfTest ...\n");

    /*logc_Open ("", "", "", 1);*/
    printf ("Expect to see a log msg about uninitialised tokens.\n");
    reply = tokens_qNwords(&t2);

    /* Check normal operations.*/
    /* (A later check will test the boundary conditions.) */
    reply = tokens_SelfTest ();
    if (reply)	printf ("\n SelfTest Failed %i\n", reply);
    else	printf ("\n SelfTest Passed.\n");
    if (reply)  return (reply);

    /* See what the input-arguments look like. */
    reply = tokens_PrintfArgs (argc, argv);
	if (reply) {
	    printf ("\n PrintfArgs Failed %i\n", reply);
	    return (reply);
	}

    /* If the user said "file= filename", then load and print it.*/
    if ((argc>=2) && (strcmp(argv[1],"file=")==0)) {
	reply = tokens_PrintfFile (argv[2]);
	if (reply) {
	    printf ("\n PrintFile Failed %i\n", reply);
	    return (reply);
	}
    }
    return (reply);
}

/* Dummy 'logc' during debugging.
int logc (char *msg) { printf ("%s\n",msg); return 0; }
*/
#endif


