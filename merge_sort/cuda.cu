#include <iostream>
#include "util.h"
#include <stdio.h>
#include <cuda.h>

using namespace std;

__global__ void merge_sort_kernel(int*, int*, int, int);

int main(int argc, char** argv){
    int N;
    parseArgs(argc, argv, N);

    // initialize array
    size_t allocSize = N * sizeof(int);
    int *arr = (int*) malloc(allocSize);
    int *tmparr = (int*) malloc(allocSize);

    initializeArray(arr, N);
    
    int *d_arr, *d_tmparr;
    cudaMalloc((void**)&d_arr, allocSize);
    cudaMalloc((void**)&d_tmparr, allocSize);

    cudaMemcpy(d_arr, arr, allocSize, cudaMemcpyHostToDevice);
    cudaMemcpy(d_tmparr, tmparr, allocSize, cudaMemcpyHostToDevice);

    int curr_size;
    for (curr_size=1; curr_size <= N-1; curr_size = 2*curr_size){
        // call kernel
        int K = 1024;
        int num_blocks = ceil((float)N/(float)(K * 2 * curr_size)) + 1;
        int num_threads = K;
        dim3 dimGrid(num_blocks, 1, 1);
        dim3 dimBlock(num_threads, 1, 1);

        merge_sort_kernel<<<dimGrid, dimBlock>>>(d_arr, d_tmparr, curr_size, N);
        cudaMemcpy(d_tmparr, d_arr, allocSize, cudaMemcpyDeviceToDevice);
    }
    cudaMemcpy(arr, d_arr, allocSize, cudaMemcpyDeviceToHost);
    cudaFree(d_arr);
    cudaFree(d_tmparr);

    // bool is_correct = checkCorrectness(arr, N);
    // if (is_correct){
    //     cout << "Correct!" << endl;
    // } else {
    //     cout << "Incorrect!" << endl;
    //     // printArray(arr, N);
    // }

}

__global__ void merge_sort_kernel(int* arr, int* tmp_arr_d, int curr_size, int N){
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int left = i * 2 * curr_size;
    if(left > N-1){
        return;
    }
    
    int right = left + curr_size;
    int end = min(right + curr_size - 1, N);

    int left_start = left, right_start = right;
    int left_end = right - 1, right_end = end;
    int result_start = left_start;

    while(left_start < left_end || right_start < right_end){
        if(left_start < left_end && right_start < right_end)
        {
            if(arr[left_start] < arr[right_start]){
                tmp_arr_d[result_start] = arr[left_start];
                left_start += 1;
            }
            else{
                tmp_arr_d[result_start] = arr[right_start];
                right_start += 1;
            }
        }
        else if(left_start < left_end){
            tmp_arr_d[result_start] = arr[left_start];
            left_start += 1;
        }
        else{
            tmp_arr_d[result_start] = arr[right_start];
            right_start += 1;
        }
        
        result_start += 1;
    }

}