/**
 * refer to this code for checking the correctness of the code below
 * https://github.com/dspavankumar/compute-mfcc/blob/master/mfcc.cc
 *
 * original code copied from this repository
 * MFCC computation library with arduino target computing platfrom
 * https://github.com/FouedDrz/arduinoMFCC/blob/main/src/arduinoMFCC.cpp
 */

#include "arduinoMFCC.h"
#include <fstream>


// constructor
arduinoMFCC::arduinoMFCC(uint8_t num_filters, uint16_t frame_size, uint16_t hop_size, unsigned int length,
						uint8_t num_cepstral_coeffs, uint16_t samplerate) {
	_num_filters = num_filters;
	_frame_size = frame_size;
	_hop_size = hop_size;
	_num_cepstral_coeffs = num_cepstral_coeffs;
	_samplerate = samplerate;
	_length = length;
	matrix_rows = _length / _hop_size - (_frame_size / _hop_size) + 1;
	_fft_bins = _frame_size / 2 +1;

	_frame = new float[_frame_size];
	_hamming_window = new float[_frame_size];

	// memory allocation for the filters and coefficients

	// frequency values range from 0 to N/2, where N is the width of the sliding window
	_mel_filter_bank = new float *[_num_filters];
	for (int i = 0; i < _num_filters; i++) {
		_mel_filter_bank[i] = new float[_fft_bins];
	}

	std::cout << "allocated mel filterbank" << std::endl;

	_dct_filters = new float *[_num_cepstral_coeffs];
	for (int i = 0; i < _num_cepstral_coeffs; i++) {
		_dct_filters[i] = new float[_num_filters];
	}

	std::cout << "allocated dct filters" << std::endl;

	_mfcc_coeffs = new float[_num_cepstral_coeffs];
	_log_mel_filters = new float[_num_filters];
	
	_spectrum = new float[_fft_bins];

	std::cout << "allocated everything" << std::endl;
}

// destructor
// TODO: optimize memory frees across the code
arduinoMFCC::~arduinoMFCC() {
	delete[] _frame;
	delete[] _hamming_window;

	for (int i = 0; i < _num_filters; i++) {
		delete[] _mel_filter_bank[i];
	}
	delete[] _mel_filter_bank;

	for (int i = 0; i < _num_cepstral_coeffs; i++) {
		delete[] _dct_filters[i];
	}
	delete[] _dct_filters;

	delete[] _mfcc_coeffs;
	delete[] _log_mel_filters;
	delete[] _spectrum;
}

int arduinoMFCC::getMatrixRows() {
	return this->matrix_rows;
}

// computation of the mel-scale frequency cepstrum coefficients
float** arduinoMFCC::compute(int16_t* audio) {
	// input of an audio sample of length x seconds, structured as a matrix for memory efficiency purposes
	// returns the entire mfcc matrix computed

	this->create_hamming_window();
	std::cout << "computed hamming window" << std::endl;
	this->create_mel_filter_bank();
	std::cout << "computed mel filterbank" << std::endl;
	this->create_dct_filters();
	std::cout << "computed dct matrix" << std::endl;


	// mfcc matrix is computed as the transpose of the usual mfcc computation
	// each frame corresponds to a row in the matrix, instead of a column
	_mfcc_matrix = new float *[this->matrix_rows];  // number of rows = number of frames processed
	
	// for each frame
	int16_t *frame_int = new int16_t[_frame_size];
	for (int i = 0; i < this->matrix_rows; i+=1) {
		std::copy(audio + i * _hop_size, audio + i * _hop_size + _frame_size, frame_int);

		for (int j = 0; j < _frame_size; j++) {
			_frame[j] = (float) frame_int[j];
		}

		_mfcc_matrix[i] = new float[_num_cepstral_coeffs];  // number of columns = number of cepstral coefficients
		// copy the array from computeFrame() into the matrix
		float* mfcc = this->computeFrame();
		std::copy(mfcc, mfcc + _num_cepstral_coeffs, _mfcc_matrix[i]);

		std::cout << "computed frame n:" << i << std::endl;
	}
	
	std::cout << "finished frames" << std::endl;
	
	for (int row = 0; row < this->matrix_rows; row++) {
		for (int col = 0; col < _num_cepstral_coeffs; col++) {
			std::cout << _mfcc_matrix[row][col] << ", ";
		}
		std::cout << std::endl;
	}

	delete[] frame_int;
	delete[] audio;
	
	return this->normalizeMFCC();
}

float* arduinoMFCC::computeFrame() {
	// mfcc computation for single frame:
	//  compute hamming window
	//  apply hamming window
	//  compute fft and magnitude
	//  compute mel filterbank
	//  apply mel filterbank
	//  take log of energies
	//  compute discrete cosine transfrom
	//  TODO: mean normalization

	this->pre_emphasis();
	std::cout << "computed pre emphasis" << std::endl;
	this->apply_hamming_window();
	std::cout << "applied hamming" << std::endl;
	this->fft_power_spectrum();
	std::cout << "applied fft" << std::endl;
	this->apply_log_mel_filter_bank();
	std::cout << "applied log mel" << std::endl;
	this->apply_dct();
	std::cout << "applied dct" << std::endl;

	return _mfcc_coeffs;
}

/**
 * computes mean-variance normalization on the mfcc matrix (z-scoring)
*/
float** arduinoMFCC::normalizeMFCC() {
	std::cout << "Normalizing" << std::endl;

	float** normalizedMFCC = new float*[this->matrix_rows];
	for (int i = 0; i < this->matrix_rows; i++) {
		normalizedMFCC[i] = new float[_num_cepstral_coeffs];
	}

	// calculate mean and variance for each column
	float mean, variance;
	for(int col = 0; col < _num_cepstral_coeffs; col++) {
		float sum = 0;
		for(int row = 0; row < this->matrix_rows; row++){
			sum += _mfcc_matrix[row][col];
		}
		mean = sum / this->matrix_rows;
		sum = 0;
		for(int row = 0; row < this->matrix_rows; row++){
			sum += pow(_mfcc_matrix[row][col] - mean, 2);
		}
		variance = sum / this->matrix_rows;
		
		for(int row = 0; row < this->matrix_rows; row++){
			normalizedMFCC[row][col] = (_mfcc_matrix[row][col] - mean) / variance;
		}
	}
	std::cout << "Normalization done" << std::endl;

	return normalizedMFCC;
}



// integer min-max quantization of the mfcc matrix
int8_t** arduinoMFCC::quantizeMFCC(float** mfcc_matrix) {
	int8_t** quantizedMFCC = new int8_t*[this->matrix_rows];
	for (int i = 0; i < this->matrix_rows; i++) {
		quantizedMFCC[i] = new int8_t[_num_cepstral_coeffs];
	}

	// find min and max values in the matrix column-wise
	for (int col = 0; col < _num_cepstral_coeffs; col++) {
		float min = mfcc_matrix[0][col], max = mfcc_matrix[0][col];

		for (int row = 0; row < this->matrix_rows; row++) {
			if (mfcc_matrix[row][col] < min) {
				min = mfcc_matrix[row][col];
			}
			if (mfcc_matrix[row][col] > max) {
				max = mfcc_matrix[row][col];
			}
		}

		// convert from a floating point number to a signed integer of 8 bits by centering the values around 0
		// and scaling them to occupy the entire range of 256 values

		float temp;
		for (int row = 0; row < this->matrix_rows; row++) {
			temp = mfcc_matrix[row][col] / (max - min) * 255.0;
			temp -= 128.0 + min / (max - min) * 255.0;

			quantizedMFCC[row][col] = static_cast<int8_t>(std::round(temp));
		}
		std::cout << std::endl;
	}

	// print quantized values
	for (int row = 0; row < this->matrix_rows; row++) {
		for (int col = 0; col < _num_cepstral_coeffs; col++) {
			std::cout << static_cast<int>(quantizedMFCC[row][col]) << ", ";
		}
		std::cout << std::endl;
	}

	std::cout << "quantized" << std::endl;

	return quantizedMFCC;
}

// pre-emphasis filter to the frame
void arduinoMFCC::pre_emphasis() {
	int previous = _frame[0], temp;
	for (uint16_t j = 1; j < _frame_size; j++) {
		temp = _frame[j];
		_frame[j] = _frame[j] - 0.95 * previous;
		previous = temp;
	}
}

// hamming window creation
void arduinoMFCC::create_hamming_window() {
	for (uint16_t i = 0; i < _frame_size; i++) {
		_hamming_window[i] = 0.54 - 0.46 * cos(2 * PI * i / (_frame_size - 1));
	}
}


// mel filters definition
void arduinoMFCC::create_mel_filter_bank() {
	float f_low = 50.; // lower frequency cutoff
	float f_high = _samplerate / 2.0;  // higher frequency cutoff
	float mel_low_freq = 2595. * std::log10(1 + (f_low / 2.) / 700.);
	float mel_high_freq = 2595. * std::log10(1 + (f_high / 2.) / 700.);

	float *hzPoints = new float[_num_filters + 2];
	float mel_f, freq;

	// Calculate Mel and Hz scale points
	float mel_freq_delta = (mel_high_freq - mel_low_freq) / (_num_filters + 1);
	for (uint8_t i = 0; i < _num_filters + 2; i++) {
		// filter center frequencies in Hz scale
		mel_f = mel_low_freq + i * mel_freq_delta;
		hzPoints[i] = 700.0 * (std::pow(10, mel_f / 2595.0) - 1);
	}

	std::cout << "computing mel fb" << std::endl;
	// Create the filter bank
	for (uint8_t i = 0; i < _num_filters; i++) {
		for (uint16_t bin = 0; bin < _fft_bins; bin++) {
			freq = (float) bin * (_samplerate / 2.0) / (_fft_bins - 1.0);
			if (freq < hzPoints[i])
				_mel_filter_bank[i][bin] = 0;
			else if (freq >= hzPoints[i] && freq < hzPoints[i + 1])
				_mel_filter_bank[i][bin] = (freq - hzPoints[i]) / (hzPoints[i + 1] - hzPoints[i]);
			else if (freq >= hzPoints[i + 1] && freq <= hzPoints[i + 2])
				_mel_filter_bank[i][bin] = (hzPoints[i + 2] - freq) / (hzPoints[i + 2] - hzPoints[i + 1]);
			else
				_mel_filter_bank[i][bin] = 0;
			//std::cout << _mel_filter_bank[i][bin] << ", ";
		}
		//std::cout << std::endl;
	}
	std::cout << std::endl;
	
	delete[] hzPoints;
}


// creates matrix with discrete cosine transform
void arduinoMFCC::create_dct_filters() {
	float sqrt_2_over_n = sqrt(2.0 / _num_filters);
	for (uint8_t i = 0; i < _num_cepstral_coeffs; i++) {
		for (uint8_t j = 0; j < _num_filters; j++) {
			// old code contained division by number of cepstral coeffs
			_dct_filters[i][j] = sqrt_2_over_n * cos(PI * i * (j + 0.5) / _num_filters );
		}
	}
}


// application of the hamming window to the signal
void arduinoMFCC::apply_hamming_window() {
	for (uint16_t n = 0; n < _frame_size; n++) {
		_frame[n] = _frame[n] * _hamming_window[n];
	}
}

// apply mel filters to the signal
void arduinoMFCC::apply_log_mel_filter_bank() {
	for (uint8_t i = 0; i < _num_filters; i++) {
		float sum = 0.0;
		for (uint16_t j = 0; j < _fft_bins; j++) {
			sum += _spectrum[j] * _mel_filter_bank[i][j];
		}
		
		// Apply Mel-flooring
		if (sum < 1.0) {
			sum = 1.0;
			//std::cout << "x";
		}
		
		_log_mel_filters[i] = std::log(sum);
	}
	//std::cout << std::endl;
	//std::cout << "fft bins " <<_fft_bins << std::endl;
}

// computes power spectrum: magnitude of FFT of the signal windowed frame
void arduinoMFCC::fft_power_spectrum() {
	/*
	arduinoFFT myFFTframe;
	double *_vframe = new double[_frame_size];
	double *_rframe = new double[_frame_size];
	for (uint16_t i = 0; i < _frame_size; i++) {
		_rframe[i] = (double) _frame[i];
		_vframe[i] = 0.0;
	}

	std::cout << "calculating fft" << std::endl;

	myFFTframe = arduinoFFT(_rframe, _vframe, _frame_size, (double)_samplerate);
	myFFTframe.Compute(FFT_FORWARD);
	myFFTframe.ComplexToMagnitude();

	std::cout << "calculated fft" << std::endl;

	//TODO: check whether the FFT computation return an array of size fft_bins or frame_size
	// in case it returns the frame_size, then it would be better to try to implement the fft from scratch
	std::cout << "calculating spectrum ";
	for (uint16_t i = 0; i < _fft_bins; i++) {
		_spectrum[i] = (float) _rframe[i];
		std::cout << i << "=" << _spectrum[i] << " ";
	}
	std::cout << " calculated spectrum" << std::endl;
	free(_rframe);
	free(_vframe);
	*/

	kiss_fftr_cfg cfg = kiss_fftr_alloc(_frame_size, 0, 0, 0);
	kiss_fft_cpx *cx_in = new kiss_fft_cpx[_frame_size];
	kiss_fft_cpx *cx_out = new kiss_fft_cpx[_fft_bins];

	for (uint16_t i = 0; i < _frame_size; i++) {
		cx_in[i].r = _frame[i];
		cx_in[i].i = 0.0;
	}
	kiss_fftr( cfg , _frame , cx_out );

	for (uint16_t i = 0; i < _fft_bins; i++) {
		_spectrum[i] = sqrt(cx_out[i].r * cx_out[i].r + cx_out[i].i * cx_out[i].i);
	}
	std::cout << " calculated spectrum" << std::endl;
				
	free(cfg);
	delete[] cx_in;
	delete[] cx_out;
}


// applies discrete cosine transform matrix to the signal
void arduinoMFCC::apply_dct() {
	for (uint8_t i = 0; i < _num_cepstral_coeffs; i++) {
		_mfcc_coeffs[i] = 0.0;
		for (uint8_t j = 0; j < _num_filters; j++) {
			_mfcc_coeffs[i] += _log_mel_filters[j] * _dct_filters[i][j];
		}
	}
}

void arduinoMFCC::writeInt8ArrayToCSV(int8_t **mfcc_coeffs, std::string csv_name) {
    // Open a file for writing
    std::ofstream outFile(csv_name + ".csv");

    // Write the matrix elements to the CSV file
    for (int i = 0; i < getMatrixRows(); ++i) {
        for (int j = 0; j < _num_cepstral_coeffs - 1; ++j) {
            outFile << static_cast<int>(mfcc_coeffs[i][j]) << ", "; // Convert int8 to int before writing
        }
        outFile << static_cast<int>(mfcc_coeffs[i][_num_cepstral_coeffs-1]) << std::endl;
    }

    // Close the file
    outFile.close();

    std::cout << "MFCC quantized coefficients csv file has been created." << std::endl;

}

void arduinoMFCC::writeFloatArrayToCSV(float **mfcc_coeffs, std::string csv_name) {
    // Open a file for writing
    std::ofstream outFile(csv_name + ".csv");

    // Write the matrix elements to the CSV file
    for (int i = 0; i < getMatrixRows(); ++i) {
        for (int j = 0; j < _num_cepstral_coeffs - 1; ++j) {
            outFile << mfcc_coeffs[i][j] << ", "; // Convert int8 to int before writing
        }
        outFile << mfcc_coeffs[i][_num_cepstral_coeffs-1] << std::endl;
    }

    // Close the file
    outFile.close();

    std::cout << "MFCC float coefficients csv file has been created." << std::endl;

}
