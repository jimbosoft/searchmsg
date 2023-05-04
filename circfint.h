#ifndef _CIRCFINT_H
#define _CIRCFINT_H
/*$Header: /home/commsa/cmdv/cmss/src/RCS/circfint.h,v 1.7 2014/09/24 00:57:23 ies Exp $
*
* v1.7  2013-11-27 Ian Senior  Remove circfint_GetFilename()
* v1.6  2002-06-14 C.Edington, add qMaxInFile.
* v1.5  2002-05-30 C.Edington, changed MAX_PTR to OTHER_PTRS.
*				Support Multi-files.
* v1.4  001111 C.Edington, add PrezeroRecords.
*/

/* Circular file Internal functions */

/* Commands to Lock/Unlock/WritePtrs: */
#define	READ_PTR  1
#define WRITE_PTR 2
#define OTHER_PTRS 4
#define ALL_PTRS  3

#ifdef __cplusplus
extern "C" {
#endif

int circfint_ValidateFilename (C c, const char *prefix);

int circfint_OpenValidate (C c,const char *prefix, enum CIRCFILE_Mode new_mode);
			/* validate parameters before */
			/* and after opening files.   */
int circfint_OpenFiles (C c);	/* to open both files in the appropriate mode.*/
int circfint_OpenPtrFile (C c); /* to open *.ptr in the appropriate mode.*/
int circfint_OpenDatFile (C c, int file_number); 
				/* to open *.dat in the appropriate mode.*/
int circfint_Get_s_mask (void);

int circfint_qMaxInFile (C c, const int file_no);
int circfint_PreZeroFiles (C c, const int oldmax);
			/* to pre-zero-fill extended files.*/
int circfint_PreZeroRecords (C c, const int oldmax, const int newmax);
			/* to pre-zero-fill new/extended file.*/
int circfint_ValidatePtrs(C c);	/* after reading-file and before writing-file */

int circfint_ReadPtrs(C c);
			/* to read the pointers from the file before updates */
int circfint_WritePtrs(C c, int which_ptrs);
			/* to write the pointers to the file after updates */

long circfint_FileOffset  (C c, const int record_no);
int circfint_ReadData  (C c, void *record, const int record_no);
int circfint_WriteData (C c, void *record, const int record_no);
int circfint_StoreRecno (C c, int *record, const int record_no);

int circfint_qNfixedLenRecords (const int vreclength, const int cirreclength);
int circfint_ReadVarLenData  (C c, void *record, const int vmaxlength,
							const int record_no);
int circfint_WriteVarLenData (C c, void *record, const int vreclength);

int circfint_Lock(C c, int which_ptr);
			/* internal function to lock the file before updates  */
int circfint_Unlock(C c, int which_ptr);
			/* internal function to unlock the file after updates */

#ifdef __cplusplus
}
#endif
#endif  /* _CIRCFINT_H */


