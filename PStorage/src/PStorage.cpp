/*
 * PStorage.cpp
 *
 *  Created on: 06.06.2018
 *      Author: Dr. Martin Schaaf
 */

#include "PStorage.h"

PStorage pStorage;

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
	return fsInfo.totalBytes/100 * PSTORAGE_SPIFFS_ALLOCFACTOR;
}

#if(PSTORAGE_DEBUG_ENABLED)
unsigned int spiffsSize() {
	FSInfo fsInfo;
	SPIFFS.info(fsInfo);
	return fsInfo.totalBytes;
}
#endif

void printPStorageStats() {
	PSTORAGE_DEBUG("Statistics");
	PSTORAGE_DEBUG("---------------------------------------------------------------------------------------------------------------");
	PSTORAGE_DEBUG("SPIFFS size:\t\t\t\t\t\t\t\t\t\t\t\t\t\t %u kBytes", spiffsSize() / 1024);
	PSTORAGE_DEBUG("SPIFFS Allocation Factor:\t\t\t\t\t\t\t\t %u %", PSTORAGE_SPIFFS_ALLOCFACTOR);
	PSTORAGE_DEBUG("Max size of storage files combined:\t\t\t %u kBytes", pStorage.maxSize() / 1024);
	PSTORAGE_DEBUG("Max value size of entry:\t\t\t\t\t\t\t\t %u Bytes", PSTORAGE_ENTRY_VALUE_MAXSIZE);
	PSTORAGE_DEBUG("Max size of entry name:\t\t\t\t\t\t\t\t %u Bytes", PSTORAGE_ENTRY_NAME_MAXSIZE);
	PSTORAGE_DEBUG("Size of Index Entry (incl. name)\t\t\t\t\t %u Bytes", sizeof(PStorageIndexEntry));
	PSTORAGE_DEBUG("Upper bound estimation of entries:\t\t\t %u", pStorage.maxSize() / (sizeof(PStorageIndexEntry) + 1));
	PSTORAGE_DEBUG("Lower bound estimation of entries:\t\t\t %u", pStorage.maxSize() / PSTORAGE_ENTRY_MAXSIZE);
	PSTORAGE_DEBUG("---------------------------------------------------------------------------------------------------------------");
}


boolean PStorage::create() {
	printPStorageStats();
	PSTORAGE_DEBUG("init(): Try to create index file %s", _getIndexFileName());
	File indexFile = SPIFFS.open(_getIndexFileName(), "w+");  // open for reading and writing, stream is positioned at the beginning
	if (!indexFile) {
		PSTORAGE_DEBUG("init(): Could not init %s", _getIndexFileName());
		return false;
	}
	PSTORAGE_DEBUG("init(): Try to create value file %s", _getValueFileName());
	File valueFile = SPIFFS.open(_getValueFileName(), "w+");
	if (!valueFile) {
		PSTORAGE_DEBUG("init(): Could not init %s", _getValueFileName());
		// house keeping
		indexFile.close();
		SPIFFS.remove(_getIndexFileName());

		return false;
	}
	// now we initialize and write the control structure
	_pcp.maxEntries = PSTORAGE_MAX_ENTRIES;
	_pcp.valueFileMaxSize = PSTORAGE_VALUEFILE_MAXSIZE;
	_pcp.entryNameMaxSize = PSTORAGE_ENTRY_NAME_MAXSIZE;
	_pcp.actualEntries = 0;
	_pcp.actualValueFileSize = 0;
	byte buf[sizeof(PStorageCtrlParams)];
	_ctrlParamsSerialize(_pcp, buf);
	unsigned int bytesWritten = indexFile.write(buf, sizeof(buf));
	if (bytesWritten != sizeof(buf)) {
		PSTORAGE_DEBUG("init(): Could only write %u bytes instead of %u bytes from Index File", bytesWritten, sizeof(buf));
		// house keeping
		indexFile.close();
		valueFile.close();
		SPIFFS.remove(_getIndexFileName());
		SPIFFS.remove(_getValueFileName());
		return false;
	}
	indexFile.flush();  // ensure writing
	// and immediately close both files
	indexFile.close();
	valueFile.close();
	PSTORAGE_DEBUG("init(): Successfully initialized index file \"%s\" and value file \"%s\"", _getIndexFileName(), _getValueFileName());
	return true;
}

void PStorage::_serializeUINT(const unsigned int v, byte buf[], int *index) {
	for (int i = 0; i < sizeof(unsigned int); i++) {
		byte b = v % 256;
		v = v >> 8;
		buf[(*index)++] = b;
	}
}

unsigned int PStorage::_deserializeUNIT(const byte buf[], int *index) {
	unsigned int result = 0;
	byte b;
	for (int i = 0; i < sizeof(unsigned int); i++) {
		b = buf[(*index)++];
		result = result + (b << (8 * i));
	}
	return result;
}

void PStorage::_ctrlParamsSerialize(const PStorageCtrlParams pcp, byte buf[]) {
	PSTORAGE_DEBUG("_ctrlParamsSerialize(): Called");
	int index = 0;
	_serializeUINT(pcp.maxEntries, buf, &index);
	_serializeUINT(pcp.valueFileMaxSize, buf, &index);
	_serializeUINT(pcp.entryNameMaxSize, buf, &index);
	_serializeUINT(pcp.actualEntries, buf, &index);
	_serializeUINT(pcp.actualValueFileSize, buf, &index);
}

void PStorage::_ctrlParamsDeserialize(const byte buf[], PStorageCtrlParams *pcp) {
	int index = 0;
	pcp->maxEntries = _deserializeUNIT(buf, &index);
	pcp->valueFileMaxSize = _deserializeUNIT(buf, &index);
	pcp->entryNameMaxSize = _deserializeUNIT(buf, &index);
	pcp->actualEntries = _deserializeUNIT(buf, &index);
	pcp->actualValueFileSize = _deserializeUNIT(buf, &index);

}

void PStorage::_indexEntrySerialize(const PStorageIndexEntry indexEntry, byte buf[]) {   // Serialize _pstorageIndexEntry into buf[sizeof(_pStorageIndexEntry)]
	PSTORAGE_DEBUG("_indexEntrySerialize(): Called");
	int index = 0;
	for (unsigned int i = 0; i < sizeof(indexEntry.name); i++) {
		buf[index++] = (byte) indexEntry.name[i];
	}
	_serializeUINT(indexEntry.maxValueSize, buf, &index);
	_serializeUINT(indexEntry.actualValueSize, buf, &index);
	_serializeUINT(indexEntry.valuePosition, buf, &index);
	buf[index++] = indexEntry.type;  // finally the type
	PSTORAGE_DEBUG("_indexEntrySerialize(): Return");
}

void PStorage::_indexEntryDeserialize(const byte buf[], PStorageIndexEntry *indexEntry) {  // De-serialize _pStorageIndexEntry from _pStorageIndexEntryBuf[sizeof(_pStorageIndexEntry)]
	PSTORAGE_DEBUG("_indexEntryDeserialize(): Called");
	int index = 0;
	for (unsigned int i = 0; i < sizeof(indexEntry->name); i++) {
		indexEntry->name[i] = (char) buf[index++];
	}
	indexEntry->maxValueSize = _deserializeUNIT(buf, &index);
	indexEntry->actualValueSize = _deserializeUNIT(buf, &index);
	indexEntry->valuePosition = _deserializeUNIT(buf, &index);
	indexEntry->type = buf[index++];
	PSTORAGE_DEBUG("_indexEntryDeserialize(): Return");
}

boolean PStorage::_readIndexEntry(File indexFile, PStorageIndexEntry *indexEntry, const unsigned int position) {  // reads an index entry from the current file position
	PSTORAGE_DEBUG("_readIndexEntry(): Called with position %u", position);
	byte indexEntryBuf[sizeof(PStorageIndexEntry)];
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

boolean PStorage::_writeIndexEntry(File indexFile, const PStorageIndexEntry indexEntry, const unsigned int position) {  // writes an index entry
	byte indexEntryBuf[sizeof(PStorageIndexEntry)];
	PSTORAGE_DEBUG("_writeIndexEntry(): Called with position %u", position);
	if (!indexFile.seek(position, SeekSet)) {
		PSTORAGE_DEBUG("_writeIndexEntry(): Could not set file position to %u", position);
		return false;
	}
	_indexEntrySerialize(indexEntry, indexEntryBuf); // _pStorageIndexEntry -----> _pStorageIndexEntryBuf[sizeof(_pStorageIndexEntry)]
	unsigned int bytesWritten = indexFile.write(indexEntryBuf, sizeof(indexEntryBuf));
	if (bytesWritten != sizeof(indexEntryBuf)) {
		PSTORAGE_DEBUG("_writeIndexEntry(): Could only write %u bytes instead of %u bytes from Index File", bytesWritten, sizeof(indexEntryBuf));
		return false;
	}
	indexFile.flush();  // ensure writing
	PSTORAGE_DEBUG("_pStorageWriteIndexEntry(): Return true with %u bytes written", bytesWritten);
	return true;
}

boolean PStorage::_searchIndexEntry(const byte type, const char *name, File indexFile, unsigned int *position) { // return true if success, file pointer points to the index entry if found
	PStorageIndexEntry indexEntry;
	*position = sizeof(PStorageCtrlParams); // position after the control structure
	indexFile.seek(*position,SeekSet);
	PSTORAGE_DEBUG("_searchIndexEntry(): Called with type \"u\" abd name \"%s\"", type, name);
	// TODO: Improve linear search
	while (indexFile.available()) {
		_readIndexEntry(indexFile, &indexEntry, *position);
		if ( (strcasecmp(indexEntry.name, name) == 0) &&
				(indexEntry.type == type) ) { // found
			PSTORAGE_DEBUG("_searchIndexEntry(): Found \"%s\" at position %u", name, *position);
			return true;
		}
		*position += sizeof(PStorageIndexEntry);
	}
	PSTORAGE_DEBUG("_searchIndexEntry(): Not found \"%s\"", name);
	return false;
}


!!!!!! hier müssen jetzt die Dateien neu geöffnet werden


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

boolean PStorage::_getPStorageFiles(File *indexFile, File *valueFile) {
	byte buf[sizeof(PStorageCtrlParams)];
	PSTORAGE_DEBUG("_getPStorageFiles(): Called");
	*indexFile = SPIFFS.open(_getIndexFileName(), "r+"); // open for reading and writing, stream is positioned at the beginning
	if (!(*indexFile)) {
		PSTORAGE_DEBUG("_getPStorageFiles(): Could not open %s", _getIndexFileName());
		return false;
	}
	// update control structure
	unsigned int bytesRead = indexFile->readBytes((char*) buf, sizeof(buf));
	if (bytesRead != sizeof(buf)) {
		PSTORAGE_DEBUG("_getPStorageFiles(): Could only read %u bytes instead of %u bytes from Index File", bytesRead, sizeof(buf));
		return false;
	}
	_ctrlParamsDeserialize(buf, &_pcp);
	// finally we have to check if PSTORAGE_ENTRY_NAME_MAXSIZE still fits. Otherwise no buffers will fit and we have to re-init
	if (_pcp.entryNameMaxSize != PSTORAGE_ENTRY_NAME_MAXSIZE) {
		PSTORAGE_DEBUG("_getPStorageFiles(): Entry name max size of %u in index file does not match actual configuration %u",
				_pcp.entryNameMaxSize, PSTORAGE_ENTRY_NAME_MAXSIZE);
		return false;
	}
	*valueFile = SPIFFS.open(_getValueFileName(), "r+"); // open for reading and writing, stream is positioned at the beginning
	if (!(*valueFile)) {
		PSTORAGE_DEBUG("_getPStorageFiles(): Could not open %s", _getValueFileName());
		return false;
	}
	return true;
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

boolean PStorage::map(const char* name, int value) {
}

boolean PStorage::get(const char* name, int* value) {
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

boolean PStorage::map(const char* name, unsigned int value) {
}

boolean PStorage::get(const char* name, unsigned int* value) {
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

boolean PStorage::map(const char* name, long value) {
}

boolean PStorage::get(const char* name, long * value) {
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

boolean PStorage::map(const char* name, unsigned long value) {
}

boolean PStorage::get(const char* name, unsigned long * value) {
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

boolean PStorage::map(const char* name, float value) {
}

boolean PStorage::get(const char* name, float* value) {
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

