/*
 * PStorage.h
 *
 *  Created on: 06.06.2018
 *      Author: Dr. Martin Schaaf
 *
 * Sizing of the PStorage. The exact amount of entries cannot be determined in advance since
 * entries only allocate their real size, which is expected to be much smaller than PSTORAGE_ENTRY_MAXSIZE.
 * However, index entries have a fixed size, which primarily depends on ENTRY_NAME_MAXSIZE. Hence, an
 * upper bound estimation of entries is given by PSTORAGE_ENTRY_UPPERBOUND.
 * Example assuming each entry having only 1 Byte value/payload:
 * SPIFFS = 128 kBytes, ALLOC = 50% -> FILES_MAXSIZE = 64 kBytes -> ca. 485 entries with NAME_MAXSIZE = 128 Bytes
 *
 */

#ifndef PSTORAGE_H_
#define PSTORAGE_H_

#include <Arduino.h>
#include <FS.h>

#define PSTORAGE_DEBUG_ENABLED 			true

#define PSTORAGE_MAX_ENTRIES			40 			// max number of entries
#define PSTORAGE_VALUEFILE_MAXSIZE		(40*1024)	// byte being consumed by the value file. Should be sufficient for
													// MAX_ENTRIES

#define PSTORAGE_ENTRY_NAME_MAXSIZE		20			// Max size of an entry name. A change may invalidate all existing PStorages
													// be careful (!!!)

#if(PSTORAGE_DEBUG_ENABLED)
#define PSTORAGE_DEBUG(...) _pStoragedebug(__VA_ARGS__)
#else
#define PSTORAGE_DEBUG(...)
#endif

#define P_INT 0
#define P_UINT 1
#define P_LONG 2
#define P_ULONG 3
#define P_FLOAT 4
#define P_ARRAY 5
#define P_STRING 6

struct PStorageIndexEntry {
	char name[PSTORAGE_ENTRY_NAME_MAXSIZE  + 1];  // one more for the \0
	unsigned int maxValueSize;  // determines the bytes being reserved
	unsigned int actualValueSize;
	unsigned int valuePosition; // position index into value .dat-file
	byte type;
};

/*
 * PStorageCtrlParams are written at the beginning of the index file and ensure proper alignment with PStorage
 * creation (!) parameter (in case they are changed throughout compilations)
 */
struct PStorageCtrlParams {
	unsigned int maxEntries;
	unsigned int valueFileMaxSize;  // the max SPIFF consumption of the value file
	unsigned int entryNameMaxSize; // used to check consistence with current
	unsigned int actualEntries;
	unsigned int actualValueFileSize;
};

void _pStoragedebug(const char *format, ...);

class PStorage {
public:
	PStorage();
	virtual ~PStorage();

	boolean create();
	boolean resume();
	boolean end();

	boolean map(const char *name, int value);
	boolean map(const char *name, unsigned int value);
	boolean map(const char *name, long value);
	boolean map(const char *name, unsigned long value);
	boolean map(const char *name, float value);

	boolean get(const char *name, int *value);
	boolean get(const char *name, unsigned int *value);
	boolean get(const char *name, long *value);
	boolean get(const char *name, unsigned long *value);
	boolean get(const char *name, float *value);


	boolean createByteArray(const char *name, unsigned int maxSize);
	boolean writeByteArray(const char *name, byte b[], unsigned int bSize);
	boolean readByteArray(const char *name, byte b[], unsigned int bSize, unsigned int *length);

	boolean createString(const char *name, unsigned int maxLength);
	boolean writeString(const char *name, const char *str);
	boolean readString(const char *name, char *str, unsigned int strSize);

	unsigned int maxSize();

private:
	void _serializeUINT(const unsigned int v, byte buf[], int *index);
	unsigned int _deserializeUNIT(const byte buf[], int *index);

	void PStorage::_ctrlParamsSerialize(const PStorageCtrlParams pcp, byte buf[]);
	void PStorage::_ctrlParamsDeserialize(const byte buf[], PStorageCtrlParams *pcp);
	void _indexEntrySerialize(const PStorageIndexEntry indexEntry, byte buf[]);
	void _indexEntryDeserialize(const byte buf[], PStorageIndexEntry *indexEntry);

	boolean _readIndexEntry(File indexFile, PStorageIndexEntry *indexEntry, const unsigned int position);
	boolean _writeIndexEntry(File indexFile, const PStorageIndexEntry indexEntry, const unsigned int position);
	boolean _searchIndexEntry(const byte type, const char *name, File indexFile, unsigned int *position);

	boolean _createEntry(const char *name, File indexFile, File valueFile, unsigned int maxLengthValue);
	boolean _writeEntry(const char *name, byte* writeBuf, unsigned int length, File indexFile, File valueFile);
	boolean _readEntry(const char *name, File indexFile, File valueFile, byte* buf, unsigned int bufSize, unsigned int *bytesReads);

	const char* _getIndexFileName();
	const char* _getValueFileName();
	boolean PStorage::_getPStorageFiles(File *indexFile, File *valueFile);

	PStorageCtrlParams _pcp;
};

extern PStorage pStorage;

#endif /* PSTORAGE_H_ */
