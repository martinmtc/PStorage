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







