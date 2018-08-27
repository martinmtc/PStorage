#define PSTORAGE_TEST_ENABLED 		true

#include "Arduino.h"
#include "PStorageTest.h"


void setup()
{
	Serial.begin(115200);
	delay(1000);
	SPIFFS.begin();
	pStorage.init();
	pStorageTest();
	pStorage.end();
}



void loop()
{
	//Add your repeated code here
}
