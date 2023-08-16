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


#include "tensorflow/lite/c/common.h"


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



// Called by the main loop to produce some output based on the x and y values
void HandleOutput(int8_t* output_prediction);

