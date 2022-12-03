#include <iostream>
#include <sys/time.h>
#include "utils.h"

#define TILE_DIM 16

using namespace std;

__global__ void mm_kernel_tiled(float*, float*, float*, int);

// matrix multiplication tiling cuda
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
    mm_kernel_tiled<<<dimGrid, dimBlock>>>(A_c, B_c, C_c, N);
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

// Kernel
__global__ void mm_kernel_tiled(float* A, float* B, float* C, int N){

    float CValue = 0;

    int Row = blockIdx.y*TILE_DIM + threadIdx.y;
    int Col = blockIdx.x*TILE_DIM + threadIdx.x;

    __shared__ float As[TILE_DIM][TILE_DIM];
    __shared__ float Bs[TILE_DIM][TILE_DIM];

    for (int k = 0; k < (TILE_DIM + N - 1)/TILE_DIM; k++) {

         if (k*TILE_DIM + threadIdx.x < N && Row < N)
             As[threadIdx.y][threadIdx.x] = A[Row*N + k*TILE_DIM + threadIdx.x];
         else
             As[threadIdx.y][threadIdx.x] = 0.0;

         if (k*TILE_DIM + threadIdx.y < N && Col < N)
             Bs[threadIdx.y][threadIdx.x] = B[(k*TILE_DIM + threadIdx.y)*N + Col];
         else
             Bs[threadIdx.y][threadIdx.x] = 0.0;

         __syncthreads();

         for (int n = 0; n < TILE_DIM; ++n)
             CValue += As[threadIdx.y][n] * Bs[n][threadIdx.x];

         __syncthreads();
    }

    if (Row < N && Col < N)
        C[((blockIdx.y * blockDim.y + threadIdx.y)*N) +
           (blockIdx.x * blockDim.x)+ threadIdx.x] = CValue;
}
