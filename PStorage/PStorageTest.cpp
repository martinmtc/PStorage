/*
 * PStorageTest.cpp
 *
 *  Created on: 10.06.2018
 *      Author: Dr. Martin Schaaf
 */

#include "PStorageTest.h"


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


const char 	*str1Name = "S1", *str1Val = "Ich hei�e Martin",
		*str2Name = "S2", *str2Val = "Zu Dionys dem Tyrannen schlich Damon den Dolch im Gewande, ihn schlugen die H�scher in Bande. Was wolltest Du mit dem Dolche sprich, entgegnet ihm finster der W�terich. Die Stadt vom Tyrannen befreien. Das sollst Du am Kreuze bereuen. Ich bin spricht jener zu sterben bereit und flehe nicht um mein Leben, doch willst Du Gnade mit geben, so bitte ich Dich um drei Tage Zeit";

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

boolean pStorageTestRead() {
	boolean totalResult = true, testResult = true;
	char strBuf[PSTORAGE_ENTRY_MAXSIZE];

	if ((testResult = pStorage.readString(str1Name, strBuf, PSTORAGE_ENTRY_MAXSIZE))) {
		pStorageTestSuccess("Read", "\"%s\" with value \"%s\"", str1Name, strBuf);
	} else {
		pStorageTestFailure("Read", "\"%s\"", str1Name);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage.readString(str2Name, strBuf, PSTORAGE_ENTRY_MAXSIZE))) {
		pStorageTestSuccess("Read", "\"%s\" with value \"%s\"", str2Name, strBuf);
	} else {
		pStorageTestFailure("Read", "\"%s\"", str2Name);
	}
	totalResult = totalResult && testResult;

	int val;
	if ((testResult = pStorage.readInt(int1Name, &val))) {
		pStorageTestSuccess("Read", "\"%s\" with value \"%d\"", int1Name, val);
	} else {
		pStorageTestFailure("Read", "\"%s\"", int1Name);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage.readInt(int2Name, &val))) {
		pStorageTestSuccess("Read", "\"%s\" with value \"%d\"", int2Name, val);
	} else {
		pStorageTestFailure("Read", "\"%s\"", int2Name);
	}
	totalResult = totalResult && testResult;

	// now we try to read something not existent
	if (!(testResult = pStorage.readInt("Not known", &val))) {
		pStorageTestSuccess("Read", "\"Not known\" not readable");
	} else {
		pStorageTestFailure("Read", "\"Not known\" readable with value %d", val);
	}
	totalResult = totalResult && !testResult;

	// try to read something with the wrong type
	if (!(testResult = pStorage.readInt(str2Name, &val))) {
		pStorageTestSuccess("Read", "Reading \"%s\" as type INT correctly failed", str2Name);
	} else {
		pStorageTestFailure("Read", "Reading \"%s\" as type INT should fail", str2Name);
	}
	totalResult = totalResult && !testResult;

	return totalResult;
}

boolean pStorageTestWrite() {
	boolean totalResult = true, testResult = true;
	if ((testResult = pStorage.writeString(str1Name, str1Val))) {
		pStorageTestSuccess("Write", "\"%s\" with value \"%s\"", str1Name, str1Val);
	} else {
		pStorageTestFailure("Write", "\"%s\" with value \"%s\"", str1Name, str1Val);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage.writeString(str2Name, str2Val))) {
		pStorageTestSuccess("Write", "\"%s\" with value \"%s\"", str2Name, str2Val);
	} else {
		pStorageTestFailure("Write", "\"%s\" with value \"%s\"", str2Name, str2Val);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage.writeInt(int1Name, int1Val))) {
		pStorageTestSuccess("Write", "\"%s\" with value %d", int1Name, int1Val);
	} else {
		pStorageTestFailure("Write", "\"%s\" with value %d", int1Name, int1Val);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage.writeInt(int2Name, int2Val))) {
		pStorageTestSuccess("Write", "\"%s\" with value %d", int2Name, int2Val);
	} else {
		pStorageTestFailure("Write", "\"%s\" with value %d", int2Name, int2Val);
	}
	totalResult = totalResult && testResult;

	// now we overwrite switch the Ints

	if ((testResult = pStorage.writeInt(int1Name, int2Val))) {
		pStorageTestSuccess("Write", "\"%s\" with value %d", int1Name, int2Val);
	} else {
		pStorageTestFailure("Write", "\"%s\" with value %d", int1Name, int2Val);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage.writeInt(int2Name, int1Val))) {
		pStorageTestSuccess("Write", "\"%s\" with value %d", int2Name, int1Val);
	} else {
		pStorageTestFailure("Write", "\"%s\" with value %d", int2Name, int1Val);
	}
	totalResult = totalResult && testResult;

	// now write something not initialized before
	if (!(testResult = pStorage.writeInt("Not known", int1Val))) {
		pStorageTestSuccess("Write", "\"Not known\" not created");
	} else {
		pStorageTestFailure("Write", "\"Not known\" created");
	}
	totalResult = totalResult && !testResult;

	// now write something exceeding the previously requested size
	if (!(testResult = pStorage.writeString(str1Name, str2Val))) {
		pStorageTestSuccess("Write", "\"%s\" with value \"%s\" correctly rejected, exceeding previous max", str1Name, str2Val);
	} else {
		pStorageTestFailure("Write", "\"%s\" with value \"%s\"", str1Name, str2Val);
	}
	totalResult = totalResult && !testResult;

	// now write something that exceeds MAX
	if (!(testResult = pStorage.writeString(str2Name, illegalValue))) {
		pStorageTestSuccess("Write", "\"%s\" with value \"%s\" correctly rejected, exceeding max val", str2Name, illegalValue);
	} else {
		pStorageTestFailure("Write", "\"%s\" with value \"%s\"", str2Name, illegalValue);
	}
	totalResult = totalResult && !testResult;

	return totalResult;
}

boolean pStorageTestCreate() {
	boolean totalResult = true, testResult = true;
	if ((testResult = pStorage.createString(str1Name, strlen(str1Val)))) {
		pStorageTestSuccess("Create", "\"%s\" of max length %u", str1Name, strlen(str1Val));
	} else {
		pStorageTestFailure("Create", "\"%s\" of max length %u", str1Name, strlen(str1Val));
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage.createString(str2Name, strlen(str2Val)))) {
		pStorageTestSuccess("Create", "\"%s\" of max length %u", str2Name, strlen(str2Val));
	} else {
		pStorageTestFailure("Create", "\"%s\" of max length %u", str2Name, strlen(str2Val));
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage.createInt(int1Name))) {
		pStorageTestSuccess("Create", " Integer with name \"%s\"", int1Name);
	} else {
		pStorageTestFailure("Create", " Integer with name \"%s\"", int1Name);
	}
	totalResult = totalResult && testResult;

	if ((testResult = pStorage.createInt(int2Name))) {
		pStorageTestSuccess("Create", " Integer with name \"%s\"", int2Name);
	} else {
		pStorageTestFailure("Create", " Integer with name \"%s\"", int2Name);
	}
	totalResult = totalResult && testResult;

	// now we create an already existing variable
	if (!(testResult = pStorage.createString(str2Name, strlen(str2Val)))) {
		pStorageTestSuccess("Create", "\"%s\" of max length %u has been prevented", str2Name, strlen(str2Val));
	} else {
		pStorageTestFailure("Create", "\"%s\" of max length %u has been created multiple times", str2Name, strlen(str2Val));
	}
	totalResult = totalResult && !testResult;

	// now we created an illegally name variable
	if (!(testResult = pStorage.createString(illegalName, 1))) {
		pStorageTestSuccess("Create", "exceeds length \"%s\"", illegalName);
	} else {
		pStorageTestFailure("Create", "\"%s\" has been created despite exceeding length", illegalName);
	}
	totalResult = totalResult && !testResult;

	// and finally we try to create a variable with exceeding value
	if (!(testResult = pStorage.createString("Too Long", strlen(illegalValue)))) {
		pStorageTestSuccess("Create", "exceeds value max size\"%s\"", illegalValue);
	} else {
		pStorageTestFailure("Create", "\"%s\" has been created despite exceeding value max size", illegalValue);
	}
	totalResult = totalResult && !testResult;

	return totalResult;
}

boolean testsCompleted = false;

void pStorageTest() {
	if (!(ESP.getResetReason().equals("Deep-Sleep Wake"))) {
		if (testsCompleted) return;
		pStorageTestSetup();
		Serial.println("Create Test start ----------------------------------------------------------------------");
		if (pStorageTestCreate()) {
			Serial.println("Create Tests successfully completed");
		} else {
			Serial.println("Create Tests failed");
		}
		Serial.println("Write Test start -----------------------------------------------------------------------");
		if (pStorageTestWrite()) {
			Serial.println("Write Tests successfully completed");
		} else {
			Serial.println("Write Tests failed");
		}
		Serial.println("Read Test start ------------------------------------------------------------------------");
		if (pStorageTestRead()) {
			Serial.println("Read Tests successfully completed");
		} else {
			Serial.println("Read Tests failed");
		}
		testsCompleted = true;
	} else {
		if (testsCompleted) return;
		Serial.println("Read Test after Deep Sleep started ------------------------------------------------------");
		pStorageTestRead();
		testsCompleted = true;
	}
}





