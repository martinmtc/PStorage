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

#define PSTORAGE_DEBUG_ENABLED 		false
#define PSTORAGE_SPIFFS_ALLOCFACTOR 50					// % percentage PStorage shall use of SPIFFS
#define PSTORAGE_ENTRY_VALUE_MAXSIZE		512			// bytes per entry
#define PSTORAGE_ENTRY_NAME_MAXSIZE	20					// bytes per index entry name

#if(PSTORAGE_DEBUG_ENABLED)
#define PSTORAGE_DEBUG(...) _pStoragedebug(__VA_ARGS__)
#else
#define PSTORAGE_DEBUG(...)
#endif


struct PStorageIndexEntry {
  char name[PSTORAGE_ENTRY_NAME_MAXSIZE  + 1];  // one more for the \o
  unsigned int maxLengthValue;  // determines the bytes being reserved
  unsigned int actualLengthValue;
  unsigned int valuePosition; // position index into value dat file
};

#define PSTORAGE_ENTRY_MAXSIZE (PSTORAGE_ENTRY_VALUE_MAXSIZE + sizeof(PStorageIndexEntry))

void _pStoragedebug(const char *format, ...);

class PStorage {
public:
	PStorage();
	virtual ~PStorage();

	boolean begin();
	boolean end();

	boolean createByteArray(const char *name, unsigned int maxSize);
	boolean writeByteArray(const char *name, byte b[], unsigned int bSize);
	boolean readByteArray(const char *name, byte b[], unsigned int bSize, unsigned int *length);

	boolean createString(const char *name, unsigned int maxLength);
	boolean writeString(const char *name, const char *str);
	boolean readString(const char *name, char *str, unsigned int strSize);

	boolean createInt(const char *name);
	boolean writeInt(const char *name, int value);
	boolean readInt(const char *name, int *result);

	boolean createUnsignedInt(const char *name);
	boolean writeUnsignedInt(const char *name, unsigned int value);
	boolean readUnsignedInt(const char *name, unsigned int *result );

	boolean createLong(const char *name);
	boolean writeLong(const char *name, long value);
	boolean readLong(const char *name, long *result);

	boolean createUnsignedLong(const char *name);
	boolean writeUnsignedLong(const char *name, unsigned long value);
	boolean readUnsignedLong(const char *name, unsigned long *result);

	boolean createFloat(const char *name);
	boolean writeFloat(const char *name, float value);
	boolean readFloat(const char *name, float *result);

	unsigned int maxSize();

private:
	void _indexEntrySerialize(const PStorageIndexEntry indexEntry, byte buf[]);
	void _indexEntryDeserialize(const byte buf[], PStorageIndexEntry *indexEntry);

	boolean _readIndexEntry(File indexFile, PStorageIndexEntry *indexEntry, const unsigned int position);
	boolean _writeIndexEntry(File indexFile, const PStorageIndexEntry indexEntry, const unsigned int position);
	boolean _searchIndexEntry(const char *name, File indexFile, unsigned int *position);

	boolean _createEntry(const char *name, File indexFile, File valueFile, unsigned int maxLengthValue);
	boolean _writeEntry(const char *name, byte* writeBuf, unsigned int length, File indexFile, File valueFile);
	boolean _readEntry(const char *name, File indexFile, File valueFile, byte* buf, unsigned int bufSize, unsigned int *bytesReads);

	const char* _getIndexFileName();
	const char* _getValueFileName();

	boolean _open();
	void _close();

	File _indexFile, _valueFile;
};


#endif /* PSTORAGE_H_ */
