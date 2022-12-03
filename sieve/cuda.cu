// sieve of eratosthenes on cuda
#include <iostream>
#include <cuda.h>
#include <math.h>
#include <sys/time.h>
#include "utils.h"

using namespace std;

#define BLOCK_SIZE 1024

__global__ void sieve(bool*, int, int, int, int);

int main(int argc, char** argv){
    int N;
    parseArgs(argc, argv, N);
    bool* is_composite = (bool*)calloc(N+1, sizeof(bool));

    struct timeval tv1, tv2;
    struct timezone tz;
    double elapsed; 

    bool* d_is_composite;
    cudaMalloc((void**)&d_is_composite, (N+1)*sizeof(bool));
    cudaMemcpy(d_is_composite, is_composite, (N+1)*sizeof(bool), cudaMemcpyHostToDevice);

    gettimeofday(&tv1, &tz);
    for(int i = 2; i <= sqrt(N); i++){
        if(!is_composite[i]){
            int start = i*i;
            int end = N;
            int step = i;
            int num_blocks = (end-start)/step/BLOCK_SIZE + 1;
            sieve<<<num_blocks, BLOCK_SIZE>>>(d_is_composite, start, end, step, N);
            cudaDeviceSynchronize();
        }
    }
    gettimeofday(&tv2, &tz);
    elapsed = (tv2.tv_sec-tv1.tv_sec) + (tv2.tv_usec-tv1.tv_usec)/1000000.0;
    cout << "Time: " << elapsed << endl;
    cudaMemcpy(is_composite, d_is_composite, (N+1)*sizeof(bool), cudaMemcpyDeviceToHost);
    cudaFree(d_is_composite);
    // printArr(is_composite, N);
    // cout << "Number of primes: " << count << endl;
}

__global__ void sieve(bool* is_composite, int start, int end, int step, int N){
    int idx = blockIdx.x*blockDim.x + threadIdx.x;
    int num = start + idx*step;
    if(num <= N){
        is_composite[num] = true;
    }
}