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
 * step 8: Cepstral Mean Normalization (Optional):
 *		perform mean normalization by subtracting the mean value of each MFCC coefficient across all frames.
 *		this step helps to minimize speaker-specific characteristics.
 * step 9: 1 byte quantization of the floating point values of the coefficients
*/

#ifndef arduinoMFCC_H
#define arduinoMFCC_H

#include <stdint.h>

#include <algorithm>
#include <cmath>

#include "arduinoFFT.h"
#define AUDIO_SAMPLE_RATE_EXACT 16000

class arduinoMFCC {
public:

	// Constructor
	arduinoMFCC(uint8_t num_channels, uint16_t frame_size, uint8_t hop_size, unsigned int length, uint8_t mfcc_size, uint16_t samplerate);
	~arduinoMFCC();

	// public functions

	void compute(int16_t** audio);
	// void compute(uint8_t ,uint16_t ,float , float *, float *);
	float* computeFrame(void);

	// void computebust(uint8_t ,uint16_t ,float , float *, float *);
	// void computebust_dct(uint8_t ,uint8_t ,uint16_t ,float *);

	void create_hamming_window(void);
	// void create_hamming_window(uint16_t  );
	// void create_hamming_window(uint16_t , float *);

	void pre_emphasis(void);

	void apply_hamming_window();
	// void apply_hamming_window(float* _frame);
	// void apply_hamming_window(float *,float *);

	void create_mel_filter_bank();
	// void create_mel_filter_bank(float , uint8_t  ,uint16_t , float**);

	//void apply_mel_filter_bank_power(void);
	void fft_power_spectrum(void);

	void apply_log_mel_filter_bank(void);
	// void apply_mel_filter_bank(uint8_t  , uint16_t  ,float *, float **, float *);v

	void create_dct_matrix();
	// void create_dct_matrix(float **);

	void apply_dct();
	//void apply_dct(uint8_t, uint8_t, uint16_t, float**, float*, float*);
	
	uint8_t _hop_size;
	uint16_t _samplerate;
	float* _frame;         // array holding the current frame being processed
	uint16_t _frame_size;  // must be integer multiple of hop_size

	// log mel-scale filter bank coefficients computed from a single frame
	// vector of length = number of filters
	float* _mfcc_coeffs; // TODO: rename -> log_mel_fb
	// final mfcc coefficients computed after applying the discrete cosine transform
	// vector of length = number of cepstral coefficients
	float* _rmfcc_coeffs; // TODO: rename -> mfcc

	// number of triangular filters used in the Mel-filterbank to filter the power spectrum of an audio signal.
	//  each channel in the Mel-filterbank corresponds to a specific frequency range in the audio spectrum.
	uint8_t _num_channels; // TODO: rename -> _num_filters

	// window function used in digital signal processing to reduce spectral leakage when analyzing or processing a signal.
	// window functions are applied to the data before performing a Fourier transform or other frequency analysis
	// to minimize artifacts caused by abrupt truncation of the signal.
	float* _hamming_window;

	// set of triangular filters used in audio signal processing to convert the linear frequency spectrum of an audio signal
	//  into the Mel-frequency scale, which approximates the non-linear frequency perception of the human auditory system.
	float** _mel_filter_bank;

	
	uint8_t _mfcc_size; // TODO: rename -> num_cepstral_coeffs
	float** _dct_matrix; // TODO: rename -> dct_filters

private:
	unsigned int _length;  // expressed in seconds

	// matrix of size = number of frames processed (rows) x number of cepstral coefficients
	float** _mfcc_matrix;  // final computed matrix

	//TODO: make the variables private
};

#endif
