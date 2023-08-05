#include "arduinoMFCC.h"

#include <iostream>
#include <fstream>

// MFCC parameters
const int num_channels = 12;
const int frame_size = 256;
const int hop_size = 128;
const int mfcc_size = 6;
const float sample_rate = 16000.0;

arduinoMFCC mymfcc(num_channels, frame_size, hop_size, mfcc_size, sample_rate);

// 1 minute audio recording at 16kHz at 2 bytes/sample
int16_t audio[960000];

void readBinary(const char *filePath);

void computeMelFilter(float **mel_filter, float **dct_matrix, float *hwindows, float *mfcc_coeffs);

int main() {
	// Specify the path to the audio file on the SD card
	const char *audioFilePath = "test_audio.bin";
	readBinary(audioFilePath);

	/*
	for(int i=0; i<960000; i++) {
		std::cout << "value" << i << " : " << audio[i] << std::endl;
	}
	*/

	float **mel_filter = (float **) malloc(num_channels * sizeof(float *));
	float **dct_matrix = (float **) malloc(mfcc_size * sizeof(float *));

	float *hwindows = (float *) malloc(frame_size * sizeof(float *));
	float *mfcc_coeffs = (float *) malloc(num_channels * sizeof(float *));
	float *rmfcc_coeffs = (float *) malloc(mfcc_size * sizeof(float *));

	computeMelFilter(mel_filter, dct_matrix, hwindows, mfcc_coeffs);

	for (int i = 0; i < num_channels; i++) {
		std::cout << mfcc_coeffs[i] << std::endl;
	}

	return 0;
}

void readBinary(const char *filePath) {

	// Open the binary file for binary input
	std::ifstream inputFile(filePath, std::ios::binary);

	// Check if the file was opened successfully
	if (!inputFile.is_open())
	{
		std::cerr << "Error: Unable to open the binary file." << std::endl;
	}
	else
	{
		int16_t valueInt16;
		int i = 0;
		while (inputFile.read(reinterpret_cast<char *>(&valueInt16), sizeof(valueInt16)))
		{
			audio[i] = valueInt16;
			i++;
		}
		// Close the file when done
		inputFile.close();
	}
}

void computeMelFilter(float **mel_filter, float **dct_matrix, float *hwindows, float *mfcc_coeffs) {
	// memory allocation
	for (int i = 0; i < num_channels; i++)
	{
		mel_filter[i] = (float *)malloc((frame_size / 2) * sizeof(float));
	}

	for (int i = 0; i < mfcc_size; i++)
	{
		dct_matrix[i] = (float *)malloc(num_channels * sizeof(float));
	}

	mymfcc.create_hamming_window(frame_size, hwindows);
	mymfcc.create_mel_filter_bank(sample_rate, num_channels, frame_size, mel_filter);

	mymfcc.pre_emphasis(frame_size, (float *)audio);
	mymfcc.apply_hamming_window((float *)audio, hwindows);
	mymfcc.apply_mel_filter_bank_power(frame_size, (float *)audio);
	mymfcc.apply_mel_filter_bank(num_channels, frame_size, (float *)audio, mel_filter, mfcc_coeffs);
}
