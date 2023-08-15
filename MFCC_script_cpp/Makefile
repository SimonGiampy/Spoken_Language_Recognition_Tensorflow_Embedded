CPPFLAGS = -Wall -std=c++17 -O3
dir = $(shell dirname $(PWD))
library = $(dir)/libraries
inc = $(library)/arduinoMFCC/
LIBRARY_FILES = $(inc)arduinoMFCC.cpp $(inc)kiss_fft.c $(inc)kiss_fftr.c

RM = rm -f

compile:
	g++ $(CPPFLAGS) -I$(library) $(LIBRARY_FILES) MFCC_Preprocessing.cpp -o output.o 

run:
	./output.o

clean: 
	$(RM) output.o