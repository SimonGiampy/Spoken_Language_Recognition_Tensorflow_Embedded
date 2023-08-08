#include <fstream>
#include <iostream>

#include "arduinoMFCC.h"

// MFCC parameters
const uint8_t num_filters = 40;
const uint16_t frame_size = 512;
const uint16_t hop_size = 128;
const uint8_t num_cepstral_coeffs = 12;
const uint16_t frequency = 16000;

// 10 seconds audio recording at 16kHz at 2 bytes/sample
const int seconds = 10;
const unsigned int length = frequency * seconds;
int16_t *audio;

// MFCC object
arduinoMFCC *mymfcc;

void readBinary(const char *filePath);

int16_t **reshapeVector(int16_t *vector);

int main(void);
