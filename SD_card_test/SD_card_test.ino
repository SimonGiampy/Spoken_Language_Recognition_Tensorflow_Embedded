#include <SD.h>


const int chipSelect = D2;


File root;


void setup() {
 // Open serial communications and wait for port to open:
	Serial.begin(9600);
	Serial.print("Initializing SD card...");

	if (!SD.begin(chipSelect)) {


		Serial.println("initialization failed. Things to check:");


		Serial.println("1. is a card inserted?");


		Serial.println("2. is your wiring correct?");


		Serial.println("3. did you change the chipSelect pin to match your shield or module?");


		Serial.println("Note: press reset or reopen this serial monitor after fixing your issue!");


		while (true);

	}


	Serial.println("initialization done.");
	root = SD.open("/");
	printDirectory(root, 0);
	Serial.println("done!");

}


void loop() {

}


void printDirectory(File dir, int numTabs) {
	while (true) {
		File entry =  dir.openNextFile();
		if (! entry) {
			break;
		}
		for (uint8_t i = 0; i < numTabs; i++) {
			Serial.print('\t');
		}
		Serial.print(entry.name());
		if (entry.isDirectory()) {
			Serial.println("/");
			printDirectory(entry, numTabs + 1);
		} else {
			Serial.print("\t\t");
			Serial.println(entry.size(), DEC);
		}
		entry.close();
	}

}