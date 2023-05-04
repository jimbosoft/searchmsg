#ifndef _CIRFILE_H
#define _CIRFILE_H

// "$Header: /home/commsa/cmdv/cmss/src/RCS/cirfile.h,v 1.9 2002/06/12 02:15:07 ace Exp $  CMSS,BoM"

//	class CirFile.h -- Circular file toolkit  (definitions).

// v1.9  2002-05-24 ACE, support multi-dat-files.
// v1.8  001116 ACE, spelling change to qNtbwMinus1 (from qNtbwLess1).
// v1.7  001116 ACE, added IncrementMyNtbr, qNtbwLess1.
// v1.6  001031 ACE, added IncrementRecordNo/DecrementRecordNo for lookers.
// v1.5  980513 ACE, added ResetMaxRecords.
// v1.4  941031 ACE, added RewriteRecord.
// v1.3  940127 ACE, make C++ cirfile objects call circfile c-routines,
//		     default to sync-writes, add SetSyncWrites (on or off),
//		     changed ResetNtbrNtbw to ResetNtbr, ResetNtbw.
// v1.2  931028 ACE, added ResetNtbrNtbw.
//
// (History below refers to SUN cirfile.h before transfer to HP)
// .v1.6  930521 ACE, deleted const from writeupdate header.
// .v1.5  930514 ACE, added SetStoreRecordno.
// .v1.4  930302 ACE, added SetDirectory, cHomeDir, AcknowledgeRecord,
//			SetDebugMode, simplified DiscardUnreadRecords.
// .v1.3  921006 ACE, added qLrnr, qLrnw, DiscardUnreadRecords.
// .v1.2  920818 ACE, added SaveTraceMessages
// .v1.1  920814 ACE, first version.

// Class definitions for Circular File toolkit  (for fixed length records).
// See full documentation in source file:  cirfile.cc

// Note: this C++ CIRFILE class makes calls to old-C CIRCFILE subroutines.

#include "circfile.h"

class CirFile {

public:
    CirFile ();			// constructor
    ~CirFile ();		// destructor
    int Close () { return (circfile_Close (&c)); }
    int SetSyncWrites (const int yesno) {
	return (circfile_SetSyncWrites(&c,yesno)); };
    int SetDirectory (const char *homedir) {
	return (circfile_SetDirectory(&c,homedir)); };
    int Create (const char *prefix,const int max_records,const int record_size){
	return (circfile_Create(&c,prefix,max_records,record_size)); };
    int CreateMultiFile (const char *prefix, const int max_records,
    			const int record_size, const int records_per_file) {
	return (circfile_CreateMultiFile(&c,prefix,max_records,record_size,
			records_per_file)); };
    int SaveTraceMessages (const int yesno) {
	return (circfile_SaveTraceMessages(&c,yesno)); };
    int OpenReader (const char *prefix) {
	return (circfile_OpenReader(&c,prefix)); };
    int OpenWriter (const char *prefix) {
	return (circfile_OpenWriter(&c,prefix)); };
    int OpenWriteOnly (const char *prefix) {
	return (circfile_OpenWriteOnly(&c,prefix)); };
    int OpenLooker (const char *prefix) {
	return (circfile_OpenLooker(&c,prefix)); };
    int SetUnlockedReads () {
	return (circfile_SetUnlockedReads(&c)); };
    int SetUnlockedWrites () {
	return (circfile_SetUnlockedWrites(&c)); };
    int SetStoreRecordno (void *record, void *record_recno) {
	return (circfile_SetStoreRecordno(&c,record,record_recno)); };
    int DiscardUnreadRecords (void) {
	return (circfile_DiscardUnreadRecords(&c)); };
    int ResetNtbr (int ntbr) {
	return (circfile_ResetNtbr(&c,ntbr)); };
    int ResetNtbw (int ntbw) {
	return (circfile_ResetNtbw(&c,ntbw)); };
    int ResetMaxRecords (int max_records) {
	return (circfile_ResetMaxRecords(&c,max_records)); };
    int LookRecord (void *record, const int record_no) {
	return (circfile_LookRecord(&c,record,record_no)); };
    int ReadRecord (void *record, const int record_no) {
	return (circfile_LookRecord(&c,record,record_no)); };
    int RewriteRecord (void *record, const int record_no) {
	return (circfile_RewriteRecord(&c,record,record_no)); };
    int ReadUpdate (void *record) {
	return (circfile_ReadUpdate(&c,record)); };
    int WriteUpdate (void *record) {
	return (circfile_WriteUpdate(&c,record)); };
    int AcknowledgeRecord (const int record_no) {
	return (circfile_AcknowledgeRecord(&c,record_no)); };
    int RefreshPointers () { return (circfile_RefreshPointers(&c)); };
    int qLrnr () { return c.Lrnr; };		// inline function
    int qLrnw () { return c.Lrnw; };		// inline function
    int qNtbr () { return c.Ntbr; };		// inline function
    int qNtbw () { return c.Ntbw; };		// inline function
    int qNtbwMinus1 () {
	return (circfile_qNtbwMinus1 (&c)); };
    int qMaxRecords () { return c.Max_records; };// inline function
    int qRecordSize () { return c.Record_size; };// inline function
    int qRecordsPerFile () { return c.Records_per_file; };// inline function
    int CorrectRecordNo (int record_no) {
	return (circfile_CorrectRecordNo(&c,record_no)); };
    int IncrementMyNtbr (int *ntbr) {
	return (circfile_IncrementMyNtbr(&c,ntbr)); };
    int IncrementRecordNo (int *record_no) {
	return (circfile_IncrementRecordNo(&c,record_no)); };
    int DecrementRecordNo (int *record_no) {
	return (circfile_DecrementRecordNo(&c,record_no)); };
    int SetDebugMode (const int i) {
	return (circfile_SetDebugMode(&c,i)); };

protected:

private:
    CIRCFILE c;

};

#endif  /* _CIRFILE_H */


