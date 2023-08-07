/**
 * refer to this code for checking the correctness of the code below
 * https://github.com/dspavankumar/compute-mfcc/blob/master/mfcc.cc
 *
 * original code copied from this repository
 * MFCC computation library with arduino target computing platfrom
 * https://github.com/FouedDrz/arduinoMFCC/blob/main/src/arduinoMFCC.cpp
 */

#include "arduinoMFCC.h"
// #define PI 3.1415926535897932384626433832795

// constructor
arduinoMFCC::arduinoMFCC(uint8_t num_filters, uint16_t frame_size, uint8_t hop_size, unsigned int length, uint8_t num_cepstral_coeffs, uint16_t samplerate) {
    _num_filters = num_filters;
    _frame_size = frame_size;
    _hop_size = hop_size;
    _num_cepstral_coeffs = num_cepstral_coeffs;
    _samplerate = samplerate;
    this->_length = length;

    //_frame = (float *) malloc(_frame_size * sizeof(float));
    //_hamming_window = (float *) malloc(_frame_size * sizeof(float));

    _frame = new float[_frame_size];
    _hamming_window = new float[_frame_size];

    // memory allocation for the filters and coefficients

    // frequency values range from 0 to N/2, where N is the width of the sliding window
    _mel_filter_bank = new float *[_num_filters];
    for (int i = 0; i < _num_filters; i++) {
        _mel_filter_bank[i] = new float[_frame_size / 2];
    }

    _dct_filters = new float *[_num_cepstral_coeffs];
    for (int i = 0; i < _num_cepstral_coeffs; i++) {
        _dct_filters[i] = new float[_num_filters];
    }

    /*
    _mel_filter_bank = (float**) malloc(_num_filters * _frame_size * sizeof(float));
    //float **_mel_filter_bank = (float **)malloc(_num_filters * sizeof(float *));
    for (uint8_t i = 0; i < _num_filters; i++)
    {
            _mel_filter_bank[i] = (float *) malloc((_frame_size / 2) * sizeof(float)); //
    }

    //float **_dct_matrix = (float **)malloc(_mfcc_size * sizeof(float *));
    _dct_matrix = (float**) malloc(_mfcc_size * _num_filters * sizeof(float));
    for (uint8_t i = 0; i < _mfcc_size; i++)
    {
            _dct_matrix[i] = (float *)malloc(_num_filters * sizeof(float)); //
    }
    */

    //_rmfcc_coeffs = (float *)malloc(_mfcc_size * sizeof(float));
    //_mfcc_coeffs = (float *)malloc(_num_filters * sizeof(float));
    _mfcc_coeffs = new float[_num_cepstral_coeffs];
    _log_mel_filters = new float[_num_filters];
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
}

// computation of the mel-scale frequency cepstrum coefficients
int8_t** arduinoMFCC::compute(int16_t* audio) {
    // input of an audio sample of length x seconds, structured as a matrix for memory efficiency purposes
    // returns the entire mfcc matrix computed

    this->create_hamming_window();
    this->create_mel_filter_bank();
    this->create_dct_filters();

    // mfcc matrix is computed as the transpose of the usual mfcc computation
    // each frame corresponds to a row in the matrix, instead of a column
    _mfcc_matrix = new float *[_length / _hop_size - (_frame_size / _hop_size) + 1];  // number of rows = number of frames processed
    
    // for each frame
    int16_t *frame_int = new int16_t[_frame_size];
    for (int i = 0; i < this->_length / _hop_size; i+=_hop_size) {
        std::copy(audio[i], audio[i + _frame_size], frame_int);

        for (int j = 0; j < _frame_size; j++) {
            _frame[j] = (float) frame_int[j];
        }

        _mfcc_matrix[i] = new float[_num_cepstral_coeffs];  // number of columns = number of cepstral coefficients
        _mfcc_matrix[i] = this->computeFrame();
    }
    delete[] frame_int;

    return this->quantizedMFCC();
}

float *arduinoMFCC::computeFrame() {
    // mfcc computation for single frame:
    //  compute hamming window
    //  apply hamming window
    //  compute fft and magnitude
    //  compute mel filterbank
    //  apply mel filterbank
    //  take log of energies
    //  compute discrete cosine transfrom
    //  mean normalization

    this->pre_emphasis();
    this->apply_hamming_window();
    this->fft_power_spectrum();
    this->apply_log_mel_filter_bank();
    this->apply_dct();

    return _mfcc_coeffs;
}

int8_t** arduinoMFCC::quantizedMFCC() {
    int8_t** quantizedMFCC = new int8_t*[_num_filters]; //TODO: number of rows number of filters 
    for (int i = 0; i < ; i++) {
        quantizedMFCC[i] = new int8_t[_num_cepstral_coeffs]; //TODO: number of columns ceptrals coeff
    }

    // integer min-max normalization
    for (int i = 0;) {
        for (int j = 0;) {
            float scaledValue =
                quantizedMFCC[i][j] = (int8_t) scaled_value;
        }
    }
}

// pre-emphasis filter to the frame
void arduinoMFCC::pre_emphasis() {
    for (uint16_t j = 1; j < _frame_size; j++) {
        _frame[j] = _frame[j] - 0.95 * _frame[j - 1];
    }
}

// hamming window creation
void arduinoMFCC::create_hamming_window() {
    for (uint16_t i = 0; i < _frame_size; i++) {
        _hamming_window[i] = 0.54 - 0.46 * cos(2 * PI * i / (_frame_size - 1));
    }
}
/*
void arduinoMFCC::create_hamming_window(uint16_t _frame_size)
{
        for (uint16_t i = 0; i < _frame_size; i++)
        {
                _hamming_window[i] = 0.54 - 0.46 * cos(2 * PI * i / (_frame_size - 1));
        }
}

void arduinoMFCC::create_hamming_window(uint16_t _frame_size, float *_hamming_window)
{
        for (uint16_t i = 0; i < _frame_size; i++)
        {
                _hamming_window[i] = 0.54 - 0.46 * cos(2 * PI * i / (_frame_size - 1));
        }
}
*/

// mel filters definition
void arduinoMFCC::create_mel_filter_bank() {
    float f_low = 300.;
    float f_high = _samplerate / 2;  // Nyquist Frequency // added here /2
    float mel_low_freq = 2595. * log10f(1 + (f_low / 2.) / 700.);
    float mel_high_freq = 2595. * log10f(1 + (f_high / 2.) / 700.);
    float *mel_f = (float *)malloc((_num_filters + 2) * sizeof(float));
    float *hzPoints = (float *)malloc((_num_filters + 2) * sizeof(float));  // Corresponding Hz scale points
    // Calculate Mel and Hz scale points
    float mel_freq_delta = (mel_high_freq - mel_low_freq) / (_num_filters + 1);
    for (uint8_t i = 0; i < _num_filters + 2; i++) {
        mel_f[i] = mel_low_freq + i * mel_freq_delta;
        hzPoints[i] = 700.0 * (powf(10, mel_f[i] / 2595.0) - 1);
    }
    // Create the filter bank
    for (uint8_t i = 0; i < _num_filters; i++) {
        for (uint16_t j = 0; j < _frame_size / 2; j++) {
            float freq = (float)j * (_samplerate / 2) / (_frame_size / 2);
            if (freq < hzPoints[i])
                _mel_filter_bank[i][j] = 0;
            else if (freq >= hzPoints[i] && freq < hzPoints[i + 1])
                _mel_filter_bank[i][j] = (freq - hzPoints[i]) / (hzPoints[i + 1] - hzPoints[i]);
            else if (freq >= hzPoints[i + 1] && freq <= hzPoints[i + 2])
                _mel_filter_bank[i][j] = (hzPoints[i + 2] - freq) / (hzPoints[i + 2] - hzPoints[i + 1]);
            else
                _mel_filter_bank[i][j] = 0;
        }
    }
    free(mel_f);
    free(hzPoints);
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
    for (uint8_t n = 0; n < _frame_size; n++) {
        _frame[n] = _frame[n] * _hamming_window[n];
    }
}

// apply mel filters to the signal
void arduinoMFCC::apply_log_mel_filter_bank() {
    for (uint8_t i = 0; i < _num_filters; i++) {
        float sum = 0.0f;
        for (uint16_t j = 0; j < _frame_size / 2; j++) {
            sum += _frame[j] * _mel_filter_bank[i][j];
        }
        /*
        // TODO: Apply Mel-flooring
        if (lmfbCoef[i] < 1.0) // sum < 1.0
            lmfbCoef[i] = 1.0;
        */
        _log_mel_filters[i] = std::log(sum);
    }
}

// computes power spectrum: magnitude of FFT of the signal windowed frame
void arduinoMFCC::fft_power_spectrum() {
    arduinoFFT myFFTframe;
    double *_vframe = (double *) malloc(_frame_size * sizeof(double));
    double *_rframe = (double *) malloc(_frame_size * sizeof(double));
    for (uint16_t i = 0; i < _frame_size; i++) {
        _rframe[i] = (double) _frame[i];
        _vframe[i] = 0.0;
    }
    myFFTframe = arduinoFFT(_rframe, _vframe, _frame_size, (double)_samplerate);
    myFFTframe.Compute(FFT_FORWARD);
    myFFTframe.ComplexToMagnitude();
    for (uint16_t i = 0; i < _frame_size; i++)
        _frame[i] = (float) _rframe[i];
    free(_rframe);
    free(_vframe);
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
