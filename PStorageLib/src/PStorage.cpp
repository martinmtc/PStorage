/*
 * PStorage.cpp
 *
 *  Created on: 06.06.2018
 *      Author: Dr. Martin Schaaf
 */

#include "PStorage.h"


#if(PSTORAGE_DEBUG_ENABLED)
void _pStoragedebug(const char *format, ...) {
	char logBuffer[256];
	va_list argList;
	va_start(argList, format);
	vsnprintf(logBuffer, sizeof(logBuffer), format, argList);
	Serial.println(String(logBuffer));
	va_end(argList);
}
#endif

PStorage::PStorage() {
}

PStorage::~PStorage() {
}


unsigned int PStorage::maxSize() {
	FSInfo fsInfo;
	SPIFFS.info(fsInfo);
	return PSTORAGE_SPIFFS_ALLOCFACTOR/100 * fsInfo.totalBytes;
}


boolean PStorage::begin() {
	PSTORAGE_DEBUG("init(): Called");
	PSTORAGE_DEBUG("PStorage() initialize");
	PSTORAGE_DEBUG("Statistics");
	PSTORAGE_DEBUG("---------------------------------------------------------------------------------------------------------------");
	PSTORAGE_DEBUG("SPIFFS size:                        %u kBytes", _mySPIFFS.totalSize() / 1024);
	PSTORAGE_DEBUG("SPIFFS Allocation Factor:           %u %", PSTORAGE_SPIFFS_ALLOCFACTOR);
	PSTORAGE_DEBUG("Max size of storage files combined: %u", maxSize());
	PSTORAGE_DEBUG("Max value size of entry:            %u bytes", PSTORAGE_ENTRY_MAXSIZE);
	PSTORAGE_DEBUG("Max size of entry name:             %u bytes", PSTORAGE_ENTRY_NAME_MAXSIZE);
	PSTORAGE_DEBUG("Upper bound estimation of entries:  %u", maxSize() / (sizeof(PStorageIndexEntry) + 1));
	PSTORAGE_DEBUG("---------------------------------------------------------------------------------------------------------------");

	// first we determine if reset was Deep Sleep
	if (ESP.getResetReason().equals("Deep-Sleep Wake")) {
		// we have to restore the persistent variables
		if (!_open()) {
			PSTORAGE_DEBUG("Could not open Persistent Storage, cannot continue");
			return false;
		}
		else {
			return true;
		}
	}
	PSTORAGE_DEBUG("init(): Try to create index file %s", _getIndexFileName());
	_indexFile = SPIFFS.open(_getIndexFileName(), "w+");  // open for reading and writing, stream is positioned at the beginning
	if (!_indexFile) {
		PSTORAGE_DEBUG("init(): Could not init %s", _getIndexFileName());
		return false;
	}
	PSTORAGE_DEBUG("init(): Try to create value file %s", _getValueFileName());
	_valueFile = SPIFFS.open(_getValueFileName(), "w+");
	if (!_valueFile) {
		PSTORAGE_DEBUG("init(): Could not init %s", _getValueFileName());
		return false;
	}
	PSTORAGE_DEBUG("init(): Successfully initialized index file \"%s\" and value file \"%s\"", _indexFile.name(), _valueFile.name());
	PSTORAGE_DEBUG("init(): File Size of index file %u kBytes, value file %u kBytes", _indexFile.size()/1024, _valueFile.size()/1024);
	return true;
}

boolean PStorage::end() {
	_close();
	return true;
}

boolean PStorage::_open() {
	PSTORAGE_DEBUG("_open(); Called");
	PSTORAGE_DEBUG("_open(): Try to open index file %s", _getIndexFileName());
	_indexFile = SPIFFS.open(_getIndexFileName(), "r+"); // open for reading and writing, stream is positioned at the beginning
	if (!_indexFile) {
		PSTORAGE_DEBUG("_openp(): Could not open %s", _getIndexFileName());
		return false;
	}
	PSTORAGE_DEBUG("_open(): Try to open value file %s", _getValueFileName());
	_valueFile = SPIFFS.open(_getValueFileName(), "r+");
	if (!_valueFile) {
		PSTORAGE_DEBUG("_openp(): Could not open %s", _getValueFileName());
		return false;
	}
	PSTORAGE_DEBUG("_open(): Successfully opened index file \"%s\" and value file \"%s\"", _getIndexFileName(), _getValueFileName());
	PSTORAGE_DEBUG("open(): File Size of index file %u kBytes, value file %u kBytes", _indexFile.size()/1024, _valueFile.size()/1024);
	return true;
}


void PStorage::_close() {
	PSTORAGE_DEBUG("_close(): Called");
	_indexFile.close();
	_valueFile.close();
}

void PStorage::_indexEntrySerialize(const PStorageIndexEntry indexEntry, byte buf[]) {   // Serialize _pstorageIndexEntry into buf[sizeof(_pStorageIndexEntry)]
	unsigned int uiv;
	unsigned long ulv;
	PSTORAGE_DEBUG("_indexEntrySerialize(): Called");
	int index = 0;
	for (unsigned int i = 0; i < sizeof(indexEntry.name); i++) {
		buf[index++] = (byte) indexEntry.name[i];
	}
	uiv = indexEntry.maxLengthValue;
	for (unsigned int i = 0; i < sizeof(indexEntry.maxLengthValue); i++) {
		byte b = uiv % 256;
		uiv = uiv >> 8;
		buf[index++] = b;
	}
	uiv = indexEntry.actualLengthValue;
	for (unsigned int i = 0; i < sizeof(indexEntry.actualLengthValue); i++) {
		byte b = uiv % 256;
		uiv = uiv >> 8;
		buf[index++] = b;
	}
	ulv = indexEntry.valuePosition;
	for (unsigned int i = 0; i < sizeof(indexEntry.valuePosition); i++) {
		byte b = ulv % 256;
		ulv = ulv >> 8;
		buf[index++] = b;
	}
	PSTORAGE_DEBUG("_indexEntrySerialize(): Return");
}

void PStorage::_indexEntryDeserialize(const byte buf[], PStorageIndexEntry *indexEntry) {  // De-serialize _pStorageIndexEntry from _pStorageIndexEntryBuf[sizeof(_pStorageIndexEntry)]
	unsigned int uiv;
	unsigned long ulv;
	byte b;
	PSTORAGE_DEBUG("_indexEntryDeserialize(): Called");
	int index = 0;
	for (unsigned int i = 0; i < sizeof(indexEntry->name); i++) {
		indexEntry->name[i] = (char) buf[index++];
	}
	uiv = 0;
	for (unsigned int i = 0; i < sizeof(indexEntry->maxLengthValue); i++) {
		b = buf[index++];
		uiv = uiv + (b << (8 * i));
	}
	indexEntry->maxLengthValue = uiv;
	uiv = 0;
	for (unsigned int i = 0; i < sizeof(indexEntry->actualLengthValue); i++) {
		b = buf[index++];
		uiv = uiv + (b << (8 * i));
	}
	indexEntry->actualLengthValue = uiv;
	ulv = 0;
	for (unsigned int i = 0; i < sizeof(indexEntry->valuePosition); i++) {
		b = buf[index++];
		ulv = ulv + (b << (8 * i));
	}
	indexEntry->valuePosition = ulv;
	PSTORAGE_DEBUG("_indexEntryDeserialize(): Return");
}

boolean PStorage::_readIndexEntry(File indexFile, PStorageIndexEntry *indexEntry, const unsigned int position) {  // reads an index entry from the current file position
	byte indexEntryBuf[sizeof(PStorageIndexEntry)];
	PSTORAGE_DEBUG("_readIndexEntry(): Called with position %u", position);
	if (!indexFile.seek(position, SeekSet)) {
		PSTORAGE_DEBUG("_readIndexEntry(): Could not set file position to %u", position);
		return false;
	}
	unsigned int bytesRead = indexFile.readBytes((char*) indexEntryBuf, sizeof(indexEntryBuf));
	if (bytesRead != sizeof(indexEntryBuf)) {
		PSTORAGE_DEBUG("_readIndexEntry(): Could only read %u bytes instead of %u bytes from Index File", bytesRead, sizeof(PStorageIndexEntry));
		return false;
	}
	_indexEntryDeserialize(indexEntryBuf, indexEntry);
	PSTORAGE_DEBUG("_readIndexEntry(): Return true with %u bytes read", bytesRead);
	return true;
}

boolean PStorage::_writeIndexEntry(File indexFile, const PStorageIndexEntry indexEntry, const unsigned int position) {  // writes an index entry at the current position
	byte indexEntryBuf[sizeof(PStorageIndexEntry)];
	PSTORAGE_DEBUG("_writeIndexEntry(): Called with position %u", position);
	if (!indexFile.seek(position, SeekSet)) {
		PSTORAGE_DEBUG("_writeIndexEntry(): Could not set file position to %u", position);
		return false;
	}
	_indexEntrySerialize(indexEntry, indexEntryBuf); // _pStorageIndexEntry -----> _pStorageIndexEntryBuf[sizeof(_pStorageIndexEntry)]
	unsigned int bytesWritten = indexFile.write(indexEntryBuf, sizeof(indexEntryBuf));
	if (bytesWritten != sizeof(indexEntryBuf)) {
		PSTORAGE_DEBUG("_writeIndexEntry(): Could only write %u bytes instead of %u bytes from Index File", bytesWritten, sizeof(PStorageIndexEntry));
		return false;
	}
	indexFile.flush();  // ensure writing
	PSTORAGE_DEBUG("_pStorageWriteIndexEntry(): Return true with %u bytes written", bytesWritten);
	return true;
}

boolean PStorage::_searchIndexEntry(const char *name, File indexFile, unsigned int *position) { // return true if success, file pointer points to the index entry if found
	PStorageIndexEntry indexEntry;
	*position = 0;
	indexFile.seek(*position,SeekSet);
	PSTORAGE_DEBUG("_searchIndexEntry(): Called with name \"%s\"", name);
	// TODO: Improve linear search
	while (indexFile.available()) {
		_readIndexEntry(indexFile, &indexEntry, *position);
		if (strcasecmp(indexEntry.name, name) == 0) { // found
			PSTORAGE_DEBUG("_searchIndexEntry(): Found \"%s\" at position %u", name, *position);
			return true;
		}
		*position += sizeof(PStorageIndexEntry);
	}
	PSTORAGE_DEBUG("_searchIndexEntry(): Not found \"%s\"", name);
	return false;
}

boolean PStorage::_createEntry(const char *name, File indexFile, File valueFile, unsigned int maxLengthValue) {  // create an entry of maxLength bytes with name 'name'
	PStorageIndexEntry indexEntry;
	unsigned int position;
	const char valuePlaceholder = '.';  // allocate the requested value buffer by writing something into it
	PSTORAGE_DEBUG("_createEntry(): Called with name \"%s\" and maxLengthValue: %d>", name, maxLengthValue);

	if (strlen(name) > PSTORAGE_ENTRY_NAME_MAXSIZE) { // check if max size for a name (including \0 !!!) is exceeded
		PSTORAGE_DEBUG("_createEntry(): Length of name \"%s\" exceeds defined max of %u", name, PSTORAGE_ENTRY_NAME_MAXSIZE);
		return false;
	}
	if (maxLengthValue > PSTORAGE_ENTRY_VALUE_MAXSIZE) {  // check if the max size for a value is exceeded
		PSTORAGE_DEBUG("_createEntry(): maxLengthValue %u exceeds defined max of %u", maxLengthValue, PSTORAGE_ENTRY_VALUE_MAXSIZE);
		return false;
	}
	if (indexFile.size() + valueFile.size() > maxSize()) {  // check if there is still storage left (!!!!)
		// TODO: Die size() Methoden liefern nix sinnvolles
		PSTORAGE_DEBUG("_createEntry(): pStorage full, combined file size is %u kBytes, max is %u kBytes",
				(indexFile.size() +  valueFile.size()) / 1024, maxSize());
		return false;
	}
	if (_searchIndexEntry(name, indexFile, &position)) {
		PSTORAGE_DEBUG("_createEntry(): Entry with name %s already exists", name);
		return false; // entry already exists
	}
	if (!indexFile.seek(0, SeekEnd) || !indexFile.seek(0, SeekEnd)) {  // move the file pointer to end of index and value file
		PSTORAGE_DEBUG("_createEntry(): Could not move file position to end of file");
		return false;
	}
	// load the index entry
	strcpy(indexEntry.name, name);
	indexEntry.maxLengthValue = maxLengthValue;
	indexEntry.actualLengthValue = 0;
	indexEntry.valuePosition = valueFile.position();
	_writeIndexEntry(indexFile, indexEntry, indexFile.position());
	for (unsigned int i = 0; i < maxLengthValue; i++) { // allocate value space
		valueFile.write(valuePlaceholder);
	}
	valueFile.flush(); // ensure writing
	PSTORAGE_DEBUG("_createEntry(): Successfully created entry with name \"%s\"", name);
	PSTORAGE_DEBUG("createEntry(): File Size of index file %u kBytes, value file %u kBytes", _indexFile.size()/1024, _valueFile.size()/1024);
	return true;
}


boolean PStorage::_writeEntry(const char *name, byte* writeBuf, unsigned int bufSize, File indexFile, File valueFile) {
	unsigned int position;
	PStorageIndexEntry indexEntry;
	PSTORAGE_DEBUG("_writeEntry(): Called with name \"%s\" and Write Buffer of length : %u>", name, bufSize);
	if (!_searchIndexEntry(name, indexFile, &position)) {
		PSTORAGE_DEBUG("_writeEntry(): Return False, entry not found");
		return false;
	}
	if (!_readIndexEntry(indexFile, &indexEntry, position)) {
		PSTORAGE_DEBUG("_writeEntry(): Entry not readable at position %u. Maybe SPIFFS bug", position);  // should not happen since the name has been successfully found
		return false;
	}
	if (indexEntry.maxLengthValue < bufSize) {
		PSTORAGE_DEBUG("_writeEntry(): Initialized maxLength of %u exceeded by requested length %u", indexEntry.maxLengthValue, bufSize);
		return false;
	}
	// now write the value date into the value file
	if (!valueFile.seek(indexEntry.valuePosition, SeekSet)) {
		PSTORAGE_DEBUG("_writeEntry(): Index corrupted, could not set file position to %u", indexEntry.valuePosition);
	}
	unsigned int bytesWritten = valueFile.write(writeBuf, bufSize);
	if (bytesWritten != bufSize) {
		PSTORAGE_DEBUG("_writeEntry(): Index corrupted or SPIFFS bug, could not write at position %u", indexEntry.valuePosition);
		return false;
	}
	valueFile.flush();
	// and update the index about the new actual length
	indexEntry.actualLengthValue = bufSize;
	if (!_writeIndexEntry(indexFile, indexEntry, position)) {  // update the index file at the position (hopefully) determined by _pStorageSearchIndex
		PSTORAGE_DEBUG("_pStorageWrite(): Cannot update index at position %u. Maybe SPIFFS bug", position); // should not happen after successfull Read
		return false;
	}
	PSTORAGE_DEBUG("_pStorageWrite(): Write successful, new size index file %u, value file %u", indexFile.size(), valueFile.size());
	return true;
}

boolean PStorage::_readEntry(const char *name, File indexFile, File valueFile, byte* buf, unsigned int bufSize, unsigned int *bytesRead) {  // expects readBuf to be large enough
	unsigned int position;
	PStorageIndexEntry indexEntry;
	PSTORAGE_DEBUG("_readEntry(): Called with name \"%s\"", name);
	if (!_searchIndexEntry(name, indexFile, &position)) {
		PSTORAGE_DEBUG("_readEntry(): Cannot find entry with name %s", name);
		return false;  // entry is loaded on success
	}
	if (!_readIndexEntry(indexFile, &indexEntry, position)) {
		PSTORAGE_DEBUG("_readEntry(): Index entry not readable at position %u. Maybe SPIFFS bug", position); // should not happen since the name has been successfully found
		return false;
	}
	if (!valueFile.seek(indexEntry.valuePosition, SeekSet)) {
		PSTORAGE_DEBUG("_readEntry(): Index corrupted, could not set value file position to &u", position);
		return false;
	}
	*bytesRead = valueFile.read(buf, min(indexEntry.actualLengthValue, bufSize));  // with buffer protection
	if (*bytesRead != indexEntry.actualLengthValue) {
		PSTORAGE_DEBUG("_readEntry(): Partially read %u bytes instead of actual value size %u", *bytesRead, indexEntry.actualLengthValue);
		return false;
	}
	PSTORAGE_DEBUG("_readEntry(): Read successful with %u bytes read", *bytesRead);
	return true;
}

const char* PStorage::_getIndexFileName() {
	return "/mem/index.dat";
}

const char* PStorage::_getValueFileName() {
	return "/mem/db.dat";
}

/*
   Convenience Methods
 */

#define INT_CHARS_DEC (sizeof(int) * 3 + 1 + 1) // string holding int and uint in dec. repr., 2 extra bytes for sign and '\0' string terminator
#define LONG_CHARS_DEC (sizeof(long) * 3 + 1 + 1) // string holding long and ulong in dec. representation
#define FLOAT_CHARS_DEC 15  // string expected to be enough for float TODO: Besser machen

boolean PStorage::createByteArray(const char *name, unsigned int maxSize) {
	PSTORAGE_DEBUG("createByteArray(): Called with name \"%s\" and length %u", name, maxSize);
	return _createEntry(name, _indexFile, _valueFile, maxSize);
}

boolean PStorage::writeByteArray(const char *name, byte b[], unsigned int bSize) {
	PSTORAGE_DEBUG("writeByteArray(): Called with name \"%s\"", name);
	return _writeEntry(name, b, bSize, _indexFile, _valueFile);
}

boolean PStorage::readByteArray(const char *name, byte b[], unsigned int bSize, unsigned int *length) {
	PSTORAGE_DEBUG("readByteArray(): Called with name \"%s\"", name);
	return _readEntry(name, _indexFile, _valueFile, b, bSize, length);
}


boolean PStorage::createString(const char *name, unsigned int maxLength) {
	PSTORAGE_DEBUG("createString(): Called with name \"%s\" and max length %u", name, maxLength);
	return createByteArray(name, maxLength  + 1); // add 1 for the '\0'
}

boolean PStorage::writeString(const char *name, const char *str) {
	PSTORAGE_DEBUG("writeString(): Called with name \"%s\" and value \"%s\"", name, str);
	return writeByteArray(name, (byte *) str, strlen(str) +1); // we want to have trailing '\0' included
}

boolean PStorage::readString(const char *name, char *str, unsigned int strSize) {
	unsigned int length;
	PSTORAGE_DEBUG("readString(): Called with name \"%s\"", name);
	return readByteArray(name, (byte*) str, strSize, &length); // length should be strlen(...) + 1 but not of interest, here
}

boolean PStorage::createInt(const char *name) {
	PSTORAGE_DEBUG("createInt(): Called with name \"%s\"", name);
	return createString(name, INT_CHARS_DEC);
}

boolean PStorage::writeInt(const char *name, int value) {
	PSTORAGE_DEBUG("writeInt(): Called with name \"%s\" and value %d", name, value);
	char str[INT_CHARS_DEC];
	sprintf(str,"%d",value);
	return writeString(name, str);
}

boolean PStorage::readInt(const char *name, int *result) {
	PSTORAGE_DEBUG("readInt(): Called with name \"%s\"", name);
	char str[INT_CHARS_DEC];
	if (!readString(name, str, INT_CHARS_DEC)) return false;
	*result = strtol(str, NULL, 10);
	return true;
}

boolean PStorage::createUnsignedInt(const char *name) {
	PSTORAGE_DEBUG("createUnsignedInt(): Called with name \"%s\"", name);
	return createString(name, INT_CHARS_DEC);
}

boolean PStorage::writeUnsignedInt(const char *name, unsigned int value) {
	PSTORAGE_DEBUG("writeUnsignedInt(): Called with name \"%s\" and value : %u", name, value);
	char str[INT_CHARS_DEC];
	sprintf(str, "%u", value);
	return writeString(name, str);
}

boolean PStorage::readUnsignedInt(const char *name, unsigned int *result ) {
	PSTORAGE_DEBUG("readUnsignedInt(): Called with name \"%s\"", name);
	char str[INT_CHARS_DEC];
	if (!readString(name, str, INT_CHARS_DEC)) return false;
	*result = strtoul(str, NULL, 10);
	return true;
}

boolean PStorage::createLong(const char *name) {
	PSTORAGE_DEBUG("createLong(): Called with name \"%s\"", name);
	return createString(name, LONG_CHARS_DEC);
}

boolean PStorage::writeLong(const char *name, long value) {
	PSTORAGE_DEBUG("writeLong(): Called with name \"%s\" and value %ld", name, value);
	char str[LONG_CHARS_DEC];
	sprintf(str, "%ld", value);
	return writeString(name, str);
}

boolean PStorage::readLong(const char *name, long *result) {
	PSTORAGE_DEBUG("readLong(): Called with name \"%s\"", name);
	char str[LONG_CHARS_DEC];
	if (!readString(name, str, LONG_CHARS_DEC)) return false;
	*result = strtol(str, NULL, 10);
	return true;
}

boolean PStorage::createUnsignedLong(const char *name) {
	PSTORAGE_DEBUG("createUnsignedLong(): Called with name \"%s\"", name);
	return createString(name, LONG_CHARS_DEC);
}

boolean PStorage::writeUnsignedLong(const char *name, unsigned long value) {
	PSTORAGE_DEBUG("writeUnsignedLong(): Called with name \"%s\" and value %lu", name, value);
	char str[LONG_CHARS_DEC];
	sprintf(str, "%lu", value);
	return writeString(name, str);
}

boolean PStorage::readUnsignedLong(const char *name, unsigned long *result) {
	PSTORAGE_DEBUG("readUnsignedLong(): Called with name \"%s\"", name);
	char str[LONG_CHARS_DEC];
	if (!readString(name, str, LONG_CHARS_DEC)) return false;
	*result = strtoul(str, NULL, 10);
	return true;
}

boolean PStorage::createFloat(const char *name) {
	PSTORAGE_DEBUG("createFloat(): Called with name \"%s\"", name);
	return createString(name, FLOAT_CHARS_DEC);
}

boolean PStorage::writeFloat(const char *name, float value) {
	PSTORAGE_DEBUG("writeFloat(): Called with name \"%s\" and value %f", name, value);
	char str[FLOAT_CHARS_DEC];
	sprintf(str, "%f", value);
	return writeString(name, str);
}

boolean PStorage::readFloat(const char *name, float *result) {
	PSTORAGE_DEBUG("readFloat(): Called with name \"%s\"", name);
	char str[FLOAT_CHARS_DEC];
	if (!readString(name, str, FLOAT_CHARS_DEC)) return false;
	*result = strtof(str, NULL);
	return true;
}

