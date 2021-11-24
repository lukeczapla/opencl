
CPP_FLAGS=-lOpenCL -DCL_TARGET_OPENCL_VERSION=220
CPP=g++

all: mult

clean: 
	rm mult.o mult


mult: mult.o
	$(CPP) -o mult mult.o $(CPP_FLAGS)

mult.o: mult.cpp
	$(CPP) -c mult.cpp $(CPP_FLAGS)

