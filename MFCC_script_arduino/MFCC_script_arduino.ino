
#include <PDM.h>
#include <Arduino.h>

#undef PI

#include "arduinoMFCC.h"

void onPDMdata(void);

// default number of output channels
static const char channels = 1;

// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[512];

// Number of audio samples read
int bytesRead;

unsigned int audio_index;

// MFCC parameters
const uint8_t num_filters = 40;
const uint16_t frame_size = 512;
const uint16_t hop_size = 128;
const uint8_t num_cepstral_coeffs = 12;
const uint16_t frequency = 16000;

// 10 seconds audio recording at 16kHz at 2 bytes/sample
const float seconds = 3.5;
const unsigned int length = frequency * seconds;
int16_t *audio_signal;


void setup() {

	Serial.begin(9600);
	
	audio_signal = new short[length];
	audio_index = 0;

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
	delay(5000);
	
	Serial.println("Start recording");

	while(1) {
		// Wait for samples to be read
		if (bytesRead && audio_index < length) {
			//Serial.write((byte *) sampleBuffer, bytesRead);

			for (int i = 0; i < bytesRead && audio_index < length; i++) {
				audio_signal[audio_index] = sampleBuffer[i];
				audio_index++;
			}

			// Clear the read count
			bytesRead = 0;
		} else if (bytesRead == 0) {
			delay(0.05);
		}
		if (audio_index == length) {
			break;
		}
	}
	
	Serial.println("Done");
	Serial.println(audio_index);

	if (audio_index == length) {
		arduinoMFCC *mymfcc = new arduinoMFCC(num_filters, frame_size, hop_size, length, num_cepstral_coeffs, frequency);
		int8_t **mfcc_coeffs = mymfcc->compute(audio_signal);
		
		for (int i = 0; i < mymfcc->getMatrixRows(); i++) {
			for (int j = 0; j < num_cepstral_coeffs; j++) {
				Serial.print(mfcc_coeffs[i][j]);
				Serial.print(" ");
			}
			Serial.println();
		}
		Serial.println();
	}
	while(1);
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
