/**
 * reference:
 * http://practicalcryptography.com/miscellaneous/machine-learning/guide-mel-frequency-cepstral-coefficients-mfccs/
 * 
 * Mel-scale frequency cesptral coefficients
 * 
 * step 1: compute pre-emphasis filter on entire audio signal to boost higher frequencies
 * step 2: apply hopping window on signal computing a series of frames with some overalp (hop)
 * step 3: apply hamming windowing function to each frame
 * step 4: apply STFT to obtain power spectrum
 *		- 4.1: apply the Discrete Fourier Transform (DFT) to each windowed frame.
 *		- 4.2: compute the magnitude spectrum (or squared magnitude) for each frame.
 * step 5: compute mel-scale filter bank and apply the filter to the magnitude spectrum obtained
 * 		-5.1: Design a Mel-filterbank consisting of a set of triangular filters, uniformly spaced on the Mel-frequency scale.
 *		-5.2: Apply each triangular filter to the magnitude spectrum to obtain the energy in each Mel-frequency band.
 * step 6: take the logarithm of the filterbank energies to compress the dynamic range.
 * step 7: apply the Discrete Cosine Transform (DCT) to the log-filterbank energies to obtain the MFCC coefficients.
 * step 8: Cepstral Mean-Variance Normalization (column-wise):
 *		perform mean-variance normalization by subtracting the mean value and dividing by the standard deviation 
 * 		of each MFCC coefficient across all frames. This step helps to minimize speaker-specific characteristics.
 * step 9: 1-byte min-max column-wise quantization of the floating point values of the coefficients
*/

#ifndef arduinoMFCC_H
#define arduinoMFCC_H

#include <stdint.h>

// library for FFT computation
#include "kiss_fftr.h"

// shortcut for enabling / disabling printing debug messages
#define ENABLE_DEBUG 1


#ifdef ARDUINO // arduino - specific code for debugging and includes
#include <Arduino.h>
#undef PI // redefinition of PI

#define PRINT_DEBUG(x) if(ENABLE_DEBUG) Serial.print((std::string(x)).c_str())

#else // pc specific code for debugging
#include <iostream>

#define PRINT_DEBUG(x) if(ENABLE_DEBUG) std::cout << x;

#endif

const float PI = 3.14159265358979323f;


class arduinoMFCC {
public:

	// Constructor
	arduinoMFCC(uint8_t num_filters, uint16_t frame_size, uint16_t hop_size, unsigned int length,
                        uint8_t num_cepstral_coeffs, uint16_t samplerate);
	// Destructor
	~arduinoMFCC();

	// Methods

	void deallocate_memory(void);

	// given a matrix-shaped audio signal, computes MFCC matrix as float values
	float** compute(int16_t** audio);

	// given a frame, computes the mfcc vector associated to the frame
	float* computeFrame(void);

	// computes hamming window coefficients to be applied to the frame
	void create_hamming_window(void);

	// computes pre-emphasis filter coefficients to be applied to the raw frame
	void pre_emphasis(void);

	// applies the hamming window to the emphasized frame
	void apply_hamming_window(void);

	// creates the mel filter bank as a memory-efficient representation of a sparse matrix, being the mel-filterbank
	void create_mel_filter_bank(void);

	// computes the FFT of the windowed signal and then computes the magnitude spectrum of the trannsformed signal
	void fft_power_spectrum(void);

	// applies the mel filter bank to the power spectrum of the signal
	// then computes the log of the filter bank energies
	// finally exponentiates the log filter bank energies with power = 2 to reduce lower energy contributions
	void apply_log_mel_filter_bank(void);

	// computes the discrete cosine transform (type II) coefficients
	void create_dct_filters(void);

	// applies the discrete cosine transform coefficients to the log mel filter bank energies
	void apply_dct(void);

	// given a MFCC matrix, computes the z-score normalization of the MFCC, coefficient-wise
	// returns the normalized MFCC matrix representation as float values
	float** normalizeMFCC(void);

	// given a MFCC matrix, computes the min-max standardization of the MFCC, coefficient-wise
	// returns the MFCC matrix as single-byte integer values
	int8_t** quantizeMFCC(void);


private:
	unsigned int _length;  // length of audio expressed in number of signal samples

	// matrix of size = number of frames processed (rows) x number of cepstral coefficients
	float** _mfcc_matrix;  // final computed matrix

	int _matrix_rows; // number of rows of the computed MFCC matrix (time on y axis)

	uint16_t _hop_size; // size of the hop between two consecutive frames
	uint16_t _samplerate; // sampling rate of the audio signal 
	float* _frame;         // array holding the current frame being processed
	uint16_t _frame_size;  // must be integer multiple of hop_size

	// log mel-scale filter bank coefficients computed from a single frame
	// vector of length = number of filters
	float* _log_mel_filters;

	// final mfcc coefficients computed after applying the discrete cosine transform
	// vector of length = number of cepstral coefficients
	float* _mfcc_coeffs;

	// number of triangular filters used in the Mel-filterbank to filter the power spectrum of an audio signal.
	//  each channel in the Mel-filterbank corresponds to a specific frequency range in the audio spectrum.
	uint8_t _num_filters;

	// Real-valued FFT configuration and output vector
	kiss_fftr_cfg _cfg;
	kiss_fft_cpx* _fft_out;

	// number of bins (frequency intervals) of the FFT
	uint16_t _fft_bins;

	// window function used in digital signal processing to reduce spectral leakage when analyzing or processing a signal.
	// window functions are applied to the data before performing a Fourier transform or other frequency analysis
	// to minimize artifacts caused by abrupt truncation of the signal.
	float* _hamming_window;

	// set of triangular filters used in audio signal processing to convert the linear frequency spectrum of an audio signal
	//  into the Mel-frequency scale, which approximates the non-linear frequency perception of the human auditory system.
	//float** _mel_filter_bank;

	// memory-efficient structure for storing the mel filter bank
	struct filterbank_filter {
		uint16_t start_index;
		uint16_t end_index;
		float* filter;
	};

	// array of filterbank filters = whole filterbank composed of a series of filters
	struct filterbank_filter* _filterbank_filters;

	// power spectrum (magnitude) of the processed signal with FFT
	float* _spectrum;
	
	// number of cepstral coefficients to keep for the MFCC computation
	// also corresponds to the MFCC matrix number of columns (coefficientx on x axis)
	uint8_t _num_cepstral_coeffs;

	// Discrete Cosine Transform (DCT) filters used to compute the MFCC coefficients
	// matrix of size = number of cepstral coefficients x number of filters
	float** _dct_filters;

	bool deallocation_done; // = true when memory has been deallocated
};

#endif
