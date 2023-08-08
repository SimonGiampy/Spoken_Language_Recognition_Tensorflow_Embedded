#include <fstream>
#include <iostream>


#include "arduinoMFCC.h"

// MFCC parameters
const int num_filters = 40;
const int frame_size = 512;
const int hop_size = 128;
const int num_cepstral_coeffs = 12;
const int sample_rate = 16000;

// 10 seconds audio recording at 16kHz at 2 bytes/sample
const int seconds = 10;
const int length = sample_rate * seconds;
int16_t *audio;

// MFCC object
arduinoMFCC *mymfcc;

void readBinary(const char *filePath);
void writeInt8ArrayToCSV(int8_t **mfcc_coeffs);

int16_t **reshapeVector(int16_t *vector);

int main() {
    // Specify the path to the audio file on the SD card
    const char *audioFilePath = "test_audio.bin";
    audio = new int16_t[length];
    readBinary(audioFilePath);

    //int16_t **matrix = reshapeVector(audio);

    mymfcc = new arduinoMFCC(num_filters, frame_size, hop_size, length, num_cepstral_coeffs, sample_rate);
    
    int8_t **mfcc_coeffs = mymfcc->compute(audio);

    writeInt8ArrayToCSV(mfcc_coeffs);

    delete mymfcc;

    // create array of frames (array of arrays)
    // for each frame
    //		compute mfcc
    //		return matrix and print it in csv file

    return 0;
}

//Saves the mfcc_coefficients into a csv file 

/**
 * reshapes audio vector into a matrix where each row is one hop, so that the original audio array
 * is progressively freed from memory as the computation of the mfcc goes forward.
 * Assuming the arduino code already creates a matrix form of data during the data collection process,
 * this code is not to be placed on the arduino
 */
int16_t **reshapeVector(int16_t *vector) {
    int16_t **matrix = new int16_t *[length / hop_size];
    for (int i = 0; i < length / hop_size; i++) {
        matrix[i] = new int16_t[hop_size];
    }

    int vecIndex = 0;
    for (int i = 0; i < length / hop_size; i++) {
        for (int j = 0; j < hop_size; j++) {
            matrix[i][j] = vector[vecIndex++];
        }
    }

    delete[] audio;  // frees from memory original audio vector
    return matrix;
}

/**
 * helper function to read bytes from a file and conver them to an array of integers of 2 bytes
 */
void readBinary(const char *filePath) {
    // Open the binary file for binary input
    std::ifstream inputFile(filePath, std::ios::binary);

    // Check if the file was opened successfully
    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open the binary file." << std::endl;
    } else {
        int16_t valueInt16;
        for (int i = 0; i < seconds * sample_rate; i++) {
            inputFile.read(reinterpret_cast<char *>(&valueInt16), sizeof(int16_t));
            audio[i] = valueInt16;
        }
        // Close the file when done
        inputFile.close();
    }
}
