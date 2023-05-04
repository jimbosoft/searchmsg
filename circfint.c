static char rcsid[] = "$Header: /home/commsa/cmdv/cmss/src/RCS/circfint.c,v 1.19 2017/04/10 03:35:53 ies Exp $" ;
/**
* 	circfint -- Internal routines only used by circfile toolkit.
*
*
* On the Compile-command you must -D define the target compiler:
*  -DTurboC, or -DHPUX, or other or none.
*
*"$Header: /home/commsa/cmdv/cmss/src/RCS/circfint.c,v 1.19 2017/04/10 03:35:53 ies Exp $"
*
* v1.19 2017-04-10 Ian Senior	Increase MAX_NUM_RECORDS
* v1.14 2015-02-17 Ian Senior	Do not trace log file as will cause
*				a loop and crash.
* v1.14 2013-11-27 Ian Senior	Remove the static string from 
*		circfint_GetFilename() to make this thread-safe
* v1.13 2009-06-03 Ian Senior	Change "ptr record" values from long
*		to int because of 64 bit port. Want to share circular
*		files between 32 bit and 64 bit programs. "int" is 32
*		bits in both cases but long is 32 in one and 64 in the
*		other.
*		
* v1.12 2002-06-18 Ian Senior	Next fix.
* v1.11 2002-06-18 C.Edington	Fix strcpy/srtcat bug in GetFilename.
* v1.10 2002-06-14 C.Edington	Pre-allocate proper size of the last file.
* v1.9  2002-05-30 C.Edington   Suport multiple-data-files.
* v1.8  2002-04-05 Ian Senior	Increase max records allowed.  
* v1.7  001114 ace Prezero file-space when files are extended. 
* v1.6  980619 ies Increase maximum number of records.
* v1.5  980602 cmdv source-control string.
* v1.4  980513 ace Write MAX pointer.
* v1.3  960227 ace Do equivalent to HPs prealloc() on all systems.
* v1.2  941031 ace Write data a given record number (supports RewriteRecord).
* v1.0  931222 ace Separate writes & locks for read vs write pointers.
*
* NOTE: These internal routines do not check their input parameters
*	as much as usual, because the inputs have already been checked
*	by the circfile.c routines which call these circfint.c routines.
*
* Pease excuse lack of comments explaining the range of these routines.
* See circfile.c for general comments and usage.
*  Clive. 
*/


/***************************************************
**
**  PROGRAMMERS BEWARE:
**
**  Within this CIRCFILE source code, if ever you are writing new code to
**  generate new TRACE messages,
**  always do this pair of lines:
**
**		if (c->Save_messages) TRACE_SAVE_NEXT (1);
**		TRACE_...etc. (...your trace message);
**
**  This is because the Log system uses Cirfile, and an error on the logfile
**   (notified by CIRCFILE) has to be prevented from being reported to the
**   logfile (causing an infinite recursion).
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef TurboC
#include <io.h>
#else
#include <sys/file.h>
#include <unistd.h>
#endif
#include <string.h>
#include "circfile.h"
typedef CIRCFILE *C;
#include "circfint.h"
#include "tracec.h"
#include "logc.h"

#define OK   0
#define MAX_NUM_RECORDS 500000000
#define MAX_RECORD_SIZE 16000
#define MAX_FULL_FN  CIRCFILE_MAX_HOMEDIR_NAME+1+CIRCFILE_MAX_CIRCFILE_NAME+10

static char *circfint_GetFilename (C c, const char *extension, char *fullfilename);
/**
*---------------------------------------------------------------------------
*								OpenValidate
*---------------------------------------------------------------------------
* Open a circular file for read or write etc,
*  (depending on the internal mode already set by the caller)
* and retrieve the file's pointer values.
* i.e. opens *.DAT, and opens and read *.PTR.
* Validate the filename and the pointers etc.
* Returns: 0=OK, n = error (and a saved Trace message is stored)
*/
int circfint_OpenValidate (C c, const char *prefix, enum CIRCFILE_Mode new_mode)
{
    int error;

    /* Validate filename. */
    error = circfint_ValidateFilename (c, prefix);
    if (error)  return (error);

    /* Reject the operation if the file is already open. */
    if (c->Cirfile_mode != CLOSED) {
        if (c->Save_messages) TRACE_SAVE_NEXT (1);
	return (TRACE_CHARS (-1,"CIR008F FILE ALREADY OPEN: ",prefix));
    }
    
    c->Cirfile_mode = new_mode;

    /* Open *.PTR (and read it in), */
    /* and open all *.DAT files. */
    error = circfint_OpenFiles (c);

    /* Woops, if there were errors, close the files again. */
    if (error) {
	/* (An error msg has already been generated.) */
	circfile_Close(c);
	return (error);
    }

    /* Set defaults to use file-locking for update operations.*/
    c->LockedReads = 1;
    c->LockedWrites = 1;
    return (OK);
}

/**
*---------------------------------------------------------------------------
*							    ValidateFilename
*---------------------------------------------------------------------------
* Validate the Filename prefix, and if it is OK, store it for use.
* Returns: 0=OK, n = error (and a saved Trace message is stored)
*/
int circfint_ValidateFilename (C c, const char *prefix)
{
    int len;

    /* Validate filename. */
    len = strlen(prefix);
    if (len > CIRCFILE_MAX_CIRCFILE_NAME) {
        if (c->Save_messages) TRACE_SAVE_NEXT (1);
	return (TRACE_CHARS (-1,"CIR001F FILENAME TOO LONG: ",prefix));
    }

    /* Validate directory length. */
    len = strlen(c->Home_dir);
    if (len > CIRCFILE_MAX_HOMEDIR_NAME) {
        if (c->Save_messages) TRACE_SAVE_NEXT (1);
	return (TRACE_CHARS (-1,"CIR009F HOMEDIRNAME TOO LONG: ",c->Home_dir));
    }

    /* Store filename for further use. */
    strcpy (c->Prefix, prefix);
    return (OK);
}

/**
*------------------------------------------------------------------------------
*							    	      OpenFiles
*------------------------------------------------------------------------------
* Opens the *.PTR file (and reads it in),
* and opens all *.DAT files (to create or check them).
*
*	It constructs the filenames from the Prefix name,
*	and opens the *.PTR file always for read & write.
*	and opens the *.DAT file(s) for either read or write,
*	If any errors occur, a trace_msg is saved and an error code is returned.
*/
int circfint_OpenFiles (C c)
{
    int    reply, nfiles, i;

    /* Open *.PTR file, first, so we can tell if this is a multi-dat-circfile.*/

    reply = circfint_OpenPtrFile (c);
    if (reply) return (reply);

    /* Open all *.DAT files. */
    /*  For Creates: unlink old file, create new file and preallocate disk.*/
    /*  For Existing files: open and check that they all exist. */
    /* Note: file 0 is (the first)  file *.dat, */
    /*       file 1 is (the second) file *1.dat, ... file n is *n.dat) */
    if (c->Multiple_files) {
	nfiles = (c->Max_records + c->Records_per_file-1) / c->Records_per_file;
	for (i=0; i<nfiles; i++) {
	    reply = circfint_OpenDatFile (c, i);
	    if (reply) return (reply);
	    if (c->Dat_handle>=0) close (c->Dat_handle);
	    c->Dat_handle = -1;
	}
    }
    else {              
	/* only a single *.dat file */
	reply = circfint_OpenDatFile (c, 0);
	if (reply) return (reply);
    }
    return (reply);
}

/**
*---------------------------------------------------------------------------
*							    	 OpenPtrFile
*---------------------------------------------------------------------------
*	Opens the *.PTR file.
*
*	It constructs the filename from the Prefix name,
*	and opens the *.PTR file (nearly) always for read & write.
*	If any errors occur, a trace_msg is saved and an error code is returned.
*/
int circfint_OpenPtrFile (C c)
{
   int   mode2, s_mask, reply;
   char fullfilename[MAX_FULL_FN];

   if (c->Ptr_handle != -1) {
	/* Normally I would expect the file to be closed. */
	/* Maybe c->Ptr_handle contains garbage. Print an error message.*/
        if (c->Save_messages) TRACE_SAVE_NEXT (1);
	return (TRACE_CHARS (-1,"CIR00?F PTR FILE ALREADY OPEN: ",c->Prefix));
   }

    mode2 = 0;
#ifdef TurboC
    mode2 |= O_BINARY;	/* Stop crlf expansion on DOS */
#endif

    /* OPEN THE cirfile.PTR PARAMETER DATASET (always read-write). */

    /* prefix.PTR filename */
    circfint_GetFilename (c, ".ptr",fullfilename);
    if (strlen(fullfilename)==0) return (-1);
	/*printf ("Opening %s\n",fullfilename);*/

    /* Open *.PTR file */
    if (c->Cirfile_mode==CREATE) {
	/* Create a new file with appropriate read-write access. */
	s_mask = circfint_Get_s_mask ();
	mode2 |= O_RDWR|O_CREAT|O_TRUNC;
	c->Ptr_handle = open (fullfilename, mode2, s_mask);
	/* Assume this created file will only be single user, */
	/* and will be closed very quickly.*/
	/* So dont set any other flags.*/
        if (c->Ptr_handle < 0) {
            if (c->Save_messages) TRACE_SAVE_NEXT (1);
	    TRACE_ERRNO_CHARS (errno,"CIR007F CANNOT CREATE PTR FILE: ",c->Prefix);
	    return (-1);
        }

	/* Write the current pointers to the file */
	reply = circfint_WritePtrs(c, ALL_PTRS);
	if (reply) return (reply);
    }

    else {
	/* Open an existing file. (usually read-write) */
	if (c->Cirfile_mode==LOOKER)	mode2 |= O_RDONLY;
	else				mode2 |= O_RDWR;
#ifndef TurboC
	/* On UNIX, maybe make writes go straight to disk.*/
	/* (aviods losing data in a crash when cache is lost).*/
	/* Recommended for all but high-volume log files.*/
	if (c->SyncWrites) mode2 |= O_SYNC;
#endif
	c->Ptr_handle = open (fullfilename, mode2);

	if (c->Ptr_handle < 0) {
	    /* DONT NOT TRACE LOG FILE AS WILL CAUSE LOOP */
	    if (strcmp(c->Prefix,logc_DEFAULT_FILE_PREFIX)) {
	       if (c->Save_messages) TRACE_SAVE_NEXT (1);
	       TRACE_ERRNO_CHARS (errno,"CIR007F CANNOT OPEN PTR FILE: ",c->Prefix);
	       }
	    return (-1);
	}

	/* Read the current pointers from the file */
	/* (the Read also validates the pointers). */
	reply = circfint_ReadPtrs(c);
	if (reply) return (reply);
   }
   return (0);
}

/**
*---------------------------------------------------------------------------
*							    	 OpenDatFile
*---------------------------------------------------------------------------
*	Opens the *.DAT file.
* Note: file 0 is (the first)  file prefix.dat,
*       file 1 is (the second) file prefix1.dat, ... file n is prefixn.dat
*
*	It constructs the filename from the Prefix name,
*	and opens the *.DAT file for either read or write,
*	If any errors occur, a trace_msg is saved and an error code is returned.
* Returns  0 = Success.
*         -n = Failed. Usually a log msg has been written.
*/
int circfint_OpenDatFile (C c, int file_number)
{
   int   mode2, s_mask, reply, maxrec;
   char fullfilename[MAX_FULL_FN];
   char extension[10];
   
   if (c->Dat_handle != -1) {
	/* Normally I would expect the file to be closed. */
	/* Maybe c->Dat_handle contains garbage. Print an error message.*/
        if (c->Save_messages) TRACE_SAVE_NEXT (1);
	return (TRACE_CHARS (-1,"CIR00?F DAT FILE ALREADY OPEN: ",c->Prefix));
   }

    mode2 = 0;
#ifdef TurboC
    mode2 |= O_BINARY;	/* Stop crlf expansion on DOS */
#endif

    /* OPEN cirfile.DAT (according to c->Cirfile_mode) */

    /* Make *.DAT filename */
    if (file_number <= 0) {
	strcpy (extension, ".dat");
    }
    else {
	sprintf (extension, "%i.dat", file_number);
    }
    circfint_GetFilename (c, extension,fullfilename);
    if (strlen(fullfilename)==0) return (-1);
	/*printf ("Opening %s\n",fullfilename);*/

      /* Open *.DAT file */
      
      if (c->Cirfile_mode==CREATE) {
	  /* Delete the old file - So if the new create fails, users wont */
	  /* accidently run with the old file. */
	  unlink (fullfilename);

	  /* Create a new file, 0 length, with appropriate read-write access.*/
	  s_mask = circfint_Get_s_mask ();
	  mode2 |= O_RDWR|O_CREAT|O_TRUNC;
	  c->Dat_handle = open (fullfilename, mode2, s_mask);

	  /* Allocate all of the disk space, and preset it to zeros.*/
	  if (c->Dat_handle > -1) {
	      if (c->Multiple_files) {
		 maxrec = circfint_qMaxInFile (c,file_number);
	      }
	      else {
		 maxrec = c->Max_records;
	      }
	      reply = circfint_PreZeroRecords (c, 0, maxrec);
	      if (reply < 0) {
                if (c->Save_messages) TRACE_SAVE_NEXT (1);
	        TRACE_CHARS (-1,"CIR007F CANNOT PREZERO: ",c->Prefix);
	        return (-1);
	      }
          }
	  /* Assume that the just created file is about to be closed.*/
	  /* (Hence dont bother about sync flags etc.) */
      }
      
      else {
	  /* Open an existing file. */
	  /* Even if the user says the file is write-only, */
	  /*  allow reads too, so that we can do debugging reads. */
	  if ( (c->Cirfile_mode==WRITER) || (c->Cirfile_mode==WRITEONLY) ) {
	  	mode2 |= O_RDWR;
	  }
	  else {
	  	mode2 |= O_RDONLY;
	  }
#ifndef TurboC
	  /* On UNIX, maybe make writes go straight to disk.*/
	  /* (avoids losing data in a crash when cache is lost).*/
	  /* Recommended for all but high-volume log files.*/
	  if (c->SyncWrites) mode2 |= O_SYNC;
#endif
	  c->Dat_handle = open (fullfilename,mode2);

	  if (c->Dat_handle < 0) {
		if (c->Save_messages) TRACE_SAVE_NEXT (1);
		TRACE_ERRNO_CHARS (errno,"CIR007F CANNOT OPEN DAT FILE: ",c->Prefix);
		return (-1);
          }
      }
   /*printf ("opened DAT file = %i\n", file_number);*/
   return (0);
}

/**
*---------------------------------------------------------------------------
*							    	 Get_s_mask
*---------------------------------------------------------------------------
* Return a file-permissions-mask for use when creating new files.
*/
int circfint_Get_s_mask (void)
{
   int    s_mask;

   /* When Creating new files, */
   /* give them read-write access to owner and group, and read to Others. */
   /* (I'm not sure about this, but also check login mask defaults too.) */
#ifdef TurboC
   s_mask = S_IREAD|S_IWRITE;
#else
   s_mask = S_IRUSR|S_IWUSR | S_IRGRP|S_IWGRP | S_IROTH;
#endif
    return (s_mask);
}

/**
*---------------------------------------------------------------------------
*							    	 GetFilename
*---------------------------------------------------------------------------
* Return a full path filename of a circular file with desired extension name.
*/
static char *circfint_GetFilename (C c, const char *extension, char *fullfilename)
{

    fullfilename[0] = '\0';
    if ( strlen(c->Home_dir)+1+strlen(c->Prefix)+strlen(extension)
	 		>= MAX_FULL_FN) {
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_CHARS (-1,"CIR007F FILENAME TOO LONG FOR: ",c->Prefix);
	 return (fullfilename);	/* null */
    }

    if (strlen(c->Home_dir)==0) c->Home_dir = ".";
    strncpy (fullfilename, c->Home_dir, CIRCFILE_MAX_HOMEDIR_NAME);
    fullfilename[CIRCFILE_MAX_HOMEDIR_NAME] = '\0';

    strcat (fullfilename, "/");
    strcat (fullfilename, c->Prefix);

    strcat (fullfilename, extension);
    return (fullfilename);
}

/**
*----------------------------------------------------------------------------
*                                                                qMaxInFile
*----------------------------------------------------------------------------
* Given a file-number (0,1,2...), compute the maximum-relative-record-number
* within the given file.
*/
int circfint_qMaxInFile (C c, const int file_no) 
{
    int maxrec, max_in_file;

    maxrec = c->Max_records;
    if (c->Multiple_files) {
	max_in_file = maxrec - (file_no * c->Records_per_file);
	if (max_in_file > c->Records_per_file) {
	    max_in_file = c->Records_per_file;
	}
    }
    else {
	max_in_file = maxrec;
    }
    /* printf ("Fileno=%i, Maxrec=%i, Max_in_file=%i\n", file_no, maxrec, max_in_file);*/
    return (max_in_file);
}

/**
*----------------------------------------------------------------------------
*							         PreZeroFiles
*----------------------------------------------------------------------------
* In the case of a multi-dat-file being expanded,
* pre-zero fill out the remainder of the current DAT file,
* then create extra DAT files (which will do an auto-pre-zero-fill).
*
* Returns:  0 = success, new zero-records written.
*	   -1 = error, cannot write to file, or other error.
*/
int circfint_PreZeroFiles (C c, const int oldmax) 
{
    int next_file_number, old_max_in_file, new_max_in_file, newmax;
    int max_file_number, ifile, reply;
    enum CIRCFILE_Mode old_mode;
    long offset;

    /* Maybe Fill up the remainder of this file.*/
    /* Start with the next record number (oldmax) */
    /* after the previous top most record number (oldmax-1) */
    next_file_number = oldmax / c->Records_per_file;   /* 0,1,2,3,...*/
    old_max_in_file  = oldmax % c->Records_per_file;   /* 0,1,2,3,...,oldmax-1*/

    if (old_max_in_file != 0) {
	/* Then there are still some records to preset on this file.*/

	/* Open the correct file.*/
	offset = circfint_FileOffset (c, oldmax-1);
	if (offset < 0) { return (-1); }

	/* Fill remainder of this file.*/
	/*printf ("Prezero file: file_number=%i, oldmax=%i, newmax=%i \n", 
	**	next_file_number, old_max_in_file, c->Records_per_file);
	*/
	new_max_in_file = circfint_qMaxInFile (c, next_file_number);
	reply = circfint_PreZeroRecords (c, old_max_in_file, new_max_in_file);
	if (reply)  { return (reply);}
	next_file_number++;	
    }

    /* Maybe create some extra new files as well.*/
    newmax = c->Max_records;
    max_file_number = (newmax-1)/c->Records_per_file;
    if (next_file_number <= max_file_number) {
	/* Close the old DAT file.*/
        if (c->Dat_handle>=0) close (c->Dat_handle);
        c->Dat_handle = -1;

	/* Create several new data files on the end.*/
	/* The files will be automatically pre-zero-filled by OpenDatFile.*/
	old_mode = c->Cirfile_mode;
	c->Cirfile_mode=CREATE;
	for (ifile=next_file_number; ifile<=max_file_number; ifile++) {
	    /* printf ("Prezero wholefile: file_number=%i \n", ifile);*/
            reply = circfint_OpenDatFile (c, ifile);
            if (reply) return (reply);
            if (c->Dat_handle>=0) close (c->Dat_handle);
            c->Dat_handle = -1;
	}
	c->Cirfile_mode=old_mode;
    }
    return (0);
}

/**
*----------------------------------------------------------------------------
*							       PreZeroRecords
*----------------------------------------------------------------------------
* Preallocate-zero-fill the data area in a single *.DAT circular file.
* (For multiple files, call this routine once per file.)
* The current DAT file is assumed to be open and ready to be written.
* oldmaxno and newmaxno are relative record numbers within this file.
*
* This is called from circfint_OpenDatFile and from circfile_ResetMaxRecords.
*
* Make circfile.dat into its full size by populating it with zeros.
* On HPs there is a system routine 'prealloc' to do this for initial files only
* For all other systems and cases, we just write our own zeros.
*
* Returns:  0 = success, new zero-records written.
*	   -1 = error, cannot write to file, or other error.
*/
int circfint_PreZeroRecords (C c, const int oldmaxno, const int newmaxno)
{
#define PREALLOC_BUFFER_SIZE 10000
    char buffer[PREALLOC_BUFFER_SIZE+1];
    long nbytes, remaining_bytes, lret_code;
    int ret_code, reply;
#ifdef TurboC
   long   offset;
#else
   int    offset;
#endif
    /*printf ("PreZero: oldmaxno=%i, newmaxno=%i, handle=%i\n", 
    		oldmaxno, newmaxno, c->Dat_handle);*/
    if (oldmaxno >= newmaxno) return (-1);
    if (newmaxno < 1) return (-1);
    if (c->Dat_handle < 0) return (-1);

#ifdef HPUX
    /* On HP systems only, and for whole file presets only, */
    /* use HPs 'prealloc' to get fast disk access. */
    if (oldmaxno == 0) {
	/* On HP only, use system routine.*/
	reply = prealloc (c->Dat_handle, c->Record_size*newmaxno);
	if (reply < 0) {
            if (c->Save_messages) TRACE_SAVE_NEXT (1);
	    TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT PREALLOC: ",c->Prefix,".dat");
	    return (-1);
        }
    }
#endif

    /* For all other cases, we just write binary zeros to fill out the */
    /* desired amount of disk space.*/

    /* Maybe position the file.*/
    if (oldmaxno != 0) {
	offset = oldmaxno * c->Record_size;
	/* SET THE FILE POINTER TO THE APPROPRIATE BYTE ON cirfile.DAT */
	ret_code = lseek (c->Dat_handle,offset,SEEK_SET);
	if (ret_code < 0) {
           if (c->Save_messages) TRACE_SAVE_NEXT (1);
           TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT SEEK: ",c->Prefix,".dat");
           return (-1);
	}
    }

    /* Zeros ready to write.*/
    remaining_bytes = (newmaxno-oldmaxno) * c->Record_size;
    memset (buffer, 0, PREALLOC_BUFFER_SIZE);

    /* Now write the zeros. */
    for (;;) {
	if (remaining_bytes <= 0) break;
	nbytes = PREALLOC_BUFFER_SIZE;
	if (nbytes > remaining_bytes)  nbytes = remaining_bytes;
	errno=0;
	lret_code = write (c->Dat_handle, buffer, nbytes);
	if (lret_code < 0) {
           if (c->Save_messages) TRACE_SAVE_NEXT (1);
           TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT ZERO-WRITE: ",c->Prefix,".dat");
           return (-1);
	}
	remaining_bytes = remaining_bytes - nbytes;
    }
    return (0);
}

/**
*----------------------------------------------------------------------------
*							         ValidatePtrs
*----------------------------------------------------------------------------
*
*	This function checks that all of the pointer values look sensible.
*
*	Returns  0  if all pointers are valid.
*	If any errors occur, a trace_msg is saved and an error code is returned.
*/
int circfint_ValidatePtrs (C c)  {

    float bytes_per_file;

    if ( (c->Max_records<2) || (c->Max_records>MAX_NUM_RECORDS) ) {
        if (c->Save_messages)  TRACE_SAVE_NEXT (1);
        TRACE_CHARS (c->Max_records,"CIR006F BAD MaxRecords ON: ",c->Prefix);
        return (-1);
    }
    if ( (c->Record_size<2) || (c->Record_size>MAX_RECORD_SIZE)   ) {
        if (c->Save_messages) TRACE_SAVE_NEXT (1);
        TRACE_CHARS (c->Record_size,"CIR006F BAD RecordSize ON: ",c->Prefix);
        return (-1);
    }
    if ( (c->Ntbw<0) || (c->Ntbw>=c->Max_records) ) {
        if (c->Save_messages) TRACE_SAVE_NEXT (1);
        TRACE_CHARS (c->Ntbw,"CIR006F BAD Ntbw ON: ",c->Prefix);
        return (-1);
    }
    if ( (c->Ntbr<0) || (c->Ntbr>=c->Max_records) ) {
        if (c->Save_messages) TRACE_SAVE_NEXT (1);
        TRACE_CHARS (c->Ntbr,"CIR006F BAD Ntbr ON: ",c->Prefix);
        return (-1);
    }
    if (c->Records_per_file < 0) {
        if (c->Save_messages) TRACE_SAVE_NEXT (1);
        TRACE_CHARS (c->Ntbr,"CIR006F BAD Records_per_file ON: ",c->Prefix);
        return (-1);
    }

    /* 2Gb per-file-size limit (32 bit addressing). */
    if (c->Records_per_file == 0) {
	/* Whole single file.*/
	bytes_per_file = (float) c->Max_records * c->Record_size;
    }
    else {   /* per individual data file in a multi-file set. */
	bytes_per_file = (float) c->Records_per_file * c->Record_size;
    }
    if (bytes_per_file > 2.E9) {
        if (c->Save_messages) TRACE_SAVE_NEXT (1);
        TRACE_CHARS (c->Ntbw,"CIR006F BAD 2GB FILE LIMIT ON: ",c->Prefix);
        return (-1);
    }

    return (OK);
}

/**
*----------------------------------------------------------------------------
*							             ReadPtrs
*----------------------------------------------------------------------------
*	This function reads all of the pointers from the *.PTR file.
*
*	It 'seek's the beginning of the file and then reads a block of
*	pointers.
*	If any errors occur, a trace_msg is saved and an error code is returned.
*/
int circfint_ReadPtrs (C c)
{
   int    ret_code;
   long	  seek_code;
   int    ptrbuf[6];

   /* SET THE cirfile.PTR SEEK POINTER TO THE START OF THE FILE */
   seek_code = lseek (c->Ptr_handle,0,SEEK_SET);
   if (seek_code < 0) {
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT SEEK: ",c->Prefix,".ptr");
	 return (-1);
   }

   /* READ THE cirfile.PTR DATASET TO OBTAIN THE cirfile.DAT pointers */
   /* Expect 4 numbers (for single-data-file circfiles), */
   /*     or 6 numbers (for multi-data-file circfiles). */
   ret_code = read (c->Ptr_handle, (char *)ptrbuf, sizeof(ptrbuf));
   if ((ret_code != 4*sizeof(ptrbuf[0])) && (ret_code != 6*sizeof(ptrbuf[0]))) {
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT READ: ",c->Prefix,".ptr");
	 return (-1);
   }
   
   /* For single-data-file circfiles.*/
   c->Ntbw = ptrbuf[0];
   c->Ntbr = ptrbuf[1];
   c->Max_records = ptrbuf[2];
   c->Record_size = ptrbuf[3];
   c->Records_per_file = 0;
   
   /* For multi-data-file circfiles.*/
   /* The original Max_records = ptrbuf[2] is set to zero */
   /* (so old code will crash) */
   /* and 2 new numbers (#5,#6) are on the end.*/
   if (ret_code == 6 * sizeof(ptrbuf[0])) { 
	if (ptrbuf[2] != 0) {
          if (c->Save_messages) TRACE_SAVE_NEXT (1);
	  TRACE_CHARS (-1, "CIR007F BAD POINTER IN: ",c->Prefix);
	  return (-1);
	}
	c->Records_per_file = ptrbuf[4];
	c->Max_records = ptrbuf[5];
	c->Multiple_files = 1;		/* True.*/
   }
      
   ret_code = circfint_ValidatePtrs(c);
   return (ret_code);
}

/**
*----------------------------------------------------------------------------
*							            WritePtrs
*----------------------------------------------------------------------------
* This function writes 1 or all of the pointers to the *.PTR file.
*
* It 'seek's the position of the pointer in the file,
* and then writes 1 or all pointers.
* If any errors occur, a trace_msg is saved and an error code is returned.
*/
int circfint_WritePtrs (C c, int which_ptrs) {

   int    ret_code, offset, index, length;
   long	  seek_code;
   int    ptrbuf[6];

   /* Pre-validate */
   ret_code = circfint_ValidatePtrs(c);
   if (ret_code)  return (ret_code);

   /* Position the PTR file on the desired pointer.*/
   switch (which_ptrs) {

     case READ_PTR:
	index = 1;
	offset = sizeof (ptrbuf[0]) * index;
	length = sizeof (ptrbuf[0]);
	break;

     case WRITE_PTR:
	index = 0;
	offset = sizeof (ptrbuf[0]) * index;
	length = sizeof (ptrbuf[0]);
	break;

     case OTHER_PTRS:
        /* Max_records, Record_size, and maybe multi-file records_per_file.*/
        /* 3rd slot onwards.*/
	index = 2;
	offset = sizeof (ptrbuf[0]) * index;
	
	if (c->Multiple_files == 0) {
	    /* For single-files, do 3rd,4th slots. */
	    length = sizeof (ptrbuf[0]) * 2;
	    break;
	}
	else {
	    /* For multiple-files, do 3rd,4th,5th,6th slots. */
	    length = sizeof (ptrbuf[0]) * 4;
	    break;
	}

     case ALL_PTRS:
	offset = 0; index = 0;
	if (c->Multiple_files == 0) {length = sizeof (ptrbuf[0]) * 4; }
	else                        {length = sizeof (ptrbuf[0]) * 6; }
	break;

     default:
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_CHARS (which_ptrs,"CIR00?F INTERNAL ERROR: ",c->Prefix);
	 return (-1);
   }
   seek_code = lseek (c->Ptr_handle, offset, SEEK_SET);
   if (seek_code < 0) {
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT SEEK: ",c->Prefix,".ptr");
	 return (-1);
   }

   /* WRITE THE cirfile.PTR DATASET TO SAVE THE cirfile.DAT pointers */
   
   /* For single-data-file circfiles.*/  
   ptrbuf[0] = c->Ntbw;
   ptrbuf[1] = c->Ntbr;
   ptrbuf[2] = c->Max_records;
   ptrbuf[3] = c->Record_size;

   /* For multi-data-file circfiles.*/
   /* The original Max_records = ptrbuf[2] is set to zero */
   /* (so old code will crash) */
   /* and 2 new numbers (slots 5,6) are on the end.*/
   if (c->Multiple_files) {
	ptrbuf[2] = 0;
	ptrbuf[4] = c->Records_per_file;
	ptrbuf[5] = c->Max_records;
   }
   ret_code = write (c->Ptr_handle, (char *)&ptrbuf[index], length);
   if (ret_code < 0) {
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT WRITE: ",c->Prefix,".ptr");
	 return (-1);
   }
   return (OK);
}

/**
*----------------------------------------------------------------------------
*							           FileOffset
*----------------------------------------------------------------------------
* Given a record number within a multi-DAT-file system,
* open the appropriate DAT-file, and return the offset within this file
* of where the given record starts.
*	record_number = 0...max-1
*	file_number = 0...n
* Returns:  offset = 0...nn  success (appropriate DAT-file is open)
*		   = -1 Failed. (Cannot open file ?, logmsg has been printed.)
*/
long circfint_FileOffset (C c, const int record_number)
{
    int file_number, reply;
    long offset;

    /* For single files...*/
    if (c->Multiple_files == 0) {
	/* Single file */
	offset = (long) record_number * c->Record_size;
	return (offset);
    }

    /* For multiple files...*/
    
    file_number = record_number / c->Records_per_file;

    /* Maybe Close the old DAT file, then open the appropriate new one.*/

    if (file_number != c->Current_file_number) {
	if (c->Dat_handle>=0) close (c->Dat_handle);
	c->Dat_handle = -1;
	c->Current_file_number = file_number;
	
	reply = circfint_OpenDatFile (c, file_number);
	if (reply) return (-1);  /* errors */
    }
    
    /* Now calculate the byte offset within this file.*/
    offset = record_number%c->Records_per_file * c->Record_size;

    /*printf ("(record_no=%i, file_no=%i, offset=%li)\n",
    **		record_number,file_number,offset);
    */
    return (offset);
}
/**
*----------------------------------------------------------------------------
*							            ReadData
*----------------------------------------------------------------------------
*    This function reads a data record from a given position in the
*    DAT file.  (the pointers are not touched)
*
*    It determines the byte offset into the DAT file for the data to be
*    written, and does a seek to set the file pointer to the correct
*    position.  The data is then read.
*    If any errors occur then a error code is returned to the
*    caller (with a saved trace_msg).
*/
int circfint_ReadData (C c, void *data_record, const int record_number)
{
   int    ret_code;
#ifdef TurboC
   long	  seek_code;
   long   offset;
#else
   off_t  seek_code;
   int    offset;
#endif

   /* Get Byte offset into DAT file */
   offset = circfint_FileOffset (c, record_number);
   if (offset < 0)  return (-1);  

   /* SET THE FILE POINTER TO THE APPROPRIATE BYTE ON cirfile.DAT */
   seek_code = lseek (c->Dat_handle,offset,SEEK_SET);
   if (seek_code < 0) {
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT SEEK: ",c->Prefix,".dat");
	 return (-1);
   }

   /* READ THE DATA FROM cirfile.DAT */
   ret_code = read (c->Dat_handle, (char *)data_record, c->Record_size);
   if (ret_code < 0) {
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT READ: ",c->Prefix,".dat");
	 return (-1);
   }
   c->Lrnr = record_number;  /* remember last record number read. */
   return (OK);
}


/**
*----------------------------------------------------------------------------
*							            WriteData
*----------------------------------------------------------------------------
*    This function writes the given data record the given position in the
*    DAT file.  (The PTR file is fixed elsewhere)
*    Optionally store the record number within the user's data before
*	doing the write.
*
*    It determines the byte offset into the DAT file for the data to be
*    written, and does a seek to set the file pointer to the correct
*    position.  The data is written.
*    If errors occur then a error code is returned to the
*    caller (with a saved trace_msg).
*/
int circfint_WriteData (C c, void *data_record, const int record_number) {

   int    ret_code;
#ifdef TurboC
   long	  seek_code;
   long   offset;
#else
   off_t  seek_code;
   int    offset;
#endif

   /* Get Byte offset into DAT file */
   offset = circfint_FileOffset (c, record_number);
   if (offset < 0)  return (-1);  

   /* Maybe store the record number within the user's data first. */
   if (c->Store_recno) circfint_StoreRecno (c, (int *)data_record,record_number);

   /* SET THE FILE POINTER TO THE NEXT AVAILABLE BYTE ON cirfile.DAT */
   seek_code = lseek (c->Dat_handle,offset,SEEK_SET);
   if (seek_code < 0) {
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT SEEK: ",c->Prefix,".dat");
	 return (-1);
   }

   /* WRITE THE DATA TO cirfile.DAT */
   ret_code = write (c->Dat_handle, (char *)data_record, c->Record_size);
   if (ret_code < 0) {
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT WRITE: ",c->Prefix,".dat");
	 return (-1);
   }
   c->Lrnw = c->Ntbw;  /* remember last record number written. */
   return (OK);
}

/**
* --------------------------------------------------------------------------
*					(internal)	StoreRecno
* ---------------------------------------------------------------------------
* Store the record number within the user's data just before it is written.
*/
int circfint_StoreRecno (C c, int *data_record, const int record_number) {
   memcpy (&data_record[c->Recno_index], &record_number, sizeof(int));
   return (OK);
}

/**
* --------------------------------------------------------------------------
*					(internal)	ReadVarLenData
* ---------------------------------------------------------------------------
* Read a variable-length record from a circular file,
* and update the ntbr pointer.
* (Variable length records are held on file as a many fixed-length records.)
* Returns: 0=OK, -1 = error (and a saved Trace message is stored),
*/
int circfint_ReadVarLenData (C c, void *data_record, const int vmaxlength,
						const int record_number) {
    int vrecord_length, part_record_number, reply;
    int file_part_no, file_vrec_len, len, len_left, recno;
    char *part_record, *vchar_record;

    /* Make sure that the fixed-length records are big enough for us.*/
    if (c->Record_size < 20) {
       if (c->Save_messages) TRACE_SAVE_NEXT (1);
       TRACE_CHARS (c->Record_size,"CIR00?F TOO SHORT FOR VARLENGTH ON: ",c->Prefix);
       return (-1);
    }

    /* Temporary storage to hold 1 fixed length record read from the file.*/
    part_record = (char *) malloc (c->Record_size);
    if (part_record == NULL) {
       if (c->Save_messages) TRACE_SAVE_NEXT (1);
       TRACE_CHARS (-1,"CIR00?F NO MEMORY LEFT ON: ",c->Prefix);
       return (-1);
    }

    /* pointer to access chars within data record.*/
    vchar_record = (char *) data_record;

    part_record_number = 0;
    vrecord_length = 0;

    /*  DO NOT RETURN WITHOUT FREE-ING MALLOC MEMORY IN "part_record" ! */

    /* Read in the fixed-length records one at a time, */
    /* check the details, then copy the raw data to the users array.*/
    for (;;) {
	recno = circfile_qCorrectN (c, record_number+part_record_number);
	reply = circfint_ReadData (c, part_record, recno);
	if (reply) { reply = -1; break; };

	/* In front of each fixed length record there is:  "rr lllll " */
	/* rr = record number within sequence 00-99, */
	/* lllll = variable record length of whole var-len-record, 0-99999.*/
	/* Check the format of the record, then extract vital numbers.*/
	/* Use reply=-2 to flag bad format.*/
	if ((part_record[2] != ' ') || (part_record[8] != ' ')) {
	    reply = -2;  break;
	}
	part_record[2] = '\0';
	part_record[8] = '\0';
        file_part_no = atoi (part_record);
	file_vrec_len = atoi (&part_record[3]);
	if (file_part_no != part_record_number) { reply=-2; break; }
	if (part_record_number == 0) {
	    /* 1st record in a sequence.*/
	    vrecord_length = file_vrec_len;
	    if (vrecord_length > vmaxlength) {
	       if (c->Save_messages) TRACE_SAVE_NEXT (1);
	       TRACE_CHARS (vrecord_length,"CIR00?F VARLENGTH RECORD TOO LONG ON: ",c->Prefix);
	       reply=-1;
	       break;
	    }
	    len_left = vrecord_length;
	}
	else {
	    if (file_vrec_len != vrecord_length) { reply=-2; break; }
	}

	/* The details are correct, now copy the raw data.*/
	len = c->Record_size - 9;
	if (len_left < len) len = len_left;
	memcpy (vchar_record, &part_record[9], len);
	len_left -= len;
	vchar_record += len;
	part_record_number ++;

	if (len_left <= 0)  { reply = OK; break; }
    }
    free (part_record);

    /* Finished reading data. Maybe trace errors. */
    if (reply == -2) {
       if (c->Save_messages) TRACE_SAVE_NEXT (1);
       TRACE_CHARS (-1,"CIR00?F NOT VARLENGTH RECORD ON: ",c->Prefix);
       return (-1);
    }
    if (reply) return (reply);
    return (vrecord_length);
}

/**
* --------------------------------------------------------------------------
*					(internal)	WriteVarLenData
* ---------------------------------------------------------------------------
* Write a new variable-length record to a circular file,
* and update the ntbw pointer.
* (Variable length records are held on file as a many fixed-length records.)
* Returns: 0=OK, -1 = error (and a saved Trace message is stored),
*                -2 = error, file full   (no trace message).
*/
int circfint_WriteVarLenData (C c, void *data_record, const int vrecord_length) {

    int part_record_number, reply;
    int len, len_left, maxlen;
    char *part_record, *vchar_record;

    /* Make sure that the fixed-length reords are big enough for us.*/
    if (c->Record_size < 20) {
       if (c->Save_messages) TRACE_SAVE_NEXT (1);
       TRACE_CHARS (c->Record_size,"CIR00?F TOO SHORT FOR VARLENGTH ON: ",c->Prefix);
       return (-1);
    }

    /* Temporary storage to hold 1 fixed length record */
    /* to be written the file.*/
    part_record = (char *) malloc (c->Record_size);
    if (part_record == NULL) {
       if (c->Save_messages) TRACE_SAVE_NEXT (1);
       TRACE_CHARS (-1,"CIR00?F NO MEMORY LEFT ON: ",c->Prefix);
       return (-1);
    }

    /* pointer to access chars within data record.*/
    vchar_record = (char *) data_record;

    /*  DO NOT RETURN WITHOUT FREE-ING MALLOC MEMORY IN "part_record" ! */

    /* Compose fixed-length records one at a time, */
    /* by adding the details & copying the raw data from the users array, */
    /* then write them to the file.*/
    part_record_number = 0;
    len_left = vrecord_length;
    for (;;) {

	/* In front of each fixed length record there is:  "rr lllll " */
	/* rr = record number within sequence 00-99, */
	/* lllll = variable record length of whole var-len-record, 0-99999.*/
	sprintf (part_record, "%02i %05i ", part_record_number, vrecord_length);

	/* The details are correct, now copy the raw data.*/
	maxlen = c->Record_size - 9;
	len = maxlen;
	if (len_left < len) len = len_left;
	memcpy (&part_record[9], vchar_record, len);
	/* For sanity, blank fill the last record.*/
	if (len < maxlen) {memset (&part_record[9+len], ' ', maxlen-len);}

	reply = circfint_WriteData (c, part_record, c->Ntbw);
	if (reply) { reply = -1; break; };

	/* Move the next-to-be-written pointer to the next record. */
	c->Ntbw++;
	if (c->Ntbw >= c->Max_records)  c->Ntbw = 0;

	part_record_number++;
	len_left -= len;
	vchar_record += len;

	if (len_left <= 0)  { reply = OK; break; }
    }
    free (part_record);

    /* Finished writing data. Maybe trace errors. */
    if (reply == -2) {
       if (c->Save_messages) TRACE_SAVE_NEXT (1);
       TRACE_CHARS (-2,"CIR00?F NOT VARLENGTH RECORD ON: ",c->Prefix);
       return (-2);
    }
    if (reply) return (reply);
    return (OK);
}

/**
* ---------------------------------------------------------------------------
*					(internal)	qNfixedLenRecords
* ---------------------------------------------------------------------------
* Query the number of fixed-length records required to store this
* variable-length record.
* (Variable length records are held on file as a many fixed-length records.)
* Returns: n = nrecords,
*	  -1 = error (and a saved Trace message is stored),
*/
int circfint_qNfixedLenRecords (const int varreclength,
				const int fixedreclength) {

    int overhead_per_record, nrecords, nleft;

    /* In front of each fixed length record there is:  "rr lllll " */
    /* rr = record number within sequence 00-99, */
    /* lllll = variable record length of whole variable-len-record, 0-99999.*/
    overhead_per_record = 2 + 1 + 5 + 1;
    nleft = fixedreclength - overhead_per_record;
    nrecords = varreclength / nleft;
    if ((nrecords*nleft) != varreclength)  nrecords++;
    return (nrecords);
}

/**
* ---------------------------------------------------------------------------
*								Lock
* ---------------------------------------------------------------------------
* Locks either the read_pointer or the write_pointer in the cirfile.PTR file.
* (Locks the 1st byte of the appropriate pointer.)
* If the lock fails then a trace_msg is saved
* and an error is returned to the caller.
*/
int circfint_Lock (C c, int which_ptr) {
   int    ret_code, offset;
   long   seek_code;

   /* Position the PTR file on the desired pointer.*/
   switch (which_ptr) {
     case READ_PTR:	offset = sizeof (int); break;
     case WRITE_PTR:	offset = 0; break;
     default:
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_CHARS (which_ptr,"CIR00?F INTERNAL ERROR: ",c->Prefix);
	 return (-1);
   }

   seek_code = lseek (c->Ptr_handle, offset ,SEEK_SET);
   if (seek_code < 0) {
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT SEEK: ",c->Prefix,".ptr");
	 return (-1);
   }

   /* LOCK 1 byte OF THE FILE cirfile.PTR */
#ifndef TurboC
   ret_code = lockf (c->Ptr_handle,F_LOCK,1);
#else
   ret_code = 0;  /* Dummy for DOS testing.*/
#endif
   if (ret_code != 0) {
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT LOCK: ",c->Prefix,".ptr");
	 return (-1);
   }
   return (OK);
}

/**
* ---------------------------------------------------------------------------
*								Unlock
* ---------------------------------------------------------------------------
* Unlocks either the read_pointer or the write_pointer in the cirfile.PTR file.
* (Unlocks the 1st byte of the appropriate pointer.)
* If the unlock fails then a trace_msg is saved
* and an error is returned to the caller.
*/
int circfint_Unlock (C c, int which_ptr) {
   int    ret_code, offset;
   long   seek_code;

   /* Position the PTR file on the desired pointer.*/
   switch (which_ptr) {
     case READ_PTR:	offset = sizeof (int); break;
     case WRITE_PTR:	offset = 0; break;
     default:
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_CHARS (which_ptr,"CIR00?F INTERNAL ERROR: ",c->Prefix);
	 return (-1);
   }

   seek_code = lseek (c->Ptr_handle, offset, SEEK_SET);
   /* N.B. IF THIS FAILS THEN THE FILE IS LEFT LOCKED !!!!!! */
   if (seek_code < 0) {
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT SEEK: ",c->Prefix,".ptr");
	 return (-1);
   }

   /* UNLOCK 1 byte OF THE FILE cirfile.PTR */
#ifndef TurboC
   ret_code = lockf (c->Ptr_handle,F_ULOCK,1);
#else
   ret_code = 0;  /* Dummy for DOS testing.*/
#endif
   if (ret_code != 0) {
         if (c->Save_messages) TRACE_SAVE_NEXT (1);
	 TRACE_ERRNO_CHARS2 (errno,"CIR007F CANNOT UNLOCK: ",c->Prefix,".ptr");
	 return (-1);
   }
   return (OK);
}


