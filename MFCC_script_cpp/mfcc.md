## Arduino MFCC (Mel-Frequency Cepstral Coefficients)

> The MFCC method is a widely used technique for extracting relevant features from audio signals. It is commonly used in speech recognition, music genre classification, and various other applications related to audio signal processing.

The process of extracting MFCC coefficients involves several steps:

1. **Preprocessing**: The audio signal is preprocessed by applying pre-emphasis to boost high frequencies. This helps to reduce problems related to high-frequency attenuation in transmission systems. The pre-emphasis formula is as follows: 
$y[n] = x[n] - α * x[n-1]$
where x(t) is the input signal, y(t) is the output signal, and α is the pre-emphasis coefficient (typically between 0.95 and 0.97).

1. **Segmentation**: The signal is divided into fixed-size frames (usually 20 to 40 ms) with some overlap between adjacent frames. This step is necessary to account for the non-stationary nature of audio signals.

2. **Windowing**: A window function (e.g., Hamming) is applied to each frame to minimize discontinuities at the frame edges. The window is defined as follows:
$w[n] = 0.5 * (1 - \cos(2 * π * n / (N - 1)))$
where w(n) is the window value at sample n, and N is the frame size.

1. **Short-Term Fourier Transform (STFT)**: The Short-Term Fourier Transform is applied to each frame to obtain the power spectrum. This step converts the signal from the time domain to the frequency domain.

2. **Mel Filters**: The power spectrum is filtered using a bank of triangular filters spaced according to the Mel frequency scale. The Mel scale is a perceptual frequency scale that takes into account how the human ear perceives frequencies. The relationship between the Mel frequency m and the linear frequency f is as follows: $m = 2595 * \log_{10}(1 + f / 700)$

3. **Logarithm**: The logarithm of the energy in each Mel filter is calculated. This step compresses the data by reducing the dynamic range.

4. **Discrete Cosine Transform (DCT)**: Finally, the Discrete Cosine Transform is applied to the log-Mel spectrum to obtain the MFCC coefficients. Typically, only the first k coefficients are retained, where k is usually between 12 and 20.

> The first MFCC coefficients (usually 12 to 20) are used as features to represent the audio signal.