

#include <PDM.h>
#include <Arduino.h>

#include <TensorFlowLite.h>


#include "model.h"
#include "output_handler.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <arduinoMFCC.h>

// Globals, used for compatibility with Arduino-style sketches.
namespace {

	const tflite::Model* model = nullptr;
	tflite::MicroInterpreter* interpreter = nullptr;
	TfLiteTensor* input = nullptr;
	TfLiteTensor* output = nullptr;

	constexpr int kTensorArenaSize = 105000;
	// Keep aligned to 16 bytes for CMSIS
	//alignas(16) uint8_t tensor_arena[kTensorArenaSize];
	uint8_t *tensor_arena;
}  // namespace


int8_t** quantized_mfccs;

int8_t** compute_mfcc(void);

void onPDMdata(void);

// default number of output channels
static const char channels = 1;

// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[512];

// Number of audio samples read
int bytesRead;

// MFCC parameters
const uint8_t num_filters = 40;
const uint16_t frame_size = 512;
const uint16_t hop_size = 256;
const uint8_t num_cepstral_coeffs = 12;
const uint16_t frequency = 16000;

// 10 seconds audio recording at 16kHz at 2 bytes/sample
const float seconds = 5.6;
const unsigned int length = frequency * seconds;

// MFCC object
arduinoMFCC *mymfcc;
const int mfcc_matrix_rows = length / hop_size - (frame_size / hop_size) + 1;
const int mfcc_matrix_cols = num_cepstral_coeffs;

short **audio_signal;
unsigned int audio_index;


void setup() {

	//Serial.begin(9600);
	
	audio_signal = new short*[length / hop_size];
	for (unsigned int i = 0; i < length / hop_size; i++) {
		audio_signal[i] = new short[hop_size];
	}
	audio_index = 0;

	// Configure the data receive callback
	PDM.onReceive(onPDMdata);

	// Optionally set the gain
	// Defaults to 20 on the BLE Sense and 24 on the Portenta Vision Shield
	PDM.setGain(40);

	// Initialize PDM with:
	// - one channel (mono mode)
	// - a 16 kHz sample rate for the Arduino Nano 33 BLE Sense
	// - a 32 kHz or 64 kHz sample rate for the Arduino Portenta Vision Shield
	if (!PDM.begin(channels, frequency)) {
		//Serial.println("Failed to start PDM!");
		while (1);
	}
}

void loop() {
	delay(5000);
	
	MicroPrintf("Start recording");

	int8_t** quantized_mfcc = compute_mfcc();

	// dynamic tensor arena allocation
	tensor_arena = new uint8_t[kTensorArenaSize];

	tflite::InitializeTarget(); // target-specific code

	MicroPrintf("initialized target");

    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    model = tflite::GetModel(_____model_lite_CNN_model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        PRINT_DEBUG("Model provided is schema version" + std::to_string(model->version()) +
		 " not equal to supported version " + std::to_string(TFLITE_SCHEMA_VERSION));
		//MicroPrintf("Model provided is schema version %d not equal to supported version %d.",
        //model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

	MicroPrintf("initialized model");

    // This pulls in all the operation implementations we need.
    // NOLINTNEXTLINE(runtime-global-variables)
    //static tflite::AllOpsResolver resolver;

	
	static tflite::MicroMutableOpResolver<6> micro_op_resolver;
	micro_op_resolver.AddConv2D();
	micro_op_resolver.AddMaxPool2D();
	micro_op_resolver.AddAveragePool2D();
	micro_op_resolver.AddReshape();
	micro_op_resolver.AddFullyConnected();
	micro_op_resolver.AddSoftmax();
	

    // Build an interpreter to run the model with.
	// NOLINTNEXTLINE(runtime-global-variables)
    static tflite::MicroInterpreter static_interpreter(model, micro_op_resolver, tensor_arena, kTensorArenaSize);
    interpreter = &static_interpreter;

	MicroPrintf("initialized interpreter");

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
		MicroPrintf("AllocateTensors() failed");
        return;
    }

	MicroPrintf("allocated tensors");

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);

	if ((input->dims->size != 4) || // 4-dimension image input
		(input->dims->data[0] != 1) || // batch size = 1
		(input->dims->data[1] != mfcc_matrix_rows) || // num rows = 349
		(input->dims->data[2] != mfcc_matrix_cols) || // num cols = 12
		(input->dims->data[3] != 1) || // num channels = 1
		(input->type != kTfLiteInt8)) {
		MicroPrintf("Bad input tensor parameters in model");
		return;
  	}

	MicroPrintf("checked input and output tensors");

	// Place the quantized input in the model's input tensor
	for (int i = 0; i < mfcc_matrix_rows * mfcc_matrix_cols; i++) {
    	input->data.int8[i] = quantized_mfcc[i / mfcc_matrix_cols][i % mfcc_matrix_cols];
	}

	MicroPrintf("placed quantized input in model's input tensor");

    // Run inference, and report any error
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
       MicroPrintf("Invoke failed on input\n");
        return;
    }

	MicroPrintf("invoked model");

    // Obtain the quantized output from model's output tensor
    int8_t* y_quantized = output->data.int8;

	MicroPrintf("obtained quantized output from model's output tensor");

    // Output the results. A custom HandleOutput function can be implemented
    // for each supported hardware target.
    HandleOutput(y_quantized);
   
	MicroPrintf("outputted results");

	while(1);
}

int8_t** compute_mfcc() {
	while(1) {
		// Wait for samples to be read
		if (bytesRead && audio_index < length) {
			//Serial.write((byte *) sampleBuffer, bytesRead);
			
			// fill up the matrix with the audio signal chunk collected
			for (int i = 0; i < bytesRead && audio_index < length; i++) {
				audio_signal[(int) audio_index / hop_size][audio_index % hop_size] = sampleBuffer[i];
				audio_index++;
			}

			// Clear the read count
			bytesRead = 0;
		} else if (bytesRead == 0) {
			delay(0.05);
		}
		if (audio_index == length) {
			break;
		}
	}
	
	MicroPrintf("Done");

	// when audio is collected, computes MFCC
	arduinoMFCC *mymfcc = new arduinoMFCC(num_filters, frame_size, hop_size, length, num_cepstral_coeffs, frequency);

	mymfcc->compute(audio_signal);
	
	mymfcc->normalizeMFCC();

	int8_t** quantized_mfcc = mymfcc->quantizeMFCC();

	// print the quantized MFCC with micro printf
	for (int i = 0; i < mfcc_matrix_rows; i++) {
		for (int j = 0; j < mfcc_matrix_cols; j++) {
			MicroPrintf("%d, ", quantized_mfcc[i][j]);
		}
		MicroPrintf("\n");
	}
	MicroPrintf("\n");
	delete mymfcc;

	MicroPrintf("completed mfcc computation");
	return quantized_mfcc;
}

/**
 * Callback function to process the data from the PDM microphone.
 * NOTE: This callback is executed as part of an ISR.
 * Therefore using `Serial` to print messages inside this function isn't supported.
 * */
void onPDMdata() {
	// Query the number of available bytes
	bytesRead = PDM.available();

	// Read into the sample buffer
	// 16-bit, 2 bytes per sample
	PDM.read(sampleBuffer, bytesRead);
}
