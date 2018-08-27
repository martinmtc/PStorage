/*
 * PStorage.h
 *
 *  Created on: 06.06.2018
 *      Author: Dr. Martin Schaaf
 *
 *
 */

#ifndef PSTORAGE_H_
#define PSTORAGE_H_

#include <Arduino.h>
#include <FS.h>
#include <spiffs/spiffs_config.h>

#define PSTORAGE_MAGIC_COOKIE			26202		// changing this will result in invalidation of all existing PStorages

#define PSTORAGE_DEBUG_ENABLED 			false

#define PSTORAGE_INDEX_NAME_MAXSIZE		5			// Max size of an entry name. A change may invalidate all existing PStorages
// be careful (!!!)
#define PSTORAGE_ENTRY_MINSIZE 4  // increases reuse of entries against fragmentation

enum EntryType {
	P_FREE = 0,
	P_INT = 1,
	P_UINT = 2,
	P_LONG = 3,
	P_ULONG = 4,
	P_FLOAT = 5,
	P_ARRAY = 6,
	P_STRING = 7
} ;

struct PStorageIndexEntry {
	char name[PSTORAGE_INDEX_NAME_MAXSIZE  + 1];  // one more for the \0
	EntryType type;
	unsigned int thisEntry; // file position
	unsigned int previousEntry; // file position
	unsigned int nextEntry;  // file position of next entry
};

/*
 * PStorageCtrlParams are written at the beginning of the index file
 */
struct PStorageParams {
	unsigned int magicCookie;
	unsigned int size;
	unsigned int firstEntry; // file position of the first entry
};

void _pStoragedebug(const char *format, ...);

class PStorage {
public:
	PStorage(const char *name);
	virtual ~PStorage();

	boolean open();
	boolean create(unsigned int maxSize);

	boolean map(const char *name, int value);
	boolean map(const char *name, unsigned int value);
	boolean map(const char *name, long value);
	boolean map(const char *name, unsigned long value);
	boolean map(const char *name, float value);
	boolean map(const char *name, byte b[], unsigned int size);
	boolean map(const char *name, const char *str);


	boolean get(const char *name, int *value);
	boolean get(const char *name, unsigned int *value);
	boolean get(const char *name, long *value);
	boolean get(const char *name, unsigned long *value);
	boolean get(const char *name, float *value);
	boolean get(const char* name, byte buf[], unsigned int bufSize);
	boolean get(const char* name, char* buf, unsigned int bufSize);

	boolean remove(const char *name);

	unsigned int getAllocatedSize();
	unsigned int getPStorageSize();
	void dumpPStorage();

private:
	boolean _readParams();
	boolean _writeParams();

	boolean _allocate(const char *name, unsigned int size, EntryType type, PStorageIndexEntry *ie);
	boolean _free(PStorageIndexEntry *ie);

	boolean _isFirstIndexEntry(PStorageIndexEntry ie);
	boolean _isLastIndexEntry(PStorageIndexEntry ie);
	unsigned int _size(PStorageIndexEntry ie);

	boolean _readFirstIndexEntry(PStorageIndexEntry *ie);
	boolean _readIndexEntry(PStorageIndexEntry *ie);  // from the current file position
	boolean _writeIndexEntry(const PStorageIndexEntry ie);  // to the current file position

	boolean _searchIndexEntry(EntryType type, const char *name, PStorageIndexEntry *ie);
	boolean _searchIndexEntry(const char *name, PStorageIndexEntry *ie);
	boolean _searchFreeIndexEntry(unsigned int minSize, PStorageIndexEntry *ie);

	boolean _writeEntry(const PStorageIndexEntry ie, byte* buf, unsigned int maxBytes);
	int _readEntry(const PStorageIndexEntry ie, byte* buf, unsigned int maxBytes);

	const char* _getStorageFileName();

	String _printType(EntryType type);
	void _printFree();
	void _printInt(PStorageIndexEntry ie);
	void _printUInt(PStorageIndexEntry ie);
	void _printLong(PStorageIndexEntry ie);
	void _printULong(PStorageIndexEntry ie);
	void _printFloat(PStorageIndexEntry ie);
	void _printString(PStorageIndexEntry ie);
	void _printArray(PStorageIndexEntry ie);
	void _printDefault();
	void _printEntry(PStorageIndexEntry ie);

	const char *_name;
	char _storageFileName[SPIFFS_OBJ_NAME_LEN];
	File _storageFile;
	PStorageParams _params;
};

#if(PSTORAGE_DEBUG_ENABLED)
#define PSTORAGE_DEBUG(...) _pStoragedebug(__VA_ARGS__)
#else
#define PSTORAGE_DEBUG(...)
#endif

#endif /* PSTORAGE_H_ */
