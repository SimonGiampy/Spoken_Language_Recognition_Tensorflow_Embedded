#include "input_data.h"

int8_t** compute_mfcc(int16_t** audio_sample) {
    std::cout << "computing mfcc " << std::endl;

    mymfcc = new arduinoMFCC(num_filters, frame_size, hop_size, length, num_cepstral_coeffs, frequency);

    float** mfcc_coeffs = mymfcc->compute(audio_sample);

	float** norm_mfcc_coeffs = mymfcc->normalizeMFCC();
    
    int8_t** quantized_mfcc_coeffs = mymfcc->quantizeMFCC();

    delete mymfcc;

    return quantized_mfcc_coeffs;
}

/**
 * reshapes audio vector into a matrix where each row is one hop, so that the original audio array
 * is progressively freed from memory as the computation of the mfcc goes forward.
 * Assuming the arduino code already creates a matrix form of data during the data collection process,
 * this code is not to be placed on the arduino
 */
int16_t **reshapeVector(int16_t *vector) {
	unsigned int matrix_height = length / hop_size;
    int16_t **matrix = new int16_t *[matrix_height];
    for (unsigned int i = 0; i < matrix_height; i++) {
        matrix[i] = new int16_t[hop_size];
    }

    int vecIndex = 0;
    for (unsigned int i = 0; i < matrix_height; i++) {
        for (unsigned int j = 0; j < hop_size; j++) {
            matrix[i][j] = vector[vecIndex++];
        }
    }

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
        for (int i = 0; i < seconds * frequency; i++) {
            inputFile.read(reinterpret_cast<char *>(&valueInt16), sizeof(int16_t));
            audio_test[i] = valueInt16;
        }
        // Close the file when done
        inputFile.close();
    }
}
