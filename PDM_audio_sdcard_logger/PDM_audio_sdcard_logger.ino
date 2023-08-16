/*
* The circuit:
* 1 lcd gnd
* 2 lcd vdd 5v
* 3 V0 contrast gnd
* 4 LCD RS pin to D10
* 5 lcd RW pin to gnd
* 6 LCD E (Enable) pin D9
* 11 LCD D4 pin to D8
* 12 LCD D5 pin to D7
* 13 LCD D6 pin to D6
* 14 LCD D7 pin to D5
* 15 lcd LED+ to vcc with 220ohm resistor
* 16 lcd LED- to gnd

*/

#include <PDM.h> // microphone library
#include <SD.h> // sd card library
#include <LiquidCrystal.h> // lcd display library

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = D10, en = D9, d4 = D8, d5 = D7, d6 = D6, d7 = D5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// default number of output channels
static const char channels = 1;

// default PCM output frequency
static const int frequency = 16000;

// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[512];

// Number of bytes of audio samples read
int bytesRead;

// CS pin for SD card using SPI protocol
const int chipSelect = D2;

// start time for recording
long start = 0;

// file pointer where to write binary data
File myFile;

const int ledPin = D4, buttonPin = D3;
int buttonState = 0;


void setup() {
	
	pinMode(ledPin, OUTPUT); // initialize the LED pin as an output:
	pinMode(buttonPin, INPUT); // initialize the pushbutton pin as an input:

	// set up the LCD's number of columns and rows:
	lcd.begin(16, 2);
	lcd.display();
	// Print a message to the LCD.
	lcd.print("hello, world!");
	analogWrite(A1, 128);
	

	Serial.begin(9600);

	// initialize SD card
	Serial.print("Initializing SD card...");
	if (!SD.begin(chipSelect)) { 
		Serial.println("initialization failed!");
		return;
	}
	Serial.println("initialization done.");
	lcd.setCursor(0, 0); // first row
	lcd.print("initialized");
	
	// Configure the data receive callback
	PDM.onReceive(onPDMdata);

	// Optionally set the gain, defaults to 20 on the BLE Sense
	PDM.setGain(40);

	// Initialize PDM with:
	// - one channel (mono mode)
	// - a 16 kHz sample rate for the Arduino Nano 33 BLE Sense
	if (!PDM.begin(channels, frequency)) {
		Serial.println("Failed to start PDM!");
		while (1);
	} else {
		Serial.println("started PDM");
	}
}

void loop() {

	// read the state of the pushbutton value:
 	buttonState = digitalRead(buttonPin);

	if (buttonState) { // button pressed
		// open the file, if it doesn't exists, it creates a new file
		// note that the length of the file cannot exceed 8 characters
		// and the extension of the file cannot exceed 8 characters
		String file_name = newName();
		myFile = SD.open(file_name, FILE_WRITE);

		if (!myFile) {
			Serial.println("error creating file");
		}

		digitalWrite(ledPin, HIGH);

		lcd.setCursor(0, 0); // first row
		lcd.print("recording ...");

		lcd.setCursor(0, 1); // second row

		// Wait for samples to be read
		long time, start = millis();
		while (millis() - start < 60000) {
			// print the number of seconds since reset:
			time = (millis() - start) /1000;
			lcd.setCursor(0, 1);
			lcd.print(time);

			if (bytesRead) {
				//Serial.write((byte *) sampleBuffer, bytesRead);
				myFile.write((byte *) sampleBuffer, bytesRead);

				// Clear the read count
				bytesRead = 0;
			}
		}
		myFile.close();
		Serial.println("closed file");

		lcd.setCursor(0, 0);
		lcd.clear();
		lcd.print("saved rec_" + file_name.substring(4, 7));

		//while(1) ;

	} else { // button not pressed
		digitalWrite(ledPin, LOW);
	}

	
}

String newName() {

	File root = SD.open("/");
	int maxFileNumber = -1;

	// Cerca il file con il numero più alto
	while (true) {
		File entry = root.openNextFile();
		if (!entry) {
			break; // Nessun altro file
		}
		if (!entry.isDirectory()) {
			String fileName = entry.name();
			if (fileName.startsWith("REC_") && fileName.endsWith(".BIN")) {
				int fileNumber = fileName.substring(4, 7).toInt(); // Estrae il numero dal nome del file
				if (fileNumber > maxFileNumber) {
					maxFileNumber = fileNumber;
				}
			}
		}

		entry.close();
	}
	root.close();

	// Incrementa il numero del file più alto
	int newFileNumber = maxFileNumber + 1;

	// Crea il nuovo nome del file
	//String newFileName = "rec_" + String(newFileNumber) + ".bin";
	char * new_name = (char *) malloc(12);
	snprintf(new_name, 12, "rec_%03d.bin", newFileNumber);
	Serial.println(String (new_name));
	return String(new_name);
}

/**
 * Callback function to process the data from the PDM microphone.
 * NOTE: This callback is executed as part of an ISR.
 * Therefore using `Serial` to print messages inside this function isn't supported.
 * */
void onPDMdata() {
	// Query the number of available bytes
	bytesRead = PDM.available();

	// Read into the sample buffer
	// 16-bit, 2 bytes per sample
	PDM.read(sampleBuffer, bytesRead);
}