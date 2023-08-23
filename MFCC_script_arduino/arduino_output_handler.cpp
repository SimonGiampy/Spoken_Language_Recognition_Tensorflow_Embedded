/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

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

#include "output_handler.h"
#include <Arduino.h>

String HandleOutput(int8_t* output_prediction, float scale, int zero_point) {

	float y_predicted[3];
	for (int i = 0; i < 3; i++) {
		y_predicted[i] = round( ((float) output_prediction[i]) / scale) + (float) zero_point;
	}

	Serial.print("output 0 (ita) = ");
	Serial.print(y_predicted[0], 4);
	Serial.print("; output 1 (eng) = ");
	Serial.print(y_predicted[1], 4);
	Serial.print("; output 2 (fra) = ");
	Serial.print(y_predicted[2], 4);
	Serial.println(".");

	
	// compute argmax of y predicted
	float max = y_predicted[0];
	int argmax = 0;
	for (int i = 1; i < 3; i++) {
		if (y_predicted[i] > max) {
			max = y_predicted[i];
			argmax = i;
		}
	}

	// show on lcd first 2 max probabilities
	String langs[3] = { "ita", "eng", "fra" };

	return langs[argmax];
	
}
