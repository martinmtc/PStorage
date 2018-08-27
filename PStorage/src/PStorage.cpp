/*
 * PStorage.cpp
 *
 *  Created on: 28.08.2018
 *      Author: Dr. Martin Schaaf
 */

#include "PStorage.h"

PStorage::~PStorage() {
}

PStorage::PStorage(const char* name) {
	this->_name = name;
	SPIFFS.begin();  // make sure that SPIFFS is mounted, should not harm if called multiple times
}

boolean PStorage::open() {
	PSTORAGE_DEBUG("open(): Called");
	_storageFile = SPIFFS.open(_getStorageFileName(), "r+"); // open for reading and writing, stream is positioned at the beginning
	if (!_storageFile) {
		PSTORAGE_DEBUG("open(): Could not open %s", _getStorageFileName());
		return false;
	}
	if (!_readParams()) {
		PSTORAGE_DEBUG("open(): Could not read parameters from %s", _getStorageFileName());
		return false;
	}
	if (_params.magicCookie != PSTORAGE_MAGIC_COOKIE) {  // incompatible
		return false;
	}
	return true;
}

boolean PStorage::create(unsigned int size) {
	PSTORAGE_DEBUG("create(): Called");

	PStorageIndexEntry ie;
	size += sizeof(PStorageParams);  // always add the storage header
	const unsigned int minSize = sizeof(PStorageParams) + sizeof(PStorageIndexEntry) + PSTORAGE_ENTRY_MINSIZE;
	// adjust size if necessary
	if (size < minSize) {
		PSTORAGE_DEBUG("create(): Requested size %d is smaller than minimal size %d, adjusting", size, minSize);
		size = minSize;
	}
	if (open()) {  // if the storage file already exists we remove it
		_storageFile.close();
		SPIFFS.remove(_getStorageFileName());
		PSTORAGE_DEBUG("create(): Previous storage file deleted");
	}
	// now create & initialize the new storage file
	_storageFile = SPIFFS.open(_getStorageFileName(), "w+"); // open for reading and writing, stream is positioned at the beginning
	if (!_storageFile) {
		PSTORAGE_DEBUG("create(): Could not create %s", _getStorageFileName());
		return false;
	}
	_params.magicCookie = PSTORAGE_MAGIC_COOKIE;
	_params.size = size - sizeof(PStorageParams);
	_params.firstEntry = sizeof(PStorageParams);
	if (!_writeParams()) {
		_storageFile.close();
		SPIFFS.remove(_getStorageFileName());
		PSTORAGE_DEBUG("create(): Could not write storage file parameters, storage removed");
		return false;
	}
	if (!_storageFile.seek(_params.firstEntry, SeekSet)) {
		PSTORAGE_DEBUG("create(): Could not set file position %d", _params.firstEntry);
		return false;
	}
	strcpy(ie.name, "");
	ie.thisEntry = _params.firstEntry;
	ie.nextEntry = ie.thisEntry + _params.size; // this is the right limit
	ie.previousEntry = 0;
	ie.type = P_FREE;

	if (!_writeIndexEntry(ie)) {
		_storageFile.close();
		SPIFFS.remove(_getStorageFileName());
		PSTORAGE_DEBUG("create(): Could not write first index entry, storage removed");
		return false;
	}
	for (unsigned int i = 0; i < _size(ie); i++) {
		if (_storageFile.write(' ') != 1) {
			PSTORAGE_DEBUG("create(): Could not allocate %s bytes, storage removed", ie.size);
			_storageFile.close();
			SPIFFS.remove(_getStorageFileName());
			return false;
		}
	}
	_storageFile.flush();
	return true;
}


boolean PStorage::map(const char *name, int value) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_INT, name, &ie)) {
		_allocate(name, sizeof(value), P_INT, &ie);
	}
	return _writeEntry(ie, (byte *) &value, sizeof(value));
}

boolean PStorage::map(const char *name, unsigned int value) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_UINT, name, &ie)) {
		_allocate(name, sizeof(value), P_UINT, &ie);
	}
	return _writeEntry(ie, (byte *) &value, sizeof(value));
}

boolean PStorage::map(const char *name, long value) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_LONG, name, &ie)) {
		_allocate(name, sizeof(value), P_LONG, &ie);
	}
	return _writeEntry(ie, (byte *) &value, sizeof(value));
}

boolean PStorage::map(const char *name, unsigned long value) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_ULONG, name, &ie)) {
		_allocate(name, sizeof(value), P_ULONG, &ie);
	}
	return _writeEntry(ie, (byte *) &value, sizeof(value));
}

boolean PStorage::map(const char *name, float value) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_FLOAT, name, &ie)) {
		_allocate(name, sizeof(value), P_FLOAT, &ie);
	}
	return _writeEntry(ie, (byte *) &value, sizeof(value));
}

boolean PStorage::map(const char* name, byte b[], unsigned int size) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_ARRAY, name, &ie)) {
		_allocate(name, size, P_ARRAY, &ie);
	}
	else { // found but probably not large enough
		if (_size(ie) < size) {
			_free(&ie);
			_allocate(name, size, P_ARRAY, &ie);
		}
	}
	return _writeEntry(ie, b, size);
}

boolean PStorage::map(const char* name, const char* str) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_STRING, name, &ie)) {
		_allocate(name, strlen(str), P_STRING, &ie);
	}
	else { // found but probably not large enough
		if (_size(ie) < strlen(str)) {
			_free(&ie);
			_allocate(name, strlen(str), P_STRING, &ie);
		}
	}
	return _writeEntry(ie, (byte *) str, strlen(str) + 1);
}

boolean PStorage::get(const char *name, int *value) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_INT, name, &ie)) {
		return false;
	}
	return (_readEntry(ie, (byte *) value, sizeof(value)) >= 0);
}

boolean PStorage::get(const char *name, unsigned int *value) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_UINT, name, &ie)) {
		return false;
	}
	return (_readEntry(ie, (byte *) value, sizeof(value)) >= 0);
}

boolean PStorage::get(const char *name, long *value) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_LONG, name, &ie)) {
		return false;
	}
	return (_readEntry(ie, (byte *) value, sizeof(value)) >= 0);
}

boolean PStorage::get(const char *name, unsigned long *value) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_ULONG, name, &ie)) {
		return false;
	}
	return (_readEntry(ie, (byte *) value, sizeof(value)) >= 0);
}

boolean PStorage::get(const char *name, float *value) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_FLOAT, name, &ie)) {
		return false;
	}
	return (_readEntry(ie, (byte *) value, sizeof(value)) >= 0);
}

boolean PStorage::get(const char* name, byte buf[], unsigned int bufSize) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_ARRAY, name, &ie)) {
		return false;
	}
	return (_readEntry(ie, buf, bufSize) >= 0);
}

boolean PStorage::get(const char* name, char* buf, unsigned int bufSize) {
	PStorageIndexEntry ie;
	if (!_searchIndexEntry(P_STRING, name, &ie)) {
		return false;
	}
	int bytesRead = _readEntry(ie, (byte *) buf, bufSize - 1);
	if (bytesRead >= 0) {
		buf[bytesRead] = '\0';
	}
	return bytesRead >= 0;
}

boolean PStorage::remove(const char *name) {
	PSTORAGE_DEBUG("remove(): Called");

	PStorageIndexEntry ie;
	while (_searchIndexEntry(name, &ie)) {
		if (!_free(&ie)) {
			return false;
		}
	}
	return true;
}


unsigned int PStorage::getAllocatedSize() {
	PSTORAGE_DEBUG("getAllocatedSize(): Called");

	unsigned int result = sizeof(PStorageParams);
	PStorageIndexEntry ie;
	if (!_readFirstIndexEntry(&ie)) {
		return 0;
	}
	boolean entriesAvailable = true;
	while (entriesAvailable) {
		if (ie.type != P_FREE) {
			result += sizeof(PStorageIndexEntry) + (ie.nextEntry - ie.thisEntry);  // compute the real consumption
		}
		if (!_isLastIndexEntry(ie)) {
			if (!_storageFile.seek(ie.nextEntry, SeekSet)) {
				PSTORAGE_DEBUG("getAllocatedSize(): Corruption, could not set position %d", ie.nextEntry);
				return 0;
			}
			if (!_readIndexEntry(&ie)) {
				return 0;
			}
		}
		else {
			entriesAvailable = false;
		}
	}
	return result;
}

unsigned int PStorage::getPStorageSize() {
	PSTORAGE_DEBUG("getPStorageSize(): Called");

	return _params.size;
}

void PStorage::dumpPStorage() {
	boolean stop = false;
	PStorageIndexEntry ie;
	_readFirstIndexEntry(&ie);
	Serial.printf("\n\nDump of %s ---------------------------------------------------------------\n", _name);
	Serial.printf("Storage size: %d bytes, Allocated size: %d bytes\n", getPStorageSize(), getAllocatedSize());
	do {
		Serial.printf("---------------------\n");
		Serial.printf("Name: %s, Type %s, Size: %d\n", ie.name, _printType(ie.type).c_str(), _size(ie));
		Serial.printf("This Entry: %d, Value starts at: %d \n", ie.thisEntry, ie.thisEntry + sizeof(PStorageIndexEntry));
		Serial.printf("Previous Entry: %d, Next Entry: %d\n", ie.previousEntry, ie.nextEntry);
		Serial.printf("Value:\n");
		_printEntry(ie);
		Serial.printf("\n");
		if (!_isLastIndexEntry(ie)) {
			_storageFile.seek(ie.nextEntry, SeekSet);
			_readIndexEntry(&ie);
		}
		else {
			stop = true;
		}
	} while (!stop);
	Serial.printf("%s -----------------------------------------------------------------------\n", _name);
}

boolean PStorage::_readParams() {
	PSTORAGE_DEBUG("_readParams(): Called");

	if (!_storageFile.seek(0, SeekSet)) {
		PSTORAGE_DEBUG("_readParams(): Could not set file position");
		return false;
	}

	byte *ptr = (byte *) &_params;
	for (unsigned int i = 0; i < sizeof(PStorageParams); i++) {
		if (_storageFile.read(ptr + i, 1) != 1) {
			PSTORAGE_DEBUG("_readParams(): Could not read parameters at position %d", _storageFile.position());
			return false;
		}
	}
	return true;
}

boolean PStorage::_writeParams() {
	PSTORAGE_DEBUG("_writeParams(): Called");

	if (!_storageFile.seek(0, SeekSet)) {
		PSTORAGE_DEBUG("_writeParams(): Could not set file position");
		return false;
	}
	byte *ptr = (byte *) &_params;
	for (unsigned int i = 0; i < sizeof(PStorageParams); i++) {
		if (_storageFile.write(*(ptr + i)) != 1) {
			PSTORAGE_DEBUG("_writeParams(): Could not write parameters at position %d", _storageFile.position());
			return false;
		}
	}
	return true;
}


boolean PStorage::_allocate(const char *name, unsigned int size, EntryType type, PStorageIndexEntry *ie) {
	PSTORAGE_DEBUG("_allocate(): Called");

	if (strlen(name) > PSTORAGE_INDEX_NAME_MAXSIZE) {
		PSTORAGE_DEBUG("_allocate(): Name %s exceeds max length of %d bytes", name, PSTORAGE_INDEX_NAME_MAXSIZE);
		return false;
	}
	if (!_searchFreeIndexEntry(size, ie)) {
		return false;
	}
	// check if the entry can be further split
	if (_size(*ie) > size + sizeof(PStorageIndexEntry) + PSTORAGE_ENTRY_MINSIZE) {
		PStorageIndexEntry newIE;
		newIE.thisEntry = ie->thisEntry + sizeof(PStorageIndexEntry) + size;
		newIE.nextEntry = ie->nextEntry;
		newIE.previousEntry = ie->thisEntry;
		newIE.type = P_FREE;
		strcpy(newIE.name, "");
		if (!_storageFile.seek(newIE.thisEntry, SeekSet)) {
			PSTORAGE_DEBUG("_allocate(): Could not set file position to new splitted free index entry %d", newIE.thisEntry);
			return false;
		}
		if (!_writeIndexEntry(newIE)) {
			return false;
		}
		ie->nextEntry = newIE.thisEntry; // wire in
	}
	ie->type = type;
	strcpy(ie->name, name);
	if (!_storageFile.seek(ie->thisEntry, SeekSet)) {
		PSTORAGE_DEBUG("_allocate(): Could not set file position to new free index entry %d", ie->thisEntry);
		return false;
	}
	if (!_writeIndexEntry(*ie)) {
		return false;
	}
	return true;
}

boolean PStorage::_free(PStorageIndexEntry *ie) {
	PSTORAGE_DEBUG("_free(): Called");

	strcpy(ie->name, "");
	ie->type = P_FREE;

	if (!_isFirstIndexEntry(*ie)) {
		// if previous entry is also free it can be merged
		if (!_storageFile.seek(ie->previousEntry, SeekSet)) {
			PSTORAGE_DEBUG("_free(): Could not set file position to %d", ie->previousEntry);
			return false;
		}
		PStorageIndexEntry prevIE;
		_readIndexEntry(&prevIE);
		if (prevIE.type == P_FREE) {
			ie->previousEntry = prevIE.previousEntry;
			ie->thisEntry = prevIE.thisEntry;  // take over previous entry
		}
	}
	if (!_isLastIndexEntry(*ie)) {
		// if next entry is free it can be merged
		if (!_storageFile.seek(ie->nextEntry, SeekSet)) {
			PSTORAGE_DEBUG("_free(): Could not set file position to %d", ie->nextEntry);
			return false;
		}
		PStorageIndexEntry nextIE;
		_readIndexEntry(&nextIE);
		if (nextIE.type == P_FREE) {
			ie->nextEntry = nextIE.nextEntry;  // extend
		}
	}
	if (!_storageFile.seek(ie->thisEntry, SeekSet)) {
		PSTORAGE_DEBUG("_free(): Could not set file position to %d", ie->thisEntry);
		return false;
	}
	_writeIndexEntry(*ie);
	return true;
}


boolean PStorage::_isFirstIndexEntry(PStorageIndexEntry ie) {
	return ie.previousEntry == 0;
}

boolean PStorage::_isLastIndexEntry(PStorageIndexEntry ie) {
	return ie.nextEntry == _params.firstEntry + _params.size;
}

unsigned int PStorage::_size(PStorageIndexEntry ie) {
	return (ie.nextEntry - (ie.thisEntry + sizeof(PStorageIndexEntry)));
}

boolean PStorage::_readFirstIndexEntry(PStorageIndexEntry *ie) {
	if (!_storageFile.seek(_params.firstEntry, SeekSet)) {
		PSTORAGE_DEBUG("_readFirstIndexEntry(): Could not find first index entry");
		return false;
	}
	return _readIndexEntry(ie);
}

boolean PStorage::_readIndexEntry(PStorageIndexEntry* ie) {
	PSTORAGE_DEBUG("_readIndexEntry(): Called");

	byte *ptr = (byte *) ie;
	for (unsigned int i = 0; i < sizeof(PStorageIndexEntry); i++) {
		if (_storageFile.read(ptr + i, 1) != 1) {
			PSTORAGE_DEBUG("_readIndexEntry(): Could not read index entry at position %d", _storageFile.position());
			return false;
		}
	}
	return true;
}

boolean PStorage::_writeIndexEntry(const PStorageIndexEntry ie) {
	PSTORAGE_DEBUG("_writeIndexEntry(): Called");

	byte *ptr = (byte *) &ie;
	for (unsigned int i = 0; i < sizeof(PStorageIndexEntry); i++) {
		if (_storageFile.write(*(ptr + i)) != 1) {
			PSTORAGE_DEBUG("_writeIndexEntry(): Could not write index entry at position %d", _storageFile.position());
			return false;
		}
	}
	_storageFile.flush();
	return true;
}

boolean PStorage::_searchIndexEntry(EntryType type, const char* name, PStorageIndexEntry *ie) {
	PSTORAGE_DEBUG("_searchIndexEntry(): Called");

	if (!_readFirstIndexEntry(ie)) {
		return false;
	}
	while ( (type != ie->type) || (strcasecmp(name, ie->name) != 0) ) {
		if (_isLastIndexEntry(*ie)) {  // we have reached the last entry without match
			return false;
		}
		if (!_storageFile.seek(ie->nextEntry, SeekSet)) {
			PSTORAGE_DEBUG("_searchIndexEntry(): Corruption, could not set %d position", ie->nextEntry);
			return false;
		}
		if (!_readIndexEntry(ie)) {  // read the next index entry
			return false;
		}
	}
	return true;
}

boolean PStorage::_searchIndexEntry(const char* name, PStorageIndexEntry *ie) {
	PSTORAGE_DEBUG("_searchIndexEntry(): Called");

	if (!_readFirstIndexEntry(ie)) {
		return false;
	}
	while ( strcasecmp(name, ie->name) != 0) {
		if (_isLastIndexEntry(*ie)) {  // we have reached the last entry without match
			return false;
		}
		if (!_storageFile.seek(ie->nextEntry, SeekSet)) {
			PSTORAGE_DEBUG("_searchIndexEntry(): Corruption, could not set %d position", ie->nextEntry);
			return false;
		}
		if (!_readIndexEntry(ie)) {  // read the next index entry
			return false;
		}
	}
	return true;
}

boolean PStorage::_searchFreeIndexEntry(unsigned int minSize, PStorageIndexEntry *ie) {
	PSTORAGE_DEBUG("_searchFreeIndexEntry(): Called");

	PStorageIndexEntry currentEntry;
	if (!_readFirstIndexEntry(&currentEntry)) {
		return false;
	}
	boolean found = false, terminate = false;
	do {
		if (
				(!found && (currentEntry.type == P_FREE) && (_size(currentEntry) >= minSize)) ||
				(found && (currentEntry.type == P_FREE) && (_size(currentEntry) >= minSize) && (_size(currentEntry) < _size(*ie)))
				// fit or even better fit than the previously found
		) {
			found = true;
			strcpy(ie->name, currentEntry.name);
			ie->nextEntry = currentEntry.nextEntry;
			ie->previousEntry = currentEntry.previousEntry;
			ie->thisEntry = currentEntry.thisEntry;
			ie->type = currentEntry.type;
		}
		if (!_isLastIndexEntry(currentEntry)) {
			if (!_storageFile.seek(currentEntry.nextEntry, SeekSet)) {
				PSTORAGE_DEBUG("_searchFreeIndexEntry(): Corruption, could not set %d position", ie->nextEntry);
				return false;
			}
			if (!_readIndexEntry(&currentEntry)) {
				return false;
			}
		} else {
			terminate = true;
		}
	} while (!terminate);
	return found;
}

boolean PStorage::_writeEntry(const PStorageIndexEntry ie, byte* buf, unsigned int maxBytes) {
	PSTORAGE_DEBUG("_writeEntry(): Called");

	unsigned int writePosition = ie.thisEntry + sizeof(PStorageIndexEntry);
	if (!_storageFile.seek(writePosition, SeekSet)) {
		PSTORAGE_DEBUG("_writeEntry(): Could not set position %d", writePosition);
		return false;
	}
	byte *ptr = buf;
	for (unsigned int i = 0; i < min(_size(ie), maxBytes); i++) {
		if (_storageFile.write(*(ptr + i)) != 1) {
			PSTORAGE_DEBUG("_writeEntry(): Could not write at position %d", _storageFile.position());
			return false;
		}
	}
	_storageFile.flush();
	return true;
}

int PStorage::_readEntry(const PStorageIndexEntry ie, byte* buf, unsigned int maxBytes) {
	PSTORAGE_DEBUG("_readEntry(): Called");

	unsigned int readPosition = ie.thisEntry + sizeof(PStorageIndexEntry);
	if (!_storageFile.seek(readPosition, SeekSet)) {
		PSTORAGE_DEBUG("_readEntry(): Could not set position %d", readPosition);
		return -1;
	}
	byte *ptr = buf;
	for (unsigned int i = 0; i < min(_size(ie), maxBytes); i++) {
		if (_storageFile.read(ptr + i, 1) != 1) {
			PSTORAGE_DEBUG("_readEntry(): Could not read value at position %d", _storageFile.position());
			return -1;
		}
	}
	return min(_size(ie), maxBytes);
}

const char* PStorage::_getStorageFileName() {
	PSTORAGE_DEBUG("_getStorageFileName(): Called");

	String result = "/pstorage/" + String(_name) + ".psf"
			"";
	return result.c_str();
}

String PStorage::_printType(EntryType type) {
	switch (type) {
	case P_FREE: return "FREE"; break;
	case P_INT: return "INT"; break;
	case P_UINT: return "UNSIGNED INT"; break;
	case P_LONG: return "LONG"; break;
	case P_ULONG: return "UNSIGNED LONG"; break;
	case P_FLOAT: return "FLOAT"; break;
	case P_ARRAY: return "ARRAY"; break;
	case P_STRING: return "STRING"; break;
	default: return "UNKNOWN"; break;
	}
}

void PStorage::_printFree() {
	Serial.printf("Free");
}

void PStorage::_printInt(PStorageIndexEntry ie) {
	byte b[sizeof(int)];
	_readEntry(ie, b, sizeof(int));
	Serial.printf("%d", (int) *b);
}

void PStorage::_printUInt(PStorageIndexEntry ie) {
	byte b[sizeof(unsigned int)];
	_readEntry(ie, b, sizeof(unsigned int));
	Serial.printf("%u", (unsigned int) *b);
}

void PStorage::_printLong(PStorageIndexEntry ie) {
	byte b[sizeof(long)];
	_readEntry(ie, b, sizeof(long));
	Serial.printf("%ld", (long) *b);
}

void PStorage::_printULong(PStorageIndexEntry ie) {
	byte b[sizeof(unsigned long)];
	_readEntry(ie, b, sizeof(unsigned long));
	Serial.printf("%lu", (unsigned long) *b);
}


void PStorage::_printFloat(PStorageIndexEntry ie) {
	byte b[sizeof(float)];
	_readEntry(ie, b, sizeof(float));
	Serial.printf("%f", (float) *b);
}

void PStorage::_printString(PStorageIndexEntry ie) {
	char b[256];
	int bytesRead = _readEntry(ie, (byte *) b, 255);
	b[bytesRead] = '\0';
	Serial.printf("%s", b);
}

void PStorage::_printArray(PStorageIndexEntry ie) {
	byte b[256];
	_readEntry(ie, b, 256);
	for (int i = 0; i < 256; i++) {
		Serial.printf("%X", b[i]);
	}
}

void PStorage::_printDefault() {
	Serial.printf("Unknown");
}


void PStorage::_printEntry(PStorageIndexEntry ie) {
	switch (ie.type) {
	case P_FREE: _printFree(); break;
	case P_INT: _printInt(ie); break;
	case P_UINT: _printUInt(ie); break;
	case P_LONG: _printLong(ie); break;
	case P_ULONG: _printULong(ie); break;
	case P_FLOAT: _printFloat(ie); break;
	case P_ARRAY: _printArray(ie); break;
	case P_STRING: _printString(ie); break;
	default: _printDefault(); break;
	}
}

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
