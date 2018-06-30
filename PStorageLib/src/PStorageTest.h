/*
 * PStorageTest.h
 *
 *  Created on: 10.06.2018
 *      Author: Dr. Martin Schaaf
 */

#ifndef PSTORAGETEST_H_
#define PSTORAGETEST_H_



#include "Arduino.h"
#include "PStorage.h"


#if(PSTORAGE_TEST_ENABLED)
#define PSTORAGE_TEST_SUCC(...) pStorageTestSuccess(__VA_ARGS__)
#define PSTORAGE_TEST_FAIL(...) pStorageTestFailure(__VA_ARGS__)
void pStorageTest(PStorage *pStorage);
#else
#define PSTORAGE_TEST_SUCC(...)
#define PSTORAGE_TEST_FAIL(...)
#endif


#endif /* PSTORAGETEST_H_ */
