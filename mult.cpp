#ifdef MAC
#include "cl.hpp"
#else
#include <CL/cl.hpp>
#endif
#include <sys/time.h>
#include <fstream>
#include <iostream>
#include <algorithm>
// compare CPU vs. GPU performance (tested on AMD Ellesmere XT - RX 580)

cl::Device getDefaultDevice();        // Return the first device found
void initializeDevice();              // Initialize dev and compile kernel.
void seqMultiplyMatrices(int *a, int *b, int *c, const int M, const int N, const int K); // regular CPU matrix multiplication term by term
void gpuMultiplyMatrices(int *a, int *b, int *c, const int M, const int N, const int K); // performs the product on the GPU device
bool checkEquals(int *c1, int *c2, const int M, const int N);


cl::Program program;  // the kernel program to run
cl::Context context;  // the device context
cl::Device device;    // GPU device used for calculation


int main(){
    
    timeval t1, t2;
    const int NTIMES = 5;
    

    const int M = 256;
    const int N = 256;
    const int K = 4096;

    // input matrices A and B
    const size_t ROWS_A = M;
    const size_t COLS_A = K;
    std::vector<int> a(ROWS_A * COLS_A, 2);

    const size_t ROWS_B = K;
    const size_t COLS_B = N;
    std::vector<int> b(ROWS_B * COLS_B, 4);

    // input for CPU and GPU versions
    const size_t ROWS_C = M;
    const size_t COLS_C = N;
    std::vector<int> cs(ROWS_C * COLS_C);
    std::vector<int> cp(ROWS_C * COLS_C);


    // using the CPU
    gettimeofday(&t1, NULL);
    for(int i = 0; i < NTIMES; i++){
        seqMultiplyMatrices(a.data(), b.data(), cs.data(), M, N, K);
    }
    gettimeofday(&t2, NULL);
    long seconds = (t2.tv_sec - t1.tv_sec);
    long micros = ((seconds * 1000000) + t2.tv_usec) - (t1.tv_usec);
    double seqTime = (double)seconds + (double)micros/1000000;


    initializeDevice();

    // do GPU multiplication
    gettimeofday(&t1, NULL);
    for (int i = 0; i < NTIMES; i++){
        gpuMultiplyMatrices(a.data(), b.data(), cp.data(), M, N, K);
    }
    gettimeofday(&t2, NULL);
    long secondsGPU = (t2.tv_sec - t1.tv_sec);
    long microsGPU = ((seconds * 1000000) + t2.tv_usec) - (t1.tv_usec);
    double gpuTime = (double)secondsGPU + (double)microsGPU/1000000;


    bool equal = checkEquals(cs.data(), cp.data(), ROWS_C, COLS_C);

    std::cout << "Status: " << (equal ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "Results: \n\tA[0] = " << a[0] << "\n\tB[0] = " << b[0] << "\n\tC[0] = " << cp[0] << std::endl;
    std::cout << "Mean execution time: \n\t1 CPU: " << seqTime << " sec;\n\tGPU: " << gpuTime << " sec." << std::endl;
    std::cout << "Performance gain: " << ((seqTime - gpuTime) / gpuTime) << "x" << std::endl;

    return 0;

}





cl::Device getDefaultDevice() {
    
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty()) {
        std::cerr << "No platforms found!" << std::endl;
        exit(1);
    }

    auto platform = platforms.front();
    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    for (int i = 0; i < devices.size(); i++) {
	    size_t valueSize;
	    clGetDeviceInfo(devices[i](), CL_DEVICE_NAME, 0, NULL, &valueSize);
	    char *value = (char *)malloc(valueSize);
	    clGetDeviceInfo(devices[i](), CL_DEVICE_NAME, valueSize, value, NULL);
	    std::cout << "device " << i << ": " << value << std::endl;
	    free(value);
    }

    if (devices.empty()) {
        std::cerr << "No devices found!" << std::endl;
        exit(1);
    }

    return devices.front();
}


void initializeDevice() {

    device = getDefaultDevice();
    
    std::ifstream kernel_file("mult.cl");
    std::string src(std::istreambuf_iterator<char>(kernel_file), (std::istreambuf_iterator<char>()));


    // compile kernel

    cl::Program::Sources sources(1, std::make_pair(src.c_str(), src.length() + 1));
    context = cl::Context(device);
    program = cl::Program(context, sources);
    
    auto err = program.build();
    if (err != CL_BUILD_SUCCESS) {
        std::cerr << "Error!\nBuild Status: " << program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(device) 
        << "\nInformation: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
        exit(1);
    }
}


void seqMultiplyMatrices(int *a, int *b, int *c, const int M, const int N, const int K) {
    #pragma omp parallel for
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < K; k++) {
                sum += a[i*K + k] * b[j + k*N];
            }
            c[i*N + j] = sum;
        }
    }
}


void gpuMultiplyMatrices(int *a, int *b, int *c, const int M, const int N, const int K) {

    cl::Buffer aBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, M * K * sizeof(int), a);
    cl::Buffer bBuf(context, CL_MEM_READ_ONLY | CL_MEM_HOST_NO_ACCESS | CL_MEM_COPY_HOST_PTR, K * N * sizeof(int), b);
    cl::Buffer cBuf(context, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, M * N * sizeof(int));

    cl::Kernel kernel(program, "matrixMultiply");
    kernel.setArg(0, aBuf);
    kernel.setArg(1, bBuf);
    kernel.setArg(2, cBuf);
    kernel.setArg(3, sizeof(unsigned int), &M);
    kernel.setArg(4, sizeof(unsigned int), &N);
    kernel.setArg(5, sizeof(unsigned int), &K);


    cl::CommandQueue queue(context, device, CL_QUEUE_PROFILING_ENABLE);
    queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(N, M));
    queue.enqueueReadBuffer(cBuf, CL_TRUE, 0, M * N * sizeof(int), c);
    queue.finish();

}


bool checkEquals(int *c1, int *c2, const int M, const int N) {
    for (int i = 0; i < M*N; i++) {
        if (c1[i] != c2[i]) {
            return false;
        }
    }
    return true;
}

