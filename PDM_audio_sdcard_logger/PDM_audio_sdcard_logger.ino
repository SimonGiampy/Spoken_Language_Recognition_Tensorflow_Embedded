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
int i = 0;

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
	/*
	for (int i = 0; i < 255; i ++) {
		analogWrite(A1, i);
		lcd.setCursor(0, 1);
		lcd.print(String(i));
		delay(100);
	}
	*/
	

	Serial.begin(9600);
	delay(5000);

	// initialize SD card
	Serial.print("Initializing SD card...");
	if (!SD.begin(chipSelect)) { 
		Serial.println("initialization failed!");
		return;
	}
	Serial.println("initialization done.");

	// open the file, if it doesn't exists, it creates a new file
	// note that the length of the file cannot exceed 8 characters
	// and the extension of the file cannot exceed 8 characters
	myFile = SD.open("rec_001.bin", FILE_WRITE);
	if (!myFile) {
		Serial.println("error creating file");
	}
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

	start = millis();
	i = 0;

	// read the state of the pushbutton value:
 	buttonState = digitalRead(buttonPin);

	if (buttonState) { // button pressed
		digitalWrite(ledPin, HIGH);

		lcd.setCursor(0, 0); // first row
		lcd.print("recording ...");

		lcd.setCursor(0, 1); // second row

		// Wait for samples to be read
		while (millis() - start < 5000) {
			// print the number of seconds since reset:
			lcd.print(millis() / 1000);

			if (bytesRead) {
				//Serial.write((byte *) sampleBuffer, bytesRead);
				myFile.write((byte *) sampleBuffer, bytesRead);

				// Clear the read count
				bytesRead = 0;
				i++; 
			}
		}
		myFile.close();
		Serial.println("closed file");

		lcd.setCursor(0, 0);
		lcd.print("stopped");

		//while(1) ;

	} else { // button not pressed
		digitalWrite(ledPin, LOW);
	}

	
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
