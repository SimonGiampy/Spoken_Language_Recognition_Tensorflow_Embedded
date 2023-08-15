#include "MFCC_Preprocessing.h"
 

// path of the ita and eng dataset to read from
std::string datasets_path = getCurrentPath() + "datasets/";

int main() {
    /*
    // read audio test file and generate mfcc matrix from it, for testing purposes
	audio_test = new int16_t[length];
    readBinary("test_audio.bin");
	int16_t **audio_test_matrix = reshapeVector(audio_test);
	delete[] audio_test;  // frees from memory original audio vector
    compute_mfcc_save(audio_test_matrix);
	*/

    
    // create entire dataset of csv files containing the MFCC coefficients
    // create array of languages to elaborate: ita, eng
    std::string split[2] = {"train", "validation"};
    for (unsigned int i = 0; i < 2; i++) {
        elaborate_dataset(split[i]);
    }
    

    return 0;
}

void elaborate_dataset(std::string split) {
    // read csv file containing the arrays of integers representing the audio samples
    std::string audio_path = datasets_path + "dataset_" + split + ".csv";
    std::cout << "reading from: " << audio_path << std::endl;

    // Open the CSV file for reading
    std::ifstream inputFile(audio_path);

    if (!inputFile.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return;
    }

    std::string line, lang;
    int16_t* dataArray = new int16_t[length];
    short sample_number = 1;

    // Read and process each line
    //std::getline(inputFile, line); // this is for skipping the header

    while (std::getline(inputFile, line)) {
        std::cout << "sample number: " << sample_number << std::endl;

        std::istringstream lineStream(line);
        std::string cell;

        int i = 0;
        std::getline(lineStream, cell, ',');  // first cell contains language label
        lang = cell;
        std::getline(lineStream, cell, ',');  // second cell contains speaker label

        // Parse each comma and space-separated cell in the line
        while (std::getline(lineStream, cell, ',')) {

            // Remove leading and trailing spaces
            unsigned int start = cell.find_first_not_of(' ');
            unsigned int end = cell.find_last_not_of(' ');

            if (cell[0] == '"') {
                start = start + 2;
            } else if (cell[end] == '"') {
                end = end - 2;
            } 

            if (start != std::string::npos && end != std::string::npos) {
                dataArray[i] = std::stoi(cell.substr(start, end - start + 1));
            }
            i++;
        }

        char formattedString[7];  // +1 for null-terminator
        snprintf(formattedString, sizeof(formattedString), "%04d", sample_number);
        std::string formatted(formattedString);
        std::string audio_name = std::string(datasets_path + split + "/" +"mfcc_" + lang + "_" + formatted + ".csv");
        
        std::cout << "computing mfcc " << sample_number << std::endl;
        compute_mfcc_save(reshapeVector(dataArray), audio_name);

        sample_number++;
    }

    // Close the input file
    inputFile.close();

}

void compute_mfcc_save(int16_t** audio_sample, std::string file_name) {
    mymfcc = new arduinoMFCC(num_filters, frame_size, hop_size, length, num_cepstral_coeffs, frequency);

    float** mfcc_coeffs = mymfcc->compute(audio_sample);

    float** norm_mfcc_coeffs = mymfcc->normalizeMFCC();
    
    int8_t** quantized_mfcc_coeffs = mymfcc->quantizeMFCC();

    std::cout << "saving mfcc to: " << file_name << std::endl;

    writeInt8ArrayToCSV(quantized_mfcc_coeffs, file_name);

    delete mymfcc;
}


void compute_mfcc_save(int16_t** audio_sample) {
    std::cout << "computing mfcc " << std::endl;

    mymfcc = new arduinoMFCC(num_filters, frame_size, hop_size, length, num_cepstral_coeffs, frequency);

    float** mfcc_coeffs = mymfcc->compute(audio_sample);

	std::cout << "saving base float mfcc to: mfcc_test_float.csv" << std::endl;
	writeFloatArrayToCSV(mfcc_coeffs, "mfcc_test_float.csv");

	float** norm_mfcc_coeffs = mymfcc->normalizeMFCC();

	std::cout << "saving base float mfcc to: mfcc_test__norm_float.csv" << std::endl;
	writeFloatArrayToCSV(mfcc_coeffs, "mfcc_test_norm_float.csv");
    
    int8_t** quantized_mfcc_coeffs = mymfcc->quantizeMFCC();

	std::cout << "saving int 8 mfcc to: mfcc_test_int8.csv" << std::endl;
	writeInt8ArrayToCSV(quantized_mfcc_coeffs, "mfcc_test_int8.csv");

    delete mymfcc;
}

std::string getCurrentPath() {
    //"D:\\Programming\\Spoken_Language_Recognition_Tensorflow_Embedded\\";
    // return parent path of the executable file created with g++ compiler
    return "/home/simon/Spoken_Language_Recognition_Tensorflow_Embedded/";
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


void writeInt8ArrayToCSV(int8_t **mfcc_coeffs, std::string csv_name) {
    // Open a file for writing
    std::ofstream outFile(csv_name);

    // Write the matrix elements to the CSV file
    for (int i = 0; i < mfcc_matrix_rows; ++i) {
        for (int j = 0; j < mfcc_matrix_cols - 1; ++j) {
            outFile << static_cast<int>(mfcc_coeffs[i][j]) << ", "; // Convert int8 to int before writing
        }
        outFile << static_cast<int>(mfcc_coeffs[i][mfcc_matrix_cols-1]) << std::endl;
    }

    // Close the file
    outFile.close();

    std::cout << "MFCC quantized coefficients csv file has been created." << std::endl;

}

void writeFloatArrayToCSV(float **mfcc_coeffs, std::string csv_name) {
    // Open a file for writing
    std::ofstream outFile(csv_name);

    // Write the matrix elements to the CSV file
    for (int i = 0; i < mfcc_matrix_rows; ++i) {
        for (int j = 0; j < mfcc_matrix_cols - 1; ++j) {
            outFile << mfcc_coeffs[i][j] << ", "; // Convert int8 to int before writing
        }
        outFile << mfcc_coeffs[i][mfcc_matrix_cols-1] << std::endl;
    }

    // Close the file
    outFile.close();

    std::cout << "MFCC float coefficients csv file has been created." << std::endl;

}

