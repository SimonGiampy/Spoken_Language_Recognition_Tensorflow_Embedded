/**
 * MFCC computation is based on the following library:
 * https://github.com/dspavankumar/compute-mfcc/blob/master/mfcc.cc
 *
 * MFCC computation is adapted for arduino computation inspired by this library:
 * https://github.com/FouedDrz/arduinoMFCC/blob/main/src/arduinoMFCC.cpp
 */

#include "arduinoMFCC.h"


// constructor
arduinoMFCC::arduinoMFCC(uint8_t num_filters, uint16_t frame_size, uint16_t hop_size, unsigned int length,
						uint8_t num_cepstral_coeffs, uint16_t samplerate) {
	_num_filters = num_filters;
	_frame_size = frame_size;
	_hop_size = hop_size;
	_num_cepstral_coeffs = num_cepstral_coeffs;
	_samplerate = samplerate;
	_length = length;
	_matrix_rows = _length / _hop_size - (_frame_size / _hop_size) + 1;
	_fft_bins = _frame_size / 2 +1;

	PRINT_DEBUG("initialized variables\n");
	PRINT_DEBUG("matrix rows: " + std::to_string(_matrix_rows) + "; fft bins = " + std::to_string(_fft_bins) + "\n");

	deallocation_done = false;

	_frame = new float[_frame_size];
	_hamming_window = new float[_frame_size];

	// memory allocation

	// frequency values range from 0 to N/2, where N is the width of the sliding window
	/*
	_mel_filter_bank = new float *[_num_filters];
	for (int i = 0; i < _num_filters; i++) {
		_mel_filter_bank[i] = new float[_fft_bins];
	}
	*/
	_filterbank_filters = new struct filterbank_filter[_num_filters];
	

	PRINT_DEBUG("allocated mel filterbank\n");

	_dct_filters = new float *[_num_cepstral_coeffs];
	for (int i = 0; i < _num_cepstral_coeffs; i++) {
		_dct_filters[i] = new float[_num_filters];
	}

	PRINT_DEBUG("allocated dct filters\n");

	// memory allocation for the real-valued fft library
	_cfg = kiss_fftr_alloc(_frame_size, 0, 0, 0);
	_fft_out = new kiss_fft_cpx[_fft_bins];

	_mfcc_coeffs = new float[_num_cepstral_coeffs];
	_log_mel_filters = new float[_num_filters];
	
	_spectrum = new float[_fft_bins];

	PRINT_DEBUG("allocated everything\n");
}

// destructor
// TODO: optimize memory frees across the code
arduinoMFCC::~arduinoMFCC() {
	this->deallocate_memory();

	// deallocate mfcc matrix
	for (int i = 0; i < _matrix_rows; i++) {
		delete[] _mfcc_matrix[i];
	}
	delete[] _mfcc_matrix;
}

void arduinoMFCC::deallocate_memory() {
	if (!deallocation_done) { // if normalization was not called
		deallocation_done = true;
		delete[] _frame;
		delete[] _hamming_window;

		//delete _cfg;
		delete[] _fft_out;

		/*
		for (int i = 0; i < _num_filters; i++) {
			delete[] _mel_filter_bank[i];
		}
		delete[] _mel_filter_bank;
		*/
		for (int i = 0; i < _num_filters; i++) {
			delete[] _filterbank_filters[i].filter;
		}
		delete[] _filterbank_filters;

		delete[] _log_mel_filters;
		delete[] _spectrum;

		for (int i = 0; i < _num_cepstral_coeffs; i++) {
			delete[] _dct_filters[i];
		}
		delete[] _dct_filters;

		delete[] _mfcc_coeffs;
	}
}



// computation of the mel-scale frequency cepstrum coefficients
// takes as input the audio sample (reshaped as matrix) and returns the mfcc matrix of the entire audio sample
float** arduinoMFCC::compute(int16_t** audio) {
	// input of an audio sample of length x seconds, structured as a matrix for memory efficiency purposes
	// returns the entire mfcc matrix computed as floating point numbers

	this->create_hamming_window();
	//PRINT_DEBUG("computed hamming window\n");
	this->create_mel_filter_bank();
	//PRINT_DEBUG("computed mel filterbank\n");
	this->create_dct_filters();
	//PRINT_DEBUG("computed dct matrix\n");


	// mfcc matrix is computed as the transpose of the usual mfcc computation
	// each frame corresponds to a row in the matrix, instead of a column
	_mfcc_matrix = new float *[_matrix_rows];  // number of rows = number of frames processed

	/* 
	// for each frame
	int16_t *frame_int = new int16_t[_frame_size];
	//compute mfcc matrix when the input is the whole signal array
	for (int i = 0; i < this->_matrix_rows; i+=1) {
		// copy the frame from the audio matrix into the frame array
		std::copy(audio + i * _hop_size, audio + i * _hop_size + _frame_size, frame_int);

		for (int j = 0; j < _frame_size; j++) {
			_frame[j] = (float) frame_int[j];
		}

		_mfcc_matrix[i] = new float[_num_cepstral_coeffs];  // number of columns = number of cepstral coefficients
		// copy the array from computeFrame() into the matrix
		float* mfcc = this->computeFrame();
		std::copy(mfcc, mfcc + _num_cepstral_coeffs, _mfcc_matrix[i]);

		PRINT_DEBUG("computed frame n: " + std::to_string(i));
	}

	delete[] frame_int;
	*/

	// compute mfcc matrix when the input signal is shaped as a matrix where each row is a hop
	// for each frame to be computed
	for (int i = 0; i < _matrix_rows; i++) {

		// copy the frame from the audio matrix into the frame array
		for (int hop = 0; hop < _frame_size / _hop_size; hop++) {
			for (int k = 0; k < _hop_size; k++) {
				_frame[hop * _hop_size + k] = (float) audio[i + hop][k];
			}
		}
		
		_mfcc_matrix[i] = new float[_num_cepstral_coeffs];  // number of columns = number of cepstral coefficients

		// increased memory efficiency by deallocating the array of the current hop
		delete[] audio[i];

		float* mfcc = this->computeFrame();

		// copy the mfcc array into the final matrix
		for (int el = 0; el < _num_cepstral_coeffs; el++) {
			_mfcc_matrix[i][el] = mfcc[el];
		}
		
		//PRINT_DEBUG("computed frame n: " + std::to_string(i) + "\n");
	}

	PRINT_DEBUG("finished frames computation\n");

	// deallocate the audio matrix pointers and the last frame
	for (int i = _matrix_rows; i < _matrix_rows + _frame_size / _hop_size - 1; i++) {
		delete[] audio[i]; // deletes the last hops in the last frame
	}
	delete[] audio;

	deallocate_memory();

	// print computed MFCC (non-normalized)
	for (int i = 0; i < _matrix_rows; i++) {
		for (int j = 0; j < _num_cepstral_coeffs; j++) {
			PRINT_DEBUG(std::to_string(_mfcc_matrix[i][j]) + ", ");
		}
		PRINT_DEBUG("\n");
	}
	PRINT_DEBUG("\n");
	
	return _mfcc_matrix;
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
	//  mean-variance normalization

	this->pre_emphasis();
	//PRINT_DEBUG("computed pre emphasis\n");
	this->apply_hamming_window();
	//PRINT_DEBUG("applied hamming\n");
	this->fft_power_spectrum();
	//PRINT_DEBUG("applied fft\n");
	this->apply_log_mel_filter_bank();
	//PRINT_DEBUG("applied log mel\n");
	this->apply_dct();
	//PRINT_DEBUG("applied dct\n");

	return _mfcc_coeffs;
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


// mel filters definition: matrix of size = number of filters x number of fft bins
// create memory-efficient representation of the sparse matrix corresponding to the mel filterbank
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

	//PRINT_DEBUG("computing mel fb\n");

	uint8_t start_index, end_index;
	// Create the filter bank memory efficient structure
	for (uint8_t i = 0; i < _num_filters; i++) {

		// index of the first and last frequency bins to which the current filter is applied
		start_index = 0;
		end_index = 0;

		// compute start index and end index of the current filter
		// in order to get only the portion of the bins containing values different from zero
		for (uint16_t bin = 0; bin < _fft_bins; bin++) {
			freq = (float) bin * (_samplerate / 2.0) / (_fft_bins - 1.0);
			if (freq < hzPoints[i]) {
				//mel_filter_bank[i][bin] = 0;
				start_index++;
				end_index++;
			} else if (freq >= hzPoints[i] && freq < hzPoints[i + 1]) {
				//_mel_filter_bank[i][bin] = (freq - hzPoints[i]) / (hzPoints[i + 1] - hzPoints[i]);
				end_index++;
			} else if (freq >= hzPoints[i + 1] && freq <= hzPoints[i + 2]) {
				//_mel_filter_bank[i][bin] = (hzPoints[i + 2] - freq) / (hzPoints[i + 2] - hzPoints[i + 1]);
				end_index++;
			} else {
				//_mel_filter_bank[i][bin] = 0;
			}
		}
		
		// create floating point array with only the necessary bins
		float* filter_values = new float[end_index - start_index];
		for (uint16_t bin = start_index, k = 0; bin < end_index; bin++, k++) {
			freq = (float) bin * (_samplerate / 2.0) / (_fft_bins - 1.0);
			//_mel_filter_bank[i][bin] = (bin - start_index + 1) / (end_index - start_index + 1);
			if (freq >= hzPoints[i] && freq < hzPoints[i + 1]) {
				//_mel_filter_bank[i][bin] = (freq - hzPoints[i]) / (hzPoints[i + 1] - hzPoints[i]);
				filter_values[k] = (freq - hzPoints[i]) / (hzPoints[i + 1] - hzPoints[i]);
			} else if (freq >= hzPoints[i + 1] && freq <= hzPoints[i + 2]) {
				//_mel_filter_bank[i][bin] = (hzPoints[i + 2] - freq) / (hzPoints[i + 2] - hzPoints[i + 1]);
				filter_values[k] = (hzPoints[i + 2] - freq) / (hzPoints[i + 2] - hzPoints[i + 1]);
			}
		}
		
		// save the memory efficient representation of the filterbank
		_filterbank_filters[i].start_index = start_index;
		_filterbank_filters[i].end_index = end_index;
		_filterbank_filters[i].filter = filter_values;
		
	}
	
	delete[] hzPoints;
}


// creates matrix with discrete cosine transform (type II)
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

	// compute FFT
	kiss_fftr( _cfg , _frame , _fft_out );

	// compute power spectrum as the magnitude of the FFT output
	for (uint16_t i = 0; i < _fft_bins; i++) {
		_spectrum[i] = sqrt(_fft_out[i].r * _fft_out[i].r + _fft_out[i].i * _fft_out[i].i);
	}
				
	
}



// apply mel filters to the signal
void arduinoMFCC::apply_log_mel_filter_bank() {
	for (uint8_t i = 0; i < _num_filters; i++) {
		float sum = 0.0;
		for (uint16_t j = 0; j < _fft_bins; j++) {

			// compute the value in the filterbank matrix at the corresponding bin
			float value;
			// uses memory-efficient representation of the filter bank matrix
			if (j >= _filterbank_filters[i].start_index && j < _filterbank_filters[i].end_index) {
				int position = j - _filterbank_filters[i].start_index;
				value = (_filterbank_filters[i].filter)[position];
			} else {
				value = 0.0;
			}

			//sum += _spectrum[j] * _mel_filter_bank[i][j];
			sum += _spectrum[j] * value;
		}
		
		// Apply Mel-flooring
		if (sum < 1.0) {
			sum = 1.0;
		}
		
		// Take log of the sum of the filtered spectrum
		_log_mel_filters[i] = std::log(sum);

		// take the second power of the log mel filters in order to decrease the influence of the noise,
		// so to improve the robustness of the algorithm (reduction of low frequency components)
		_log_mel_filters[i] = _log_mel_filters[i] * _log_mel_filters[i];
	}
}

// applies discrete cosine transform matrix to the signal
void arduinoMFCC::apply_dct() {
	// for all the cepstral coefficients to keep
	for (uint8_t i = 0; i < _num_cepstral_coeffs; i++) {
		_mfcc_coeffs[i] = 0.0;
		for (uint8_t j = 0; j < _num_filters; j++) {
			_mfcc_coeffs[i] += _log_mel_filters[j] * _dct_filters[i][j];
		}
	}
}



/**
 * computes mean-variance normalization on the mfcc matrix (z-scoring)
 * returns floating point matrix of normalized mfcc coefficients
*/
float** arduinoMFCC::normalizeMFCC() {
	PRINT_DEBUG("normalizing mfcc matrix\n");

	// calculate mean and variance for each column
	float mean, variance;
	for(int col = 0; col < _num_cepstral_coeffs; col++) {
		float sum = 0;
		//compute mean
		for(int row = 0; row < _matrix_rows; row++){
			sum += _mfcc_matrix[row][col];
		}
		mean = sum / _matrix_rows;

		//compute variance
		sum = 0;
		for(int row = 0; row < _matrix_rows; row++){
			sum += pow(_mfcc_matrix[row][col] - mean, 2);
		}
		variance = sum / _matrix_rows;
		
		// normalization computed in place
		for(int row = 0; row < _matrix_rows; row++){
			_mfcc_matrix[row][col] = (_mfcc_matrix[row][col] - mean) / variance;
		}
	}

	PRINT_DEBUG("normalization complete\n");

	return _mfcc_matrix;
}



// integer min-max quantization of the mfcc matrix
int8_t** arduinoMFCC::quantizeMFCC() {
	PRINT_DEBUG("quantizing mfcc matrix\n");

	int8_t** quantizedMFCC = new int8_t*[_matrix_rows];
	for (int i = 0; i < _matrix_rows; i++) {
		quantizedMFCC[i] = new int8_t[_num_cepstral_coeffs];
	}

	// find min and max values in the matrix column-wise
	for (int col = 0; col < _num_cepstral_coeffs; col++) {
		float min = _mfcc_matrix[0][col], max = _mfcc_matrix[0][col];

		for (int row = 0; row < _matrix_rows; row++) {
			if (_mfcc_matrix[row][col] < min) {
				min = _mfcc_matrix[row][col];
			}
			if (_mfcc_matrix[row][col] > max) {
				max = _mfcc_matrix[row][col];
			}
		}

		// convert from a floating point number to a signed integer of 8 bits by centering the values around 0
		// and scaling them to occupy the entire range of 256 values

		float temp;
		for (int row = 0; row < _matrix_rows; row++) {
			temp = _mfcc_matrix[row][col] / (max - min) * 255.0;
			temp -= 128.0 + min / (max - min) * 255.0;

			quantizedMFCC[row][col] = static_cast<int8_t>(std::round(temp));
		}
	}

	
	// print quantized values
	PRINT_DEBUG("printing quantized values:\n");
	for (int row = 0; row < _matrix_rows; row++) {
		for (int col = 0; col < _num_cepstral_coeffs; col++) {
			PRINT_DEBUG(std::to_string( static_cast<int>(quantizedMFCC[row][col]) ) + ", ");
		}
		PRINT_DEBUG("\n");
	}

	PRINT_DEBUG("quantization complete\n");
	

	return quantizedMFCC;
}