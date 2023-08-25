
#include <LiquidCrystal.h> // lcd display library
#include <PDM.h> // microphone library
#include <Arduino.h>

#include <TensorFlowLite.h>


#include "model.h"
#include "output_handler.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <arduinoMFCC.h> // custom MFCC library

// Globals, used for compatibility with Arduino-style sketches.
namespace {

	const tflite::Model* model = nullptr;
	tflite::MicroInterpreter* interpreter = nullptr;
	TfLiteTensor* input = nullptr;
	TfLiteTensor* output = nullptr;

	constexpr int tensorArenaSize = 115000;
	// Keep aligned to 16 bytes for CMSIS
	uint8_t *tensor_arena;
}  // namespace


int8_t** quantized_mfccs;

// features computation
int8_t** compute_mfcc(void);

// inference function using tensorflow lite for microcontrollers
String tfmicro_inference(void);

// callback function to process the data from the PDM microphone
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

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = D10, en = D9, d4 = D8, d5 = D7, d6 = D6, d7 = D5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int ledPin = D4, buttonPin = D3;
int buttonState = 0;

void setup() {

	//Serial.begin(9600);
	pinMode(ledPin, OUTPUT); // initialize the LED pin as an output:
	pinMode(buttonPin, INPUT); // initialize the pushbutton pin as an input:
	
	// memory allocation for the audio signal shaped as 2d vector
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

	lcd.begin(16, 2);
	lcd.display();
	analogWrite(A1, 128); // contrast setting for lcd display
}

void loop() {
	delay(5000);

	// read the state of the pushbutton value:
 	buttonState = digitalRead(buttonPin);

	while (buttonState == 0) { // button pressed
		// read the state of the pushbutton value:
 		buttonState = digitalRead(buttonPin);
	}
	digitalWrite(ledPin, HIGH);
	
	MicroPrintf("Start recording\n");

	int8_t** quantized_mfcc = compute_mfcc(); // compute features

	// print the quantized MFCC with micro printf
	for (int i = 0; i < mfcc_matrix_rows; i++) {
		for (int j = 0; j < mfcc_matrix_cols; j++) {
			MicroPrintf("%d, ", quantized_mfcc[i][j]);
		}
		MicroPrintf("\n");
	}
	MicroPrintf("\n");

	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("inference...");

	String prediction = tfmicro_inference(); // inference function
   
	MicroPrintf("final output computed\n");

	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("predicted:");
	lcd.setCursor(0, 1);
	lcd.print(prediction);

	while(1); // blocks new inferences
}

// records audio and stores it into a 2d vector for memory efficiency purposes
// computes MFCC features from the recorded audio signal
// finally returns the quantized MFCC features
int8_t** compute_mfcc() {

	lcd.setCursor(0, 0);
	lcd.print("Recording");

	while(1) {
		// Wait for samples to be read
		if (bytesRead && audio_index < length) {
			//Serial.write((byte *) sampleBuffer, bytesRead);
			
			// fill up the matrix with the audio signal chunk collected
			for (int i = 0; i < (bytesRead >> 1) && audio_index < length; i++) {
				audio_signal[(int) audio_index / hop_size][audio_index % hop_size] = sampleBuffer[i];
				audio_index++;
			}

			// Clear the read count
			bytesRead = 0;
		} else if (bytesRead == 0) {
			// small delay to create some busy waiting time. This is needed so to not block the CPU
			// in this loop, and let it run the interrupt service routine onPDMdata(), which will
			// fill up the sample buffer and collect audio samples.
			// The ISR is executed only when the main processor is in idle waiting for another interrupt
			// to be handled, such as a waiting for a timer to expire or serial data communication.
			delay(0.05); 
		}
		if (audio_index == length) {
			break; // audio recording completed
		}
	}
	
	MicroPrintf("Done\n");

	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print("Computing MFCC");

	digitalWrite(ledPin, LOW); // stopped audio recording

	// when audio is collected, computes MFCC
	arduinoMFCC *mymfcc = new arduinoMFCC(num_filters, frame_size, hop_size, length, num_cepstral_coeffs, frequency);

	mymfcc->compute(audio_signal);
	
	mymfcc->normalizeMFCC();

	int8_t** quantized_mfcc = mymfcc->quantizeMFCC();

	delete mymfcc;

	MicroPrintf("completed mfcc computation\n");
	return quantized_mfcc;
}

// inference function using tensorflow lite for microcontrollers
// allocates tensor arena, loads model and runs inference with quantized integer input, output and ops
void tfmicro_inference() {
	// dynamic tensor arena allocation
	tensor_arena = new uint8_t[kTensorArenaSize];

	tflite::InitializeTarget(); // target-specific code

	MicroPrintf("initialized target\n");

    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    model = tflite::GetModel(_____model_lite_CNN_model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
		MicroPrintf("Model provided is schema version %d not equal to supported version %d.\n",
        model->version(), TFLITE_SCHEMA_VERSION);
        return;
    }

	MicroPrintf("initialized model\n");

	// Pull in only the operation implementations we need.
	// This relies on a complete list of all the ops needed by this graph.
	// An easier approach is to just use the AllOpsResolver, but this will
	// incur some penalty in code space for op implementations that are not
	// needed by this graph.
	static tflite::MicroMutableOpResolver<6> micro_op_resolver;
	micro_op_resolver.AddConv2D(); // convolution (1d and 2d)
	micro_op_resolver.AddMaxPool2D(); // max pooling
	micro_op_resolver.AddAveragePool2D(); // global average pooling
	micro_op_resolver.AddReshape(); // flatten
	micro_op_resolver.AddFullyConnected(); // dense
	micro_op_resolver.AddSoftmax(); // final layer
	

    // Build an interpreter to run the model with.
	// NOLINTNEXTLINE(runtime-global-variables)
    static tflite::MicroInterpreter static_interpreter(model, micro_op_resolver, tensor_arena, kTensorArenaSize);
    interpreter = &static_interpreter;

	MicroPrintf("initialized interpreter\n");

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
		MicroPrintf("AllocateTensors() failed\n");
        return;
    }

	MicroPrintf("allocated tensors\n");

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);

	if ((input->dims->size != 4) || // 4-dimension image input
		(input->dims->data[0] != 1) || // batch size = 1
		(input->dims->data[1] != mfcc_matrix_rows) || // num rows = 349
		(input->dims->data[2] != mfcc_matrix_cols) || // num cols = 12
		(input->dims->data[3] != 1) || // num channels = 1
		(input->type != kTfLiteInt8)) { // input type = int8
		MicroPrintf("Bad input tensor parameters in model\n");
		return;
  	}

	MicroPrintf("checked input and output tensors\n");

	
	/*
	for (int i = 0; i < mfcc_matrix_rows * mfcc_matrix_cols; i++) {
    	input->data.int8[i] = quantized_mfcc[i / mfcc_matrix_cols][i % mfcc_matrix_cols];
	}
	*/
	// Place the quantized input in the model's input tensor
	int8_t* input_data = input->data.int8;
	for (int row = 0; row < mfcc_matrix_rows; row++) {
		for (int col = 0; col < mfcc_matrix_cols; col++) {
			*input_data++ = quantized_mfcc[row][col];
		}
	}

	MicroPrintf("placed quantized input in model's input tensor\n");

    // Run inference, and report any error
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
       MicroPrintf("Invoke failed on input\n");
        return;
    }

	MicroPrintf("invoked model\n");

    // Obtain the quantized output from model's output tensor
    int8_t* y_quantized = output->data.int8;

	// get quantization parameters from output tensor
	float output_scale = output->params.scale;
	int output_zero_point = output->params.zero_point;

	MicroPrintf("obtained quantized output from model's output tensor\n");

    // Output the results. A custom HandleOutput function can be implemented
    // for each supported hardware target.
    String prediction = HandleOutput(y_quantized, output_scale, output_zero_point);
	return prediction;
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
