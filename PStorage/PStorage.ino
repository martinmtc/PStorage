#define PSTORAGE_TEST_ENABLED 		true

#include "Arduino.h"
#include "PStorageTest.h"


void setup()
{
	Serial.begin(115200);
	delay(1000);
	SPIFFS.begin();

	// we create 2 PStoraged, one that will be recreated each time, the other will be persisted

	PStorage pNew("New");
	PStorage pPer("Persistent");

	int newC1 = 0, perC1 = 0, perC2, perC3, perC4;

	pNew.create(512); //
	if (!pPer.open()) {
		pPer.create(512);
		pPer.map("c1", perC1);
	}

	pNew.dumpPStorage();
	pPer.dumpPStorage();

	pNew.map("c1", (int) 1);
	pPer.get("c1", &perC1);
	pPer.map("c1", perC1 + 1);
	pNew.dumpPStorage();
	pPer.dumpPStorage();


	pPer.map("c2", (int) 2);
	pPer.map("c3", (int) 3);
	pPer.map("c4", (int) 4);
	pPer.dumpPStorage();




	pNew.get("c1", &newC1);
	pPer.get("c1", &perC1);
	pPer.get("c2", &perC2);
	pPer.get("c3", &perC3);
	pPer.get("c4", &perC4);

	Serial.println("New C1: " + String(newC1));
	Serial.println("Per C1: " + String(perC1));
	Serial.println("Per C2: " + String(perC2));
	Serial.println("Per C3: " + String(perC3));
	Serial.println("Per C4: " + String(perC4));

	pPer.remove("c3");
	pPer.dumpPStorage();

	pPer.map("c5", (int) 5);
	pPer.dumpPStorage();

	pPer.remove("c5");
	pPer.dumpPStorage();

	Serial.println("Remove C4");
	pPer.remove("c4");
	Serial.println("Remove C4, done");
	pPer.dumpPStorage();

	pPer.map("S1", "Ich heiﬂe Martin Schaaf");
	pPer.dumpPStorage();

	pPer.map("S1", "Und ich heiﬂe immer noch Martin Schaaf");
	pPer.dumpPStorage();

	pPer.remove("S1");
	pPer.dumpPStorage();

}



void loop()
{
	//Add your repeated code here
}
