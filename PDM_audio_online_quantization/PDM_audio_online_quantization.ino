/*
  This example reads audio data from the on-board PDM microphones, and prints
  out the samples to the Serial console. The Serial Plotter built into the
  Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)

  Circuit:
  - Arduino Nano 33 BLE board, or
  - Arduino Nano RP2040 Connect, or
  - Arduino Portenta H7 board plus Portenta Vision Shield, or
  - Arduino Nicla Vision

  This example code is in the public domain.
*/

#include <PDM.h>

// default number of output channels
static const char channels = 1;

// default PCM output frequency
static const int frequency = 16000;

// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[512];
int8_t bytesBuffer[512];

// Number of audio samples read
int bytesRead;


void setup() {
	Serial.begin(9600);

	// Configure the data receive callback
	PDM.onReceive(onPDMdata);

	// Optionally set the gain
	// Defaults to 20 on the BLE Sense and 24 on the Portenta Vision Shield
	PDM.setGain(40);

	// Initialize PDM with:
	// - one channel (mono mode)
	// - a 16 kHz sample rate for the Arduino Nano 33 BLE Sense
	// - a 32 kHz or 64 kHz sample rate for the Arduino Portenta Vision Shield
	if (!PDM.begin(channels, frequency)) {
		Serial.println("Failed to start PDM!");
		while (1);
	}
}

void loop() {
	// Wait for samples to be read
	if (bytesRead) {
		Serial.write((byte *) bytesBuffer, bytesRead);
		// Clear the read count
		bytesRead = 0;
	}
}

/**
 * Callback function to process the data from the PDM microphone.
 * NOTE: This callback is executed as part of an ISR (interrupt service routine).
 * Therefore using `Serial` to print messages inside this function isn't supported.
 * */
void onPDMdata() {
	// Query the number of available bytes
	bytesRead = PDM.available();

	// Read into the sample buffer
	// 16-bit, 2 bytes per sample
	PDM.read(sampleBuffer, bytesRead);

	float scale = 65535.0;
	bytesRead = bytesRead >> 1;
	for (int i = 0; i < bytesRead; i+=1) {
		bytesBuffer[i] = (int8_t) ((float) (sampleBuffer[i]) * (256.0 / scale));
	}
}
