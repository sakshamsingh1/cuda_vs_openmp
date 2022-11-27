#include <iostream>
#include <sys/time.h>
#include "util.h"

void mergesort(int*, long, dim3, dim3);

__global__ void gpu_mergesort(int*, int*, long, long, long, dim3*, dim3*);
__device__ void gpu_bottomUpMerge(int*, int*, long, long, long);

#define min(a, b) (a < b ? a : b)

int main(int argc, char** argv) {

    int size;
    parseArgs(argc, argv, size);

    int *arr = (int*)malloc(size * sizeof(int));
    struct timeval tv1, tv2;
    struct timezone tz;
    double elapsed;

    initializeRandomArray(arr, size);

    dim3 threadsPerBlock(128,32,1);
    dim3 blocksPerGrid(128,32,1);

//    threadsPerBlock.x = 512;
//    threadsPerBlock.y = 512;
//    threadsPerBlock.z = 1;
//
//    blocksPerGrid.x = 512;
//    blocksPerGrid.y = 512;
//    blocksPerGrid.z = 1;

    gettimeofday(&tv1, &tz);
    mergesort(arr, size, threadsPerBlock, blocksPerGrid);
    gettimeofday(&tv2, &tz);
    elapsed = (double) (tv2.tv_sec-tv1.tv_sec) + (double) (tv2.tv_usec-tv1.tv_usec) * 1.e-6;
    printf("elapsed time = %f seconds.\n", elapsed);
}

void mergesort(int* data, long size, dim3 threadsPerBlock, dim3 blocksPerGrid) {
    int* D_data;
    int* D_swp;
    dim3* D_threads;
    dim3* D_blocks;
    
    // Actually allocate the two arrays
    cudaMalloc((void**) &D_data, size * sizeof(int));
    cudaMalloc((void**) &D_swp, size * sizeof(int));

    // Copy from our input list into the first array
    cudaMemcpy(D_data, data, size * sizeof(int), cudaMemcpyHostToDevice);

    cudaMalloc((void**) &D_threads, sizeof(dim3));
    cudaMalloc((void**) &D_blocks, sizeof(dim3));

    cudaMemcpy(D_threads, &threadsPerBlock, sizeof(dim3), cudaMemcpyHostToDevice);
    cudaMemcpy(D_blocks, &blocksPerGrid, sizeof(dim3), cudaMemcpyHostToDevice);

    int* A = D_data;
    int* B = D_swp;

    long nThreads = threadsPerBlock.x * threadsPerBlock.y * threadsPerBlock.z *
                    blocksPerGrid.x * blocksPerGrid.y * blocksPerGrid.z;

    for (int width = 2; width < (size << 1); width <<= 1) {
        long slices = size / ((nThreads) * width) + 1;

        gpu_mergesort<<<blocksPerGrid, threadsPerBlock>>>(A, B, size, width, slices, D_threads, D_blocks);

        A = A == D_data ? D_swp : D_data;
        B = B == D_data ? D_swp : D_data;
    }

    cudaMemcpy(data, A, size * sizeof(long), cudaMemcpyDeviceToHost);
    
    // Free the GPU memory
    cudaFree(A);
    cudaFree(B);
}

__device__ unsigned int getIdx(dim3* threads, dim3* blocks) {
    int x;
    return threadIdx.x +
           threadIdx.y * (x  = threads->x) +
           threadIdx.z * (x *= threads->y) +
           blockIdx.x  * (x *= threads->z) +
           blockIdx.y  * (x *= blocks->z) +
           blockIdx.z  * (x *= blocks->y);
}

__global__ void gpu_mergesort(int* source, int* dest, long size, long width, long slices, dim3* threads, dim3* blocks) {
    unsigned int idx = getIdx(threads, blocks);
    long start = width*idx*slices, 
         middle, 
         end;

    for (long slice = 0; slice < slices; slice++) {
        if (start >= size)
            break;

        middle = min(start + (width >> 1), size);
        end = min(start + width, size);
        gpu_bottomUpMerge(source, dest, start, middle, end);
        start += width;
    }
}

__device__ void gpu_bottomUpMerge(int* source, int* dest, long start, long middle, long end) {
    long i = start;
    long j = middle;
    for (long k = start; k < end; k++) {
        if (i < middle && (j >= end || source[i] < source[j])) {
            dest[k] = source[i];
            i++;
        } else {
            dest[k] = source[j];
            j++;
        }
    }
}