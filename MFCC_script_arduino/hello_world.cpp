/* Copyright 2023 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <TensorFlowLite.h>

#include "model.h"
#include "output_handler.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_log.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"


// Globals, used for compatibility with Arduino-style sketches.
namespace {
	const tflite::Model* model = nullptr;
	tflite::MicroInterpreter* interpreter = nullptr;
	TfLiteTensor* input = nullptr;
	TfLiteTensor* output = nullptr;

	const int kTensorArenaSize = 2000;
	// Keep aligned to 16 bytes for CMSIS
	uint8_t tensor_arena[kTensorArenaSize];
}  // namespace


#include "input_data.h"

int main() {
	setup();
	loop();
}

int8_t** quantized_mfccs;


// The name of this function is important for Arduino compatibility.
void setup() {

	audio_test = new int16_t[length];
    readBinary("test_audio.bin");
	int16_t **audio_test_matrix = reshapeVector(audio_test);
	delete[] audio_test;  // frees from memory original audio vector
    quantized_mfccs = compute_mfcc(audio_test_matrix);

    tflite::InitializeTarget(); // target-specific code

    // Map the model into a usable data structure. This doesn't involve any
    // copying or parsing, it's a very lightweight operation.
    model = tflite::GetModel(_____model_lite_CNN_model_tflite);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        PRINT_DEBUG("Model provided is schema version" + std::to_string(model->version()) +
		 " not equal to supported version " + std::to_string(TFLITE_SCHEMA_VERSION));
        return;
    }

    // This pulls in all the operation implementations we need.
    // NOLINTNEXTLINE(runtime-global-variables)
    static tflite::AllOpsResolver resolver;

	/*
	static tflite::MicroMutableOpResolver<5> micro_op_resolver;
	micro_op_resolver.AddAveragePool2D();
	micro_op_resolver.AddConv2D();
	micro_op_resolver.AddFullyConnected();
	micro_op_resolver.AddSoftmax();

	// Build an interpreter to run the model with.
	// NOLINTNEXTLINE(runtime-global-variables)
	static tflite::MicroInterpreter static_interpreter(
		model, micro_op_resolver, tensor_arena, kTensorArenaSize);
	*/

    // Build an interpreter to run the model with.
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensor_arena, kTensorArenaSize);
    interpreter = &static_interpreter;

    // Allocate memory from the tensor_arena for the model's tensors.
    TfLiteStatus allocate_status = interpreter->AllocateTensors();
    if (allocate_status != kTfLiteOk) {
        PRINT_DEBUG("AllocateTensors() failed");
        return;
    }

    // Obtain pointers to the model's input and output tensors.
    input = interpreter->input(0);
    output = interpreter->output(0);

	if ((input->dims->size != 4) || // 4-dimension image input
		(input->dims->data[0] != 1) || // batch size = 1
		(input->dims->data[1] != mfcc_matrix_rows) || // num rows = 349
		(input->dims->data[2] != mfcc_matrix_cols) || // num cols = 12
		(input->dims->data[3] != 1) || // num channels = 1
		(input->type != kTfLiteInt8)) {
		PRINT_DEBUG("Bad input tensor parameters in model");
		return;
  	}

}

// The name of this function is important for Arduino compatibility.
void loop() {

    // Place the quantized input in the model's input tensor
	for (int i = 0; i < mfcc_matrix_rows * mfcc_matrix_cols; i++) {
    	input->data.int8[i] = quantized_mfccs[i / mfcc_matrix_cols][i % mfcc_matrix_cols];
	}

    // Run inference, and report any error
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
        PRINT_DEBUG("Invoke failed on input\n");
        return;
    }

    // Obtain the quantized output from model's output tensor
    int8_t* y_quantized = output->data.int8;

    // Output the results. A custom HandleOutput function can be implemented
    // for each supported hardware target.
    HandleOutput(y_quantized);
   
}
