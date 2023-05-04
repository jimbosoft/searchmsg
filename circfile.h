#ifndef _CIRCFILE_H
#define _CIRCFILE_H

/* "$Header: /home/commsa/cmdv/cmss/src/RCS/circfile.h,v 1.8 2002/06/12 02:13:18 ace Exp $  BoM" */

/**	circfile.h -- Circular file toolkit  (definitions).
*
* Definitions for Circular File toolkit.
* See full documentation in source file:  circfile.c
*
* v1.8  2002-05-30 ACE, multiple-data-files.
* v1.7  001116 ACE, spelling change to qNtbwMinus1 (from qNtbwLess1).
* v1.6  001116 ACE, added qNtbwLess1, IncrementMyNtbr.
* v1.5  001031 ACE, added IncrementRecordNo/DecrementRecordNo for lookers.
* v1.4  980513 ACE, added ResetMaxRecords.
* v1.3  941031 ACE, added RewriteRecord.
* v1.2  940127 ACE, added ResetNtbr, ResetNtbw.
* v1.0e 931222 ACE, added SetUnlockedReads and Writes.
* v1.0d 931130 ACE, added SetSyncWrites, qNavailable, all XxxVariableLength.
* v1.0c 931123 ACE, added qNnotRead.
* v1.0b 931012 ACE, made it DOS compilable. Added Magic check.
* v1.0a 931005 ACE, first "C" language version (adapted from v1.6 C++ cirfile)
*                   Note name change from cirfile to circfile.
*/

/*  Maximum length of the prefix filename for *.DAT and *.TMP files.     */
/*	(This is only the 'name' component of name.dat)                  */
/*	(Later, if we need a directory, we should create a new extra parameter) */
#ifdef TurboC
#define CIRCFILE_MAX_CIRCFILE_NAME  8
#else
#define CIRCFILE_MAX_CIRCFILE_NAME  20
#endif

/*  Maximum length of the home-directory string */
#define CIRCFILE_MAX_HOMEDIR_NAME  80

/*  Structure that holds all details about 1 circular file.*/

enum CIRCFILE_Mode {CLOSED, CREATE, WRITER, READER, WRITEONLY, LOOKER};
#define CIRCFILE_MAGIC 12347

struct CIRCFILE_STRUCT {
    char Prefix [CIRCFILE_MAX_CIRCFILE_NAME+1];
			/* Used to name the 2 files: prefix.DAT, prefix.PTR*/
    int  Max_records;	/* maximum number of records in the file           */
    int  Record_size;	/* number of bytes in each fixed length record     */
    int  Records_per_file; /* number of records per data file	*/
    int  Multiple_files;   /* 0 = single data file, 1 = Multiple files. */
    int  Current_file_number; /* 0...n, which of the multiple data files
				is currently open.  -1 means nothing open.*/
    enum CIRCFILE_Mode Cirfile_mode;
    int  Dat_handle;	/* file handle for  name.DAT file.      */
    int  Ptr_handle;	/* file handle for  name.PTR file.      */
    int  Ntbr;		/* next_to_be_read (a record number)    */
    int  Ntbw;		/* next_to_be_written (a record number) */
    int  Lrnr;		/* last record number read.             */
    int  Lrnw;		/* last record number written.          */
    char *Home_dir;	/* home directory to be used during opens. */
    int  Store_recno;	/* 0=no store, 1=store record number within data record.*/
    int  Recno_index;	/* if(cStore_recno), store the Ntbw record number at   */
			/* this integer position within the user's data, before*/
			/* writing the data record to the file. */
    int  SyncWrites;	/* maybe sync writes direct to disk (non-cached). */
			/* Default is yes locked reads and writes. */
    int  LockedReads;	/* 0=none, 1=lock read-pointer during read-updates.*/
    int  LockedWrites;	/* 0=none, 1=lock write-pointer during write-updates.*/
    int  Save_messages;/* 0 = no dont save them,  1 = yes, save them (dont log) */
    int  Debug;	/* 0=noprints, 1=print debug msgs       */
    int  Magic;		/* must be equal to a special value.*/
			/* This traps uninitialised/overwritten structures.*/
};
typedef struct CIRCFILE_STRUCT CIRCFILE;

#ifdef __cplusplus
extern "C" {
#endif
int circfile_Init (CIRCFILE *c);
int circfile_NotInit (CIRCFILE *c);
int circfile_Close (CIRCFILE *c);
int circfile_SetSyncWrites (CIRCFILE *c, const int yesno);
int circfile_SetDirectory (CIRCFILE *c, const char *homedir);
int circfile_Create (CIRCFILE *c, const char *prefix, const int max_records, 
			const int record_size);
int circfile_CreateMultiFile (CIRCFILE *c, const char *prefix, 
			const int max_records,
			const int record_size, const int records_per_file);
int circfile_SetToMultiFile (CIRCFILE *c, const int records_per_file);
int circfile_SaveTraceMessages (CIRCFILE *c, const int yesno);
int circfile_OpenReader (CIRCFILE *c, const char *prefix);
int circfile_OpenWriter (CIRCFILE *c, const char *prefix);
int circfile_OpenWriteOnly (CIRCFILE *c, const char *prefix);
int circfile_OpenLooker (CIRCFILE *c, const char *prefix);
int circfile_SetUnlockedReads (CIRCFILE *c);
int circfile_SetUnlockedWrites (CIRCFILE *c);
int circfile_SetStoreRecordno (CIRCFILE *c, void *record, void *record_recno);
int circfile_DiscardUnreadRecords (CIRCFILE *c);
int circfile_ResetNtbr (CIRCFILE *c, const int ntbr);
int circfile_ResetNtbw (CIRCFILE *c, const int ntbw);
int circfile_ResetMaxRecords (CIRCFILE *c, const int max_records);
int circfile_LookRecord (CIRCFILE *c, void *record, const int record_no);
int circfile_RewriteRecord (CIRCFILE *c, void *record, const int record_no);
int circfile_ReadUpdate (CIRCFILE *c, void *record);
int circfile_WriteUpdate (CIRCFILE *c, void *record);
int circfile_AcknowledgeRecord (CIRCFILE *c, const int record_no);
int circfile_LookVarLength (CIRCFILE *c, void *record,
				const int maxlength, const int record_no);
int circfile_ReadVarLength (CIRCFILE *c, void *record, const int maxlength);
int circfile_WriteVarLength (CIRCFILE *c, void *record, const int reclength);
int circfile_AckVarLength (CIRCFILE *c, const int record_no,
						const int reclength);
int circfile_RefreshPointers (CIRCFILE *c);

int circfile_qLrnr (CIRCFILE *c);
int circfile_qLrnw (CIRCFILE *c);
int circfile_qNtbr (CIRCFILE *c);
int circfile_qNtbw (CIRCFILE *c);
int circfile_qNtbwMinus1 (CIRCFILE *c);
int circfile_qNnotRead   (CIRCFILE *c);
int circfile_qNavailable (CIRCFILE *c);
int circfile_qMaxRecords (CIRCFILE *c);
int circfile_qRecordsPerFile (CIRCFILE *c);
int circfile_qRecordSize (CIRCFILE *c);
int circfile_qCorrectN (CIRCFILE *c, int n);
int circfile_CorrectRecordNo   (CIRCFILE *c, int record_no);
int circfile_IncrementMyNtbr   (CIRCFILE *c, int *my_ntbr);
int circfile_IncrementRecordNo (CIRCFILE *c, int *record_no);
int circfile_DecrementRecordNo (CIRCFILE *c, int *record_no);

int circfile_SetDebugMode (CIRCFILE *c, const int i);

#ifdef __cplusplus
}
#endif
#endif  /* _CIRCFILE_H */


