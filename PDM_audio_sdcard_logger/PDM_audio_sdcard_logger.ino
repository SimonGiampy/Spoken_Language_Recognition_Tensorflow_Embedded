#include <PDM.h>
#include <SD.h>

// default number of output channels
static const char channels = 1;

// default PCM output frequency
static const int frequency = 16000;

// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[512];

// Number of audio samples read
int bytesRead;

const int chipSelect = D2;
long start = 0;
int i = 0;
File myFile;


void setup() {
	Serial.begin(9600);
	delay(5000);

	Serial.print("Initializing SD card...");

	if (!SD.begin(chipSelect)) {
		Serial.println("initialization failed!");
		return;
	}
	Serial.println("initialization done.");

	// open the file, if it doesn't exists, it creates a new file
	// note that the length of the file cannot exceed 8 characters
	// and the extension of the file cannot exceed 8 characters
	myFile = SD.open("rec00000.bin", FILE_WRITE);
	if (!myFile) {
		Serial.println("error creating file");
	}
	// Configure the data receive callback
	PDM.onReceive(onPDMdata);

	// Optionally set the gain
	// Defaults to 20 on the BLE Sense and 24 on the Portenta Vision Shield
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
	start = millis();
	i = 0;
}

void loop() {
	// Wait for samples to be read
	if (bytesRead && millis() - start < 5000) {
		//Serial.write((byte *) sampleBuffer, bytesRead);
		myFile.write((byte *) sampleBuffer, bytesRead);

		// Clear the read count
		bytesRead = 0;
		i++; 
	} else if (millis() - start >= 5000) {
		myFile.close();
		Serial.println("closed file");
		while(1) ;
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
