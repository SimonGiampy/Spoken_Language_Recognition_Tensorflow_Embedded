#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "arduinoMFCC.h"

// MFCC parameters
const uint8_t num_filters = 40;
const uint16_t frame_size = 512;
const uint16_t hop_size = 128;
const uint8_t num_cepstral_coeffs = 13;
const uint16_t frequency = 16000;

// 10 seconds audio recording at 16kHz at 2 bytes/sample
const int seconds = 10;
const unsigned int length = frequency * seconds;

// MFCC object
arduinoMFCC *mymfcc;
int mfcc_matrix_rows = length / hop_size - (frame_size / hop_size) + 1;
int mfcc_matrix_cols = num_cepstral_coeffs;

int16_t *audio_test;

void readBinary(const char *filePath);

int16_t **reshapeVector(int16_t *vector);

// computes MFCC, normalize, quantize and save to csv with formatted file name
void compute_mfcc_save(int16_t** audio_sample, std::string lang, int sample_number);
// computes MFCC, normalize, quantize, and save to csv as a test file
void compute_mfcc_save(int16_t** audio_sample);

std::string getCurrentPath(void);

// reads dataset_[lang].csv, computes MFCC for each sample in the dataset and save it to csv files in the datasets folder
void elaborate_dataset(std::string lang);
   
// saves MFCC matrix into csv file
void writeInt8ArrayToCSV(int8_t **mfcc_coeffs, std::string csv_name);
void writeFloatArrayToCSV(float **mfcc_coeffs, std::string csv_name);

int main(void);
