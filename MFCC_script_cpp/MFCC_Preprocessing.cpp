#include "MFCC_Preprocessing.h"
 

// path of the ita and eng dataset to read from
std::string datasets_path = getCurrentPath() + "datasets\\";



int main() {
    // create array of languages to elaborate: ita, eng
    std::string languages[2] = {"ita", "eng"};
    for (int i = 0; i < languages->size(); i++) {
        elaborate_dataset(languages[i]);
    }

    return 0;
}

void elaborate_dataset(std::string lang) {
// read csv file containing the arrays of integers representing the audio samples
    std::string audio_path = datasets_path + "dataset_" + lang + ".csv";

    std::cout << "reading from: " << audio_path << std::endl;

    // Open the CSV file for reading
    std::ifstream inputFile(audio_path);

    if (!inputFile.is_open()) {
        std::cerr << "Failed to open the file." << std::endl;
        return;
    }

    std::string line;
    int length = 10 * 16000;  // 10 seconds of audio at 16kHz
    int16_t* dataArray = new int16_t[length];
    int sample_number = 0;

    // Read and process each line
    std::getline(inputFile, line); // this is the header

    while (std::getline(inputFile, line)) {
        std::cout << "sample number: " << sample_number << std::endl;

        std::istringstream lineStream(line);
        std::string cell;

        int i = 0;
        // Parse each comma and space-separated cell in the line
        while (std::getline(lineStream, cell, ',')) {

            // Remove leading and trailing spaces
            int start = cell.find_first_not_of(' ');
            int end = cell.find_last_not_of(' ');

            if (cell[0] == '"') {
                start = start + 2;
            } else if (cell[end] == '"') {
                end = end - 2;
            } else if (cell[0] == 'i' || cell[0] == 'e') {
                break;
            }

            if (start != std::string::npos && end != std::string::npos) {
                dataArray[i] = std::stoi(cell.substr(start, end - start + 1));
            }
            i++;
        }

        compute_mfcc_save(dataArray, lang, sample_number);
        sample_number++;
    }

    // Close the input file
    inputFile.close();

}

void compute_mfcc_save(int16_t* audio_sample, std::string lang, int sample_number) {
    std::cout << "computing mfcc " << sample_number << std::endl;
    mymfcc = new arduinoMFCC(num_filters, frame_size, hop_size, length, num_cepstral_coeffs, frequency);

    float** mfcc_coeffs = mymfcc->compute(audio_sample);
    //mymfcc->writeFloatArrayToCSV(mfcc_coeffs, mfcc_path);

    int8_t** quantized_mfcc_coeffs = mymfcc->quantizeMFCC(mfcc_coeffs);

    char formattedString[4 + 1];  // +1 for null-terminator
    snprintf(formattedString, sizeof(formattedString), "%04d", sample_number);
    std::string formatted(formattedString);
    std::string audio_name = std::string(datasets_path + "mfcc_q_" + lang + "_" + formatted);

    std::cout << "saving mfcc to: " << audio_name << std::endl;

    mymfcc->writeInt8ArrayToCSV(quantized_mfcc_coeffs, audio_name);

    delete mymfcc;
}

std::string getCurrentPath() {
    return "D:\\Programming\\Spoken_Language_Recognition_Tensorflow_Embedded\\";
}



/**
 * reshapes audio vector into a matrix where each row is one hop, so that the original audio array
 * is progressively freed from memory as the computation of the mfcc goes forward.
 * Assuming the arduino code already creates a matrix form of data during the data collection process,
 * this code is not to be placed on the arduino
 */
int16_t **reshapeVector(int16_t *vector) {
    int16_t **matrix = new int16_t *[length / hop_size];
    for (unsigned int i = 0; i < length / hop_size; i++) {
        matrix[i] = new int16_t[hop_size];
    }

    int vecIndex = 0;
    for (unsigned int i = 0; i < length / hop_size; i++) {
        for (unsigned int j = 0; j < hop_size; j++) {
            matrix[i][j] = vector[vecIndex++];
        }
    }

    //delete[] audio;  // frees from memory original audio vector
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
            //audio[i] = valueInt16;
        }
        // Close the file when done
        inputFile.close();
    }
}
