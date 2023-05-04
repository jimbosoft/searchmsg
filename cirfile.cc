static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/cirfile.cc,v 1.7 2001/03/05 04:37:04 ace Exp $" ;

// <S>		class CirFile.cc -- a Circular File toolkit.
/****************************************************************************
*
* FUNCTION	class CirFile -- a Circular File toolkit.
*
* PROTOTYPE IN	cirfile.h
*
* SUMMARY	#include "cirfile.h"
*
*		NOTE: 
*		See latest "circfile.h" and "circfile.c" for full details.
*			The list below is out of date.
*
*		int SetDirectory (const char *homedir);
*		int Create (const char *prefix, const int max_records,
*				const int record_size);
*		int SaveTraceMessages (const int yesno);
*		int OpenReader (const char *prefix);
*		int OpenWriter (const char *prefix);
*		int OpenWriteOnly (const char *prefix);
*		int OpenLooker (const char *prefix);
*		int SetStoreRecordno (void *record, void *record_recno);
*		int DiscardUnreadRecords (void);
*		int ResetNtbrNtbw (int ntbr,int ntbw);
*		int ReadUpdate (void *record);
*		int WriteUpdate (void *record);
*		int ReadRecord (void *record, const int record_no);
*		int RewriteRecord (void *record, const int record_no);
*		int AcknowledgeRecord (const int record_no);
*		int RefreshPointers ();
*		int qLrnr ()			i.e. query last-recordno-read
*		int qLrnw ()			i.e. query last-recordno-written
*		int qNtbr ()			i.e. query next-to-be-read
*		int qNtbw ()			i.e. query next-to-be-written
*		int qMaxRecords ()		i.e. query maximum records
*		int qRecordSize ()		i.e. query record size
*		int Close ();
*
* PARAMETERS:
*		char *homedir	(in)	pointer to a static string, the home
*					directory to be used for the next open.
*		char *prefix	(in)	1-20 name to be used in the
*					file names  prefix.dat,  prefix.ptr
*		int  yesno	(in)	0 = no, 1 = yes.
*               int  max_records(in)	the file holds this many records
*		int  record_size(in)	each record is this long (in bytes)
*		void *record	(in/out) one data record
*		void *record_recno (in) integer within data record that will
*					hold record number.
*		int  record_no	(in)	specific record number in the file
*
* RETURN VALUE	0 = success,
*		-2 = (for ReadUpdate only)  no data to be read.
*		-2 = (for OpenWriter/WriteUpdate only)  failed, file is full.
*		-2 = (for AcknowledgeRecord only)  failed, this record is not
*						the ntbr record number.
*		-1 = error, TRACE messages have been logged
*			(or saved for the caller to retrieve it later
*			 with TRACE_PREVIOUS)
*
* DESCRIPTION	This is a toolkit of subroutines for reading and writing
*		circular files containing fixed length records.
*
*		The data records are stored on a file called "yourname.dat"
*		and the pointers on file "yourname.ptr". Pointers are:
*		Ntbw (next to be written), Ntbr (next to be read),
*		MaxRecords  (records in file),  RecordSize  (bytes/record).
*
*		The files are interlocked (using f-lock) so that multiple
*		readers and/or writers can safely update the file any time.
*
*		On HPs the circular files are automatically 'prealloc'ed to
*		to get fast disk accesses.
*
*		Full details of all cirfile operations and conventions are
*		given below in "Usage".
*
* WARNINGS	If an error occurs within CirFile, an explanatory traceback
*		message is put onto the CMSS log file. The messages look like:
*			myfile xxxx failed: (system errno text)
*		(where xxxx is open,read,write,seek,etc).
*		The caller could possibly follow this up with their own Alarm
*		e.g.   "myfile failure. See Log file."
*
*		(Traceback error messages can be saved instead of logged,
*		 see below).
*
*		When new files are created, existing files are lost.
*<F>
*
* USAGE		There are various "roles" of users:
*		"Writers"  write data and update the Ntbw pointer.
*		"Readers"  read data and update the Ntbr pointer.
*		"Lookers"  only read the data and read the pointers.
*			   (Lookers are programs like operator displays)
*
*		(0) All users must declare a circular file object by:
*			CirFile  myfile();
*		    There is no specific need to close the file, this will
*		    be done automatically when the object is destroyed.
*
*		(0b) If you want to access circular files in other than the
*		    current directory, you must precede all Open or Create calls
*		    with a call to set up the home directory of the files.
*			e.g.   myfile.SetDirectory ("/home/fred/files")
*		    Note that after the files are closed (after Create, a bad
*		    Open, or Close) the directory is reset to the current dir.
*
*		(1) The circular files are assumed to exist. So the system
*		    manager must specifically create them using a small main
*		    program calling "myfile.Create(...)".
*			(e.g. see programs createlog, createdir)
*
*		(2) "Writers" must open the files with:
*				myfile.OpenWriter (filename)
*		     and then write their data using
*				myfile.WriteUpdate (record)
*
*		(3) "Readers" must open the files with:
*				myfile.OpenReader (filename)
*		     and then read their data using
*				myfile.ReadUpdate (record)
*
*		(4) "Lookers" must open the files with:
*				myfile.OpenLooker (filename)
*		     then get the latest pointers by:
*				myfile.RefreshPointers()
*				ntbr = myfile.qNtbr()
*				..etc ntbw, maxrecords, etc
*		     and then read specific records of data using
*				myfile.ReadRecord (record,recordno)
*
*		(5) The query operations qLrnr and qLrnw will reply the record
*		    number of the last record number read or written.
*		    (This is only valid after successful reads/writes).
*
*		(6) The query operations qNtbr, qNtbw will reply the next-to-be-
*		    read or written (record number) pointers as they were after
*		    the last read/write/refresh operation. These pointers are
*		    NOT necesarily the the most up-to-date values on the file
*		    itself (since further operations may have been done on it
*		    by other users).
*		    Remember to use the RefreshPointers() operation to get the
*		    up-to-date values.
*
*		(7) Any users who want to suppress logging of trace messages
*		    issued from CirFile, should use
*				myfile.SaveTraceMessages (1)
*		    before opening the cirfile.  The messages can be retrieved
*		    later by TRACE_PREVIOUS (1) if required.
*
*		(8) AcknowledgeRecord and DiscardUnreadRecords are two
*		    operations that are included for managing files of alarm
*		    messages where individual alarms need to be acknowledged
*		    or excess alarms need to be discarded.
*
*		(9) SetStoreRecordno is a special call to set up a mode where
*		    the record number of each record written is stored within
*		    the record before it is written.
*		    You can invoke this mode just after you have opened a
*		    file for Writing, by doing a single call as follows:
*				SetStoreRecordno (&record, &record.recno)
*		    where "recno" is the integer within the record structure
*		    that will hold its own record number.
*		    (N.B. this call just saves the address of "recno". The
*			actual record numbers are stored during Write calls.)
*
*		(10) ResetNtbrNtbw resets the next-to-be-read and 
*		     next-to-be-written pointers. This is useful for going back
*		     in a circular file to re-process earlier messages.
*		     (e.g. in test-systems, or after a crash.)
*		     Do an OpenWriter before using ResetNtbrNtbw.
*
* SEE ALSO	log, alarm
*
* HISTORY	$Header: /home/commsa/cmdv/cmss/src/RCS/cirfile.cc,v 1.7 2001/03/05 04:37:04 ace Exp $
*
*		v1.7  010305 ACE,  Fixed '// *' comments.
*		v1.6  001123 ACE, trim unused comments and unused code.
*		v1.5  980602 cmdv, source-id.
*		v1.4  941031 ACE, add RewriteRecord, and test-code for it.
*		v1.3  940127 ACE, now calls old-C CIRCFILE subroutines.
*		v1.2  931028 ACE, added ResetNtbrNtbw.
*		v1.1  930825 cmdv, transferred from SUN to HP.
*		(See v1.5 or less for SUN versions before transfer to HP)
*
**************************************************************************/

#include <stdio.h>
#include "cirfile.h"
#include "circfile.h"

#define OK   0

// This source file only contains the C++ code for object 'CirFile'.
// i.e. only the constructor and destructor.
// All of the rest of the code is in ANSI-C subroutines
// in 'circfile.c'.
// The C++ cirfile.h is mainly macros pointing to the ANSI-C routines.

// Main programs for TESTING CirFile are in separate files:
//	cirftest.cc
//	circftst.c

// *************************************************************  Constructor

CirFile::CirFile () {

    ;// Initialises all simple variables for a newly constructed object.

    ;// Note that we force any real work (e.g. opening files)
    ;// to be done in other routines that the user calls later,
    ;// so that sensible error codes can be returned.

    ;// zero-out all variables.
    circfile_Init (&c);

    if (c.Debug) printf (" Constructing a CirFile object.\n");

}


// **************************************************************  Destructor

CirFile::~CirFile () {

    ;// Empty out all internal work prior to the object being destroyed.

    if (c.Debug) printf (" Destroying a CirFile object.\n");

    ;// Close the 2 files.
    circfile_Close (&c);
}


// Main programs for TESTING CirFile are in separate files:
//	cirftest.cc
//	circftst.c

