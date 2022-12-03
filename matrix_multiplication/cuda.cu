#include <iostream>
#include <sys/time.h>
#include "utils.h"
#define TILE_DIM 16

using namespace std;

__global__ void mm_kernel(float*, float*, float*, int);

int main(int argc, char** argv){
    int N;
    parseArgs(argc, argv, N);
    size_t allocSize = N*N*sizeof(float);
    
    float* A = (float*)malloc(allocSize);
    float* B = (float*)malloc(allocSize);
    float* C = (float*)malloc(allocSize);

    initializeArray(A, N);
    initializeArray(B, N);

    struct timeval tv1, tv2;
    struct timezone tz;
	double elapsed; 
    
    // Copy array to device
    float* A_c = NULL;
    float* B_c = NULL;
    float* C_c = NULL;
    
	cudaMalloc((void**)&A_c, allocSize);
	cudaMemcpy(A_c, A, allocSize, cudaMemcpyHostToDevice);

    cudaMalloc((void**)&B_c, allocSize);
	cudaMemcpy(B_c, B, allocSize, cudaMemcpyHostToDevice);

    cudaMalloc((void**)&C_c, allocSize);


    // Launch kernel
    int bx = ceil(N/(float)TILE_DIM);
    int by = ceil(N/(float)TILE_DIM);
    dim3 dimGrid(bx, by, 1);
    dim3 dimBlock(TILE_DIM, TILE_DIM, 1);

    gettimeofday(&tv1, &tz);
    cudaThreadSynchronize();
    mm_kernel<<<dimGrid, dimBlock>>>(A_c, B_c, C_c, N);
    cudaThreadSynchronize();
    gettimeofday(&tv2, &tz);

    // Copy result back to host
    cudaMemcpy(C, C_c, allocSize, cudaMemcpyDeviceToHost);

    cudaFree(A_c);
    cudaFree(B_c);
    cudaFree(C_c);

    // printArr(C, N);
/*    bool all_good = checkCorrectness(C, N);
    if(all_good)
        cout << "Correctness check passed" << endl;
    else
        cout << "Correctness check failed" << endl;
 */   
    elapsed = (double) (tv2.tv_sec-tv1.tv_sec) + (double) (tv2.tv_usec-tv1.tv_usec) * 1.e-6;
    printf("elapsed time = %f seconds.\n", elapsed);
}

__global__ void mm_kernel(float* A, float* B, float* C, int N){
    int row = blockIdx.y*blockDim.y + threadIdx.y;
    int col = blockIdx.x*blockDim.x + threadIdx.x;

    if(row >= N || col >= N)
        return;
    
    float sum = 0;
    for(int i = 0; i < N; i++){
        sum += A[row*N + i]*B[i*N + col];
    }
    C[row*N + col] = sum;
}
