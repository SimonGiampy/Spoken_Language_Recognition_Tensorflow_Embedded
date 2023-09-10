# Spoken_Language_Recognition_Tensorflow_Embedded

#### An artificial neural network using mel spectrograms to recognize the language from human conversations, running with Tensorflow Micro on an Arduino Nano 33 BLE Sense

Project for the course Hardware Architecture for Embedded and Edge AI at Politecnico di Milano, A.Y. 2022-2023

Devoloped by:
- Simone Giampà
- Claudio Galimberti

## Project summary

The goal of this project is to develop a neural network able to recognize the language spoken by a person, given a short audio clip of his/her voice. The network is trained on a dataset that we produced on our own. This dataset contains audio clips of people speaking in 3 languages: italian, english and french. The network is trained on a modified version of the mel spectrograms: MFCC (mel frequency cepstral coefficients) extracted from the audio clips, and it is able to tell apart the 3 different languages. This convolutional neural network runs on an Arduino Nano 33 BLE Sense, using Tensorflow Lite for Microcontrollers library

The dataset is kept private for privacy purposes.

## Project structure

The project is structured as follows:

- [Arduino audio recorder](arduino&#32;audio&#32;recorders/): contains arduino code meant to record audio data and store it on the SD card of the Arduino Nano 33 BLE Sense, or send it to a computer via serial port
- [Audio recording notebooks](data_recorder_notebooks/): contains jupyter notebooks used to transform binary data into WAV audio files, and process them. They can also listen to a serial port and record audio data from it, saving the raw data in a file
- [Course Noteooks](haeeai_course_notebooks/): contains jupyter notebooks used to train the neural network, and to test it. Taken from the professor of our university course
- [arduinoMFCC library](libraries/arduinoMFCC/): contains the arduino library used to extract MFCC from audio data. This library can be executed both in an arduino environment and in a computer linux environment
- [C++ MFCC scripts](MFCC_script_cpp/): contains a C++ script that uses the arduinoMFCC library to extract MFCC from audio data. It can be executed in a linux environment
- [Arduino TFMicro Inference with MFCC](MFCC_script_arduino/): contains an arduino script that uses the arduinoMFCC library to extract MFCC from audio data. It can be executed in an arduino environment. It also uses a tfmicro model to make inference about a short audio clip recorded from the microcontroller itself. This folder contains the core of this project.
- [Models](model_lite/): contains the Tensorflow model, the Tensorflow Lite model and the TFMicro model (C bytes array) used to make inference on the Arduino Nano 33 BLE Sense
- [dataset_creator](dataset_creator.ipynb): jupyter notebook used to create the dataset and the splitting between training and validation
- [training_network](training_net.ipynb): jupyter notebook used to train the neural network, quantize the model into TFLite and convert to TFMicro. Then evaluates the model on the different test sets created ad hoc
- [Report](Spoken&#32;Language&#32;Recognition&#32;-&#32;Project&#32;Report.pdf): report of the project
- [Presentation](Spoken&#32;Language&#32;Recognition&#32;-&#32;Project&#32;Presentation.pptx): presentation of the project