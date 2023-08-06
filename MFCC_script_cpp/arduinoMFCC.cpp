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
arduinoMFCC::arduinoMFCC(uint8_t num_channels, uint16_t frame_size, uint8_t hop_size, unsigned int length, uint8_t mfcc_size, uint16_t samplerate) {
    _num_channels = num_channels;
    _frame_size = frame_size;
    _hop_size = hop_size;
    _mfcc_size = mfcc_size;
    _samplerate = samplerate;
    this->_length = length;

    //_frame = (float *) malloc(_frame_size * sizeof(float));
    //_hamming_window = (float *) malloc(_frame_size * sizeof(float));

    _frame = new float[_frame_size];
    _hamming_window = new float[_frame_size];

    // memory allocation for the filters and coefficients

    // frequency values range from 0 to N/2, where N is the width of the sliding window
    _mel_filter_bank = new float *[_num_channels];
    for (int i = 0; i < _num_channels; i++) {
        _mel_filter_bank[i] = new float[_frame_size / 2];
    }

    _dct_matrix = new float *[_mfcc_size];
    for (int i = 0; i < _mfcc_size; i++) {
        _dct_matrix[i] = new float[_num_channels];
    }

    /*
    _mel_filter_bank = (float**) malloc(_num_channels * _frame_size * sizeof(float));
    //float **_mel_filter_bank = (float **)malloc(_num_channels * sizeof(float *));
    for (uint8_t i = 0; i < _num_channels; i++)
    {
            _mel_filter_bank[i] = (float *) malloc((_frame_size / 2) * sizeof(float)); //
    }

    //float **_dct_matrix = (float **)malloc(_mfcc_size * sizeof(float *));
    _dct_matrix = (float**) malloc(_mfcc_size * _num_channels * sizeof(float));
    for (uint8_t i = 0; i < _mfcc_size; i++)
    {
            _dct_matrix[i] = (float *)malloc(_num_channels * sizeof(float)); //
    }
    */

    //_rmfcc_coeffs = (float *)malloc(_mfcc_size * sizeof(float));
    //_mfcc_coeffs = (float *)malloc(_num_channels * sizeof(float));
    _rmfcc_coeffs = new float[_mfcc_size];
    _mfcc_coeffs = new float[_num_channels];
}

// destructor
// TODO: optimize memory frees across the code
arduinoMFCC::~arduinoMFCC() {
    delete[] _frame;
    delete[] _hamming_window;

    for (int i = 0; i < _num_channels; i++) {
        delete[] _mel_filter_bank[i];
    }
    delete[] _mel_filter_bank;

    for (int i = 0; i < _mfcc_size; i++) {
        delete[] _dct_matrix[i];
    }
    delete[] _dct_matrix;

    delete[] _rmfcc_coeffs;
    delete[] _mfcc_coeffs;
}

// computation of the mel-scale frequency cepstrum coefficients
void arduinoMFCC::compute(int16_t **audio) {
    // input of an audio sample of length x seconds, structured as a matrix for memory efficiency purposes
    // returns the entire mfcc matrix computed

    this->create_hamming_window();
    this->create_mel_filter_bank();
    this->create_dct_matrix();

    int16_t *frame_int = new int16_t[_frame_size];

    // mfcc matrix is computed as the transpose of the usual mfcc computation
    // each frame corresponds to a row in the matrix, instead of a column
    _mfcc_matrix = new float *[_length / _hop_size - (_frame_size / _hop_size) + 1];  // number of rows = number of frames processed

    // for each frame
    for (int i = 0; i < this->_length / _hop_size; i++) {
        // copy each slot of size hop_size into each slot of the frame, until the frame is filled
        for (int n = 0; n < _frame_size / _hop_size; n++) {
            std::copy(audio[i + n], audio[i + n][_hop_size - 1], frame_int[n]);
        }

        for (int j = 0; j < _frame_size; j++) {
            _frame[j] = (float)frame_int[j];
        }

        _mfcc_matrix[i] = new float[_mfcc_size];  // number of columns = number of cepstral coefficients
        _mfcc_matrix[i] = this->computeFrame();
    }
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

int8_t **quantizedMFCC() {
    int8_t** quantizedMFCC = new int8_t*[?]; //TODO: number of rows
    for (int i = 0; i < _mfcc_size; i++) {
        quantizedMFCC[i] = new int8_t[?]; //TODO: number of columns
    }

    // integer min-max normalization
    for (int i = 0;) {
        for (int j = 0;) {
            float scaledValue =
                quantizedMFCC[i][j] = (int8_t)scaled_value;
        }
    }
}

/*
void arduinoMFCC::compute(uint8_t _num_channels, uint16_t _frame_size, float _samplerate, float *_frame, float *_mfcc_coeffs) {
    create_hamming_window(_frame_size, _hamming_window);
    apply_hamming_window(_frame, _hamming_window);
    create_mel_filter_bank(_samplerate, _num_channels, _frame_size, _mel_filter_bank);
    apply_mel_filter_bank_power(_frame_size, _frame);
    apply_mel_filter_bank(_num_channels, _frame_size, _frame, _mel_filter_bank, _mfcc_coeffs);
}

void arduinoMFCC::computebust_dct(uint8_t _mfcc_size, uint8_t _num_channels, uint16_t _frame_size, float *_rmfcc_coeffs)
{

        pre_emphasis(_frame_size, _frame);
        create_hamming_window(_frame_size, _hamming_window);
        apply_hamming_window(_frame, _hamming_window);
        apply_mel_filter_bank(_num_channels, _frame_size, _frame, _mel_filter_bank, _mfcc_coeffs);
        apply_dct(_mfcc_size, _num_channels, _frame_size, _mel_filter_bank, _mfcc_coeffs, _rmfcc_coeffs);
}
*/

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
    float *mel_f = (float *)malloc((_num_channels + 2) * sizeof(float));
    float *hzPoints = (float *)malloc((_num_channels + 2) * sizeof(float));  // Corresponding Hz scale points
    // Calculate Mel and Hz scale points
    float mel_freq_delta = (mel_high_freq - mel_low_freq) / (_num_channels + 1);
    for (uint8_t i = 0; i < _num_channels + 2; i++) {
        mel_f[i] = mel_low_freq + i * mel_freq_delta;
        hzPoints[i] = 700.0 * (powf(10, mel_f[i] / 2595.0) - 1);
    }
    // Create the filter bank
    for (uint8_t i = 0; i < _num_channels; i++) {
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

/*
void arduinoMFCC::create_mel_filter_bank(float _samplerate, uint8_t _num_channels, uint16_t _frame_size, float **_mel_filter_bank)
{
        float f_low = 300.;
        float f_high = _samplerate; // Nyquist Frequency
        float mel_low_freq = 2595. * log10f(1 + (f_low / 2.) / 700.);
        float mel_high_freq = 2595. * log10f(1 + (f_high / 2.) / 700.);
        float *mel_f = (float *)malloc((_num_channels + 2) * sizeof(float));
        float *hzPoints = (float *)malloc((_num_channels + 2) * sizeof(float)); // Corresponding Hz scale points
        // Calculate Mel and Hz scale points
        float mel_freq_delta = (mel_high_freq - mel_low_freq) / (_num_channels + 1);
        for (uint8_t i = 0; i < _num_channels + 2; i++)
        {
                mel_f[i] = mel_low_freq + i * mel_freq_delta;
                hzPoints[i] = 700.0 * (powf(10, mel_f[i] / 2595.0) - 1);
        }
        // Create the filter bank
        for (uint8_t i = 0; i < _num_channels; i++)
        {
                for (uint16_t j = 0; j < _frame_size / 2; j++)
                {
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
*/

// creates matrix with discrete cosine transform
void arduinoMFCC::create_dct_matrix() {
    float sqrt_2_over_n = sqrt(2.0 / _num_channels);
    for (uint8_t i = 0; i < _mfcc_size; i++) {
        for (uint8_t j = 0; j < _num_channels; j++) {
            //_dct_matrix[i][j] = sqrt_2_over_n *cos((M_PI * i * (j + 0.5)) / _num_channels);
            _dct_matrix[i][j] = sqrt_2_over_n * cos((PI * i * (j + 0.5)) / _mfcc_size);
            // TODO: should be divided by the number of filters, not the mfcc size
        }
    }
}

/*
void arduinoMFCC::create_dct_matrix(float **_dct_matrix) {
        float sqrt_2_over_n = sqrt(2.0 / _num_channels);
        for (uint8_t i = 0; i < _mfcc_size; i++)
        {
                for (uint8_t j = 0; j < _num_channels; j++)
                {
                        //_dct_matrix[i][j] = sqrt_2_over_n *cos((M_PI * i * (j + 0.5)) / _num_channels);
                        _dct_matrix[i][j] = cos((PI * i * (j + 0.5)) / _mfcc_size);
                }
        }
}
*/

// application of the hamming window to the signal

void arduinoMFCC::apply_hamming_window() {
    for (uint8_t n = 0; n < _frame_size; n++) {
        _frame[n] = _frame[n] * _hamming_window[n];
    }
}

/*
void arduinoMFCC::apply_hamming_window(float *_frame) {
    for (uint16_t n = 0; n < _frame_size; n++) {
        _frame[n] = _frame[n] * _hamming_window[n];
    }
}

void arduinoMFCC::apply_hamming_window(float *_frame, float *_hamming_window)
{
        for (uint16_t n = 0; n < _frame_size; n++)
        {
                _frame[n] = _frame[n] * _hamming_window[n];
        }
}
*/

// apply mel filters to the signal
void arduinoMFCC::apply_log_mel_filter_bank() {
    for (uint8_t i = 0; i < _num_channels; i++) {
        float output = 0.0f;
        for (uint16_t j = 0; j < _frame_size / 2; j++) {
            output += _frame[j] * _mel_filter_bank[i][j];
        }
        /*
        // TODO: Apply Mel-flooring
        if (lmfbCoef[i] < 1.0) // output < 1.0
            lmfbCoef[i] = 1.0;
        */
        _mfcc_coeffs[i] = std::log(output);
    }
}

// computes power spectrum: magnitude of FFT of the signal windowed frame
void arduinoMFCC::fft_power_spectrum() {
    arduinoFFT myFFTframe;
    double *_vframe = (double *)malloc(_frame_size * sizeof(double));
    double *_rframe = (double *)malloc(_frame_size * sizeof(double));
    for (uint16_t i = 0; i < _frame_size; i++) {
        _rframe[i] = _frame[i];
        _vframe[i] = 0.0;
    }
    myFFTframe = arduinoFFT(_rframe, _vframe, _frame_size, (double)_samplerate);
    myFFTframe.Compute(FFT_FORWARD);
    myFFTframe.ComplexToMagnitude();
    for (uint16_t i = 0; i < _frame_size; i++)
        _frame[i] = (float)_rframe[i];
    free(_rframe);
    free(_vframe);
}

/*
void arduinoMFCC::apply_mel_filter_bank(float **_mel_filter_bank, float *_mfcc_coeffs) {
    for (uint8_t i = 0; i < _num_channels; i++) {
        float output = 0.0f;
        for (uint16_t j = 0; j < _frame_size / 2; j++) {
            output += _frame[j] * _mel_filter_bank[i][j];
        }
        _mfcc_coeffs[i] = log10f(output);
    }
}
*/

// applies discrete cosine transform matrix to the signal
void arduinoMFCC::apply_dct() {
    for (uint8_t i = 0; i < _mfcc_size; i++) {
        _rmfcc_coeffs[i] = 0.0;
        for (uint8_t j = 0; j < _num_channels; j++) {
            _rmfcc_coeffs[i] += _mfcc_coeffs[j] * _dct_matrix[i][j];
        }
    }
}

/*
void arduinoMFCC::apply_dct(float **_mel_filter_bank, float *_mfcc_coeffs, float *_rmfcc_coeffs) {
    for (uint8_t i = 0; i < _mfcc_size; i++) {
        float sum = 0.0;
        for (uint8_t j = 0; j < _num_channels; j++) {
            _rmfcc_coeffs[i] += _mfcc_coeffs[j] * _dct_matrix[i][j];
        }
        _rmfcc_coeffs[i] = sum;
    }
}
*/