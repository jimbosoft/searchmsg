#ifndef TOKENS_H
#define TOKENS_H
/** $Header: /home/commsa/cmdv/cmss/src/RCS/tokens.h,v 1.12 2002/07/03 00:27:25 ies Exp $
*
*	tokens.h -- subroutine-toolkit to handle "Tokens-Stuctures".
*
*	A single TOKENS structure can contain many KEYWORDs and/or VALUE-WORDS.
*
* v1.12 2002-07-03 Ian Senior  fix comment.
* v1.11 2002-07-01 C.Edington, added comments.
* v1.10 2002-07-01 C.Edington, added DeleteThisKeyword.
* v1.9  010313 Doug Adams  Increase the size of token string to 10K.
* v1.8  000629 C.Edington, add AllIn1Read/Write/Rewrite, GetNextKeywordsWords.
* v1.7  960223 C.Edington, __STDC__ not __stdc__.
* v1.6  951123 Doug Adams  Increased the size of token string to 2048.
* v1.5  950812 C.Edington, use our own tokeniser. (see *.c code for details)
* v1.4  950406 ACE, added ToValueString.
* v1.3  950405 ACE, added tokens_Filter. Tokens structure up to 512 bytes long.
*			Removed unused WildcardMatch.
* v1.2  941003 ACE, fixed C++, Ansi-C compiles.
* v1.0b 940130 ACE, added GetThisKeywordsWords, RestartNextKeyword.
* v1.0a 931111 ACE, initial code.
*/

/* These two set the size of the structure TOKENS (squeezed of most blanks)*/

#define 	TOKENS_MAX_WORDS	1024
#define TOKENS_MAX_STRING_LENGTH	8192

/* This is used internally as scratch storage when unsqueezed and */
/* commented strings are read from a file.*/
#define TOKENS_MAX_TMP_STRING_LENGTH	8192

/* Below is a magic number that is stored in correctly initialised */
/* TOKENS structures */
#define TOKENS_MAGIC     12401

struct TOKENS_STRUCT {
    int  Totwords;	/* total number of words stored here so far.*/
    int  Totchars;      /* total number of chars stored in String so far.*/
    int  Nextword;	/* index of next word to be got by GetNextKeyword.*/
    int  Keyword    [TOKENS_MAX_WORDS]; /* 0=no, 1=yes */
    int  Wordlength [TOKENS_MAX_WORDS]; /* length of this word.*/
    char *Word      [TOKENS_MAX_WORDS]; /* pointer to actual word.*/
    char String	    [TOKENS_MAX_STRING_LENGTH]; /* Safe storage of all words.*/
    int  Magic;     /* magic number for this type of structure.*/
};
typedef struct TOKENS_STRUCT TOKENS;

#ifdef __cplusplus
extern "C" {
#endif

#if defined (__cplusplus) || defined (__STDC__)


/* Internally used subroutines which may be used standalone if needed.*/
int tokens_TokeniserNext  (char *instring,
				char **tokenpointer, int *tokenlength);
int tokens_StripLeadingWhiteSpace (char *partialstring, int *position);
int tokens_SwallowQuotedString (char *partialstring, int *position);


/* INPUTS to structure */

int tokens_Init       (TOKENS *token);
int tokens_AddWord    (TOKENS *token, char *word, int wordlength);
int tokens_AddString  (TOKENS *token, char *instring);
int tokens_AddTokens  (TOKENS *token, TOKENS *intoken);

/* FILE operations. */

/* Read/write 1 line (many keywords&values) from/to an open file.*/
/* i.e. each line on the file has many keywords/values as 1 structure.*/

int tokens_FileRead1Token  (TOKENS *token, FILE *stream);
int tokens_FileWrite1Token (TOKENS *token, FILE *stream);

/* Read/Write each line to/from an array of tokens strucures.*/
/* i.e. each token in the array of tokens is 1 line of many keywords&values.*/

int tokens_FileReadAllTokens  (TOKENS *tokens, int maxtokens, char *filename);
int tokens_FileWriteAllTokens (TOKENS *tokens, int   ntokens, char *filename);

/* "AllIn1" files have 1 keyword=value per line, and whole file is one token.*/
/* Beware: Comment-lines are lost from files which are read/rewritten.*/
/*	(later there will be a subroutine to preserve comments and formatting)*/
/* Read whole file into one structure.*/
/* Write one structure into one file, with 1 keyword=value per line.*/
/* (read and) Rewrite file, changing one keyword=value (given in a string).*/

int tokens_FileAllIn1ReadToken     (TOKENS *token, char *filename);
int tokens_FileAllIn1WriteToken    (TOKENS *token, char *filename);
int tokens_FileAllIn1RewriteString  (char *string, char *filename);

/* OPERATIONS on, and OUTPUTS from tokens-structure.*/
/* NOTE: for 'ThisKeyword' type calls, use "keyword=" string (include the '='). */
int tokens_qNwords    (TOKENS *token);

int tokens_ToString        (TOKENS *token, char *outstring, int maxlen);
int tokens_ToValueString   (TOKENS *token, char *outstring, int maxlen);

int tokens_GetWords              (TOKENS *token, char *words[], int maxwords);
int tokens_GetNextKeywordsWords  (TOKENS *token, char *words[], int maxwords);
int tokens_GetThisKeywordsWords  (TOKENS *token, char *keyword, 
						 char *words[], int maxwords);
int tokens_GetNextKeywordsTokens (TOKENS *token, TOKENS *kwtoken);
int tokens_GetThisKeywordsTokens (TOKENS *token, char *keyword, 
						 TOKENS *kwtoken);
int tokens_RestartNextKeywords   (TOKENS *token); /* To rescan keywords.*/

/* Delete this keyword and all of its words.*/
/* (Note: this does not free up any space in the structure) */
/* NOTE: for 'ThisKeyword' type calls, use "keyword=" string (include the '='). */
int tokens_DeleteThisKeyword (TOKENS *token, char *keyword); 

/* If Tests: */
/* if (tokens_KeywordIs(t,"kw="))  Yes, */
/*                        this string is the first keyword in this structure.*/
/* if (tokens_Filter(t,"kw1=a kw2=b"))  Yes, this structure contains these.*/
int tokens_KeywordIs             (TOKENS *token, char *keyword);
int tokens_Filter                (TOKENS *token, TOKENS *filter);

int tokens_PrintfTokens (TOKENS *token, char *format);
int tokens_PrintfArgs   (int argc, char *argv[]);
int tokens_PrintfFile   (char *filename);

/* SELF-TEST routines.*/

int tokens_SelfTest  (void);
int tokens_SelfTestFile (void);

#endif  /* __cplusplus or __stdc__ */

#ifdef __cplusplus
}
#endif

#endif


