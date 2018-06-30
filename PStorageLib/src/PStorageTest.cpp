/*
 * PStorageTest.cpp
 *
 *  Created on: 10.06.2018
 *      Author: Dr. Martin Schaaf
 */


#include "Arduino.h"
#include "PStorageTest.h"

#if(PSTORAGE_TEST_ENABLED)

void _pStorageTestResult(boolean result, const char *testSuite , const char *format, va_list argList) {
	char logBuffer[256];
	vsnprintf(logBuffer, sizeof(logBuffer), format, argList);
	Serial.println((result? "SUCCESS \t\t" : "FAILURE \t\t") + String(testSuite) + ": " + String(logBuffer));
}

void pStorageTestSuccess(const char *testSuite, const char *format, ...) {
	va_list argList;
	va_start(argList, format);
	_pStorageTestResult(true, testSuite, format, argList);
	va_end(argList);
}


void pStorageTestFailure(const char *testSuite, const char *format, ...) {
	va_list argList;
	va_start(argList, format);
	_pStorageTestResult(false, testSuite, format, argList);
	va_end(argList);
}


const char 	*str1Name = "S1", *str1Val = "Ich heiße Martin",
		*str2Name = "S2", *str2Val = "Zu Dionys dem Tyrannen schlich Damon den Dolch im Gewande, ihn schlugen die Häscher in Bande. Was wolltest Du mit dem Dolche sprich, entgegnet ihm finster der Wüterich. Die Stadt vom Tyrannen befreien. Das sollst Du am Kreuze bereuen. Ich bin spricht jener zu sterben bereit und flehe nicht um mein Leben, doch willst Du Gnade mit geben, so bitte ich Dich um drei Tage Zeit";

const char 	*int1Name = "I1", *int2Name = "I2";
int 		int1Val = 123, int2Val = -456;

char	illegalName[PSTORAGE_ENTRY_NAME_MAXSIZE + 2];
char	illegalValue[PSTORAGE_ENTRY_VALUE_MAXSIZE + 2];

void createIllegalName() {
	for (int i = 0; i < PSTORAGE_ENTRY_NAME_MAXSIZE + 1; i++) {
		illegalName[i] = (i % ('Z' - 'A' + 1)) + 'A';
	}
	illegalName[PSTORAGE_ENTRY_NAME_MAXSIZE+1] = '\0';
}

void createIllegalValue() {
	for (int i = 0; i < PSTORAGE_ENTRY_VALUE_MAXSIZE + 1; i++) {
		illegalValue[i] = (i % ('Z' - 'A' + 1)) + 'A';
	}
	illegalValue[PSTORAGE_ENTRY_VALUE_MAXSIZE+1] = '\0';
}

void pStorageTestSetup() {
	createIllegalName();
	createIllegalValue();
}

boolean pStorageTestRead(PStorage *pStorage) {
	boolean totalResult = true, testResult = true;
	char strBuf[PSTORAGE_ENTRY_MAXSIZE];

	if ((testResult = pStorage->readString(str1Name, strBuf, PSTORAGE_ENTRY_MAXSIZE))) {
		PSTORAGE_TEST_SUCC("Read", "\"%s\" with value \"%s\"", str1Name, strBuf);
	} else {
		PSTORAGE_TEST_FAIL("Read", "\"%s\"", str1Name);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage->readString(str2Name, strBuf, PSTORAGE_ENTRY_MAXSIZE))) {
		PSTORAGE_TEST_SUCC("Read", "\"%s\" with value \"%s\"", str2Name, strBuf);
	} else {
		PSTORAGE_TEST_FAIL("Read", "\"%s\"", str2Name);
	}
	totalResult = totalResult && testResult;

	int val;
	if ((testResult = pStorage->readInt(int1Name, &val))) {
		PSTORAGE_TEST_SUCC("Read", "\"%s\" with value \"%d\"", int1Name, val);
	} else {
		PSTORAGE_TEST_FAIL("Read", "\"%s\"", int1Name);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage->readInt(int2Name, &val))) {
		PSTORAGE_TEST_SUCC("Read", "\"%s\" with value \"%d\"", int2Name, val);
	} else {
		PSTORAGE_TEST_FAIL("Read", "\"%s\"", int2Name);
	}
	totalResult = totalResult && testResult;

	// now we try to read something not existent
	if (!(testResult = pStorage->readInt("Not known", &val))) {
		PSTORAGE_TEST_SUCC("Read", "\"Not known\" not readable");
	} else {
		PSTORAGE_TEST_FAIL("Read", "\"Not known\" readable with value %d", val);
	}
	totalResult = totalResult && !testResult;

	// try to read something with the wrong type
	if (!(testResult = pStorage->readInt(str2Name, &val))) {
		PSTORAGE_TEST_SUCC("Read", "Reading \"%s\" as type INT correctly failed", str2Name);
	} else {
		PSTORAGE_TEST_FAIL("Read", "Reading \"%s\" as type INT should fail", str2Name);
	}
	totalResult = totalResult && !testResult;

	return totalResult;
}

boolean pStorageTestWrite(PStorage *pStorage) {
	boolean totalResult = true, testResult = true;
	if ((testResult = pStorage->writeString(str1Name, str1Val))) {
		PSTORAGE_TEST_SUCC("Write", "\"%s\" with value \"%s\"", str1Name, str1Val);
	} else {
		PSTORAGE_TEST_FAIL("Write", "\"%s\" with value \"%s\"", str1Name, str1Val);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage->writeString(str2Name, str2Val))) {
		PSTORAGE_TEST_SUCC("Write", "\"%s\" with value \"%s\"", str2Name, str2Val);
	} else {
		PSTORAGE_TEST_FAIL("Write", "\"%s\" with value \"%s\"", str2Name, str2Val);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage->writeInt(int1Name, int1Val))) {
		PSTORAGE_TEST_SUCC("Write", "\"%s\" with value %d", int1Name, int1Val);
	} else {
		PSTORAGE_TEST_FAIL("Write", "\"%s\" with value %d", int1Name, int1Val);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage->writeInt(int2Name, int2Val))) {
		PSTORAGE_TEST_SUCC("Write", "\"%s\" with value %d", int2Name, int2Val);
	} else {
		PSTORAGE_TEST_FAIL("Write", "\"%s\" with value %d", int2Name, int2Val);
	}
	totalResult = totalResult && testResult;

	// now we overwrite switch the Ints

	if ((testResult = pStorage->writeInt(int1Name, int2Val))) {
		PSTORAGE_TEST_SUCC("Write", "\"%s\" with value %d", int1Name, int2Val);
	} else {
		PSTORAGE_TEST_FAIL("Write", "\"%s\" with value %d", int1Name, int2Val);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage->writeInt(int2Name, int1Val))) {
		PSTORAGE_TEST_SUCC("Write", "\"%s\" with value %d", int2Name, int1Val);
	} else {
		PSTORAGE_TEST_FAIL("Write", "\"%s\" with value %d", int2Name, int1Val);
	}
	totalResult = totalResult && testResult;

	// now write something not initialized before
	if (!(testResult = pStorage->writeInt("Not known", int1Val))) {
		PSTORAGE_TEST_SUCC("Write", "\"Not known\" not created");
	} else {
		PSTORAGE_TEST_FAIL("Write", "\"Not known\" created");
	}
	totalResult = totalResult && !testResult;

	// now write something exceeding the previously requested size
	if (!(testResult = pStorage->writeString(str1Name, str2Val))) {
		PSTORAGE_TEST_SUCC("Write", "\"%s\" with value \"%s\" correctly rejected, exceeding previous max", str1Name, str2Val);
	} else {
		PSTORAGE_TEST_FAIL("Write", "\"%s\" with value \"%s\"", str1Name, str2Val);
	}
	totalResult = totalResult && !testResult;

	// now write something that exceeds MAX
	if (!(testResult = pStorage->writeString(str2Name, illegalValue))) {
		PSTORAGE_TEST_SUCC("Write", "\"%s\" with value \"%s\" correctly rejected, exceeding max val", str2Name, illegalValue);
	} else {
		PSTORAGE_TEST_FAIL("Write", "\"%s\" with value \"%s\"", str2Name, illegalValue);
	}
	totalResult = totalResult && !testResult;

	return totalResult;
}

boolean pStorageTestCreate(PStorage *pStorage) {
	boolean totalResult = true, testResult = true;
	if ((testResult = pStorage->createString(str1Name, strlen(str1Val)))) {
		PSTORAGE_TEST_SUCC("Create", "\"%s\" of max length %u", str1Name, strlen(str1Val));
	} else {
		PSTORAGE_TEST_FAIL("Create", "\"%s\" of max length %u", str1Name, strlen(str1Val));
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage->createString(str2Name, strlen(str2Val)))) {
		PSTORAGE_TEST_SUCC("Create", "\"%s\" of max length %u", str2Name, strlen(str2Val));
	} else {
		PSTORAGE_TEST_FAIL("Create", "\"%s\" of max length %u", str2Name, strlen(str2Val));
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage->createInt(int1Name))) {
		PSTORAGE_TEST_SUCC("Create", " Integer with name \"%s\"", int1Name);
	} else {
		PSTORAGE_TEST_FAIL("Create", " Integer with name \"%s\"", int1Name);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage->createInt(int2Name))) {
		PSTORAGE_TEST_SUCC("Create", " Integer with name \"%s\"", int2Name);
	} else {
		PSTORAGE_TEST_FAIL("Create", " Integer with name \"%s\"", int2Name);
	}
	totalResult = totalResult && testResult;

	// now we create an already existing variable
	if (!(testResult = pStorage->createString(str2Name, strlen(str2Val)))) {
		PSTORAGE_TEST_SUCC("Create", "\"%s\" of max length %u has been prevented", str2Name, strlen(str2Val));
	} else {
		PSTORAGE_TEST_FAIL("Create", "\"%s\" of max length %u has been created multiple times", str2Name, strlen(str2Val));
	}
	totalResult = totalResult && !testResult;

	// now we created an illegally name variable
	if (!(testResult = pStorage->createString(illegalName, 1))) {
		PSTORAGE_TEST_SUCC("Create", "\"%s\" exceeds length", illegalName);
	} else {
		PSTORAGE_TEST_FAIL("Create", "\"%s\" has been created despite exceeding length", illegalName);
	}
	totalResult = totalResult && !testResult;

	// and finally we try to create a variable with exceeding value
	if (!(testResult = pStorage->createString("Too Long", strlen(illegalValue)))) {
		PSTORAGE_TEST_SUCC("Create", "\"%s\" exceeds value max size", illegalValue);
	} else {
		PSTORAGE_TEST_FAIL("Create", "\"%s\" has been created despite exceeding value max size", illegalValue);
	}
	totalResult = totalResult && !testResult;

	return totalResult;
}

boolean testsCompleted = false;

void pStorageTest(PStorage *pStorage) {
	if (!(ESP.getResetReason().equals("Deep-Sleep Wake"))) {
		if (testsCompleted) return;
		pStorageTestSetup();
		Serial.println("Create Test start ----------------------------------------------------------------------");
		if (pStorageTestCreate(pStorage)) {
			Serial.println("Create Tests successfully completed");
		} else {
			Serial.println("Create Tests failed");
		}
		Serial.println("Write Test start -----------------------------------------------------------------------");
		if (pStorageTestWrite(pStorage)) {
			Serial.println("Write Tests successfully completed");
		} else {
			Serial.println("Write Tests failed");
		}
		Serial.println("Read Test start ------------------------------------------------------------------------");
		if (pStorageTestRead(pStorage)) {
			Serial.println("Read Tests successfully completed");
		} else {
			Serial.println("Read Tests failed");
		}
		testsCompleted = true;
	} else {
		if (testsCompleted) return;
		Serial.println("Read Test after Deep Sleep started ------------------------------------------------------");
		pStorageTestRead(pStorage);
		testsCompleted = true;
	}
}
#endif






