#include <SD.h>

File myFile;

// from left to right in the sd card reader
// gnd
// vcc
// SPI MISO pin D12 
// SPI MOSI pin D11
// SPI SCK pin D13
// SPI CS pin D2 (digital gpio)
const int chipSelect = D2;

void setup() {
	// Open serial communications and wait for port to open:
	Serial.begin(9600);
	delay(5000);
	Serial.print("Initializing SD card...");

	if (!SD.begin(chipSelect)) {
		Serial.println("initialization failed!");
		return;
	}
	Serial.println("initialization done.");

	// open the file. note that only one file can be open at a time,
	// so you have to close this one before opening another.
	myFile = SD.open("test.txt", FILE_WRITE);

	// if the file opened okay, write to it:
	if (myFile) {
		Serial.print("Writing to test.txt...");
		myFile.println("testing 1, 2, 3.");
		// close the file:
		myFile.close();
		Serial.println("done.");
	} else {
		// if the file didn't open, print an error:
		Serial.println("error opening test.txt");
	}

	// re-open the file for reading:
	myFile = SD.open("test.txt");
	if (myFile) {
		Serial.println("test.txt:");

		// read from the file until there's nothing else in it:
		while (myFile.available()) {
			Serial.write(myFile.read());
		}
		// close the file:
		myFile.close();
	} else {
		// if the file didn't open, print an error:
		Serial.println("error opening test.txt");
	}
}

void loop()
{
  // nothing happens after setup
}