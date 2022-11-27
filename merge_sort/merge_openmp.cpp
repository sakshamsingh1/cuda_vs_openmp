#include <cstdio>
#include <omp.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <algorithm>
#include "util.h"

void merge(int arr[], int l, int m, int r, int* tempBuffer) {

    int lSize = m - l + 1, rSize = r - m;
    int *left = &tempBuffer[l], *right = &tempBuffer[m + 1];

    // Copy elements to temporary array
    for(int i = 0; i < lSize; i++)
        left[i] = arr[l + i];
    for(int i = 0; i < rSize; i++)
        right[i] = arr[m + 1 + i];

    // Merge temporary arrays into original array
    int i = 0, j = 0, k = l;
    while (i < lSize && j < rSize) {
        if (left[i] <= right[j]) {
            arr[k++] = left[i++];
        } else {
            arr[k++] = right[j++];
            j++;
        }
    }

    while (i < lSize) arr[k++] = left[i++];
    while (j < rSize) arr[k++] = right[j++];
}

void mergeSort(int* arr, int l, int r, int* tempBuffer) {
    if (l < r) {
        int m = l + (r - l) / 2;
        mergeSort(arr, l, m, tempBuffer);
        mergeSort(arr, m + 1, r, tempBuffer);
        merge(arr, l, m, r, tempBuffer);
    }
}

int main(int argc, char **argv) {
    int arrSize;
    parseArgs(argc, argv, arrSize);
    int arrSizeBytes = arrSize * sizeof(int);

    int *arr = (int*)malloc(arrSizeBytes);
    initializeRandomArray(arr, arrSize);
    int *tempBuffer = (int*)malloc(arrSizeBytes);

    omp_set_nested(1);
    #pragma omp target data map(tofrom:tempBuffer[0:arrSize]) map(tofrom:arr[0:arrSize])
    {
        int vectorLengthPerThread = 2;

        while(vectorLengthPerThread < arrSize) {
            #pragma omp target
            {
                #pragma omp parallel for
                for(int i=0; i < arrSize; i+= vectorLengthPerThread){
                    int threadNum = omp_get_thread_num();
                    int l = threadNum * vectorLengthPerThread;
                    int r = std::min(l + vectorLengthPerThread - 1, arrSize);
                    mergeSort(arr, l, r, tempBuffer);
                }
            }
            vectorLengthPerThread *= 2;
        }

        # pragma omp target update from(arr[0:arrSize])
        # pragma omp single
        {
            int l = vectorLengthPerThread / 2, r = arrSize;
            mergeSort(arr, l, r, tempBuffer);
            merge(arr, 0, vectorLengthPerThread / 2 - 1, arrSize, tempBuffer);
        }
    }

    return 0;
}
