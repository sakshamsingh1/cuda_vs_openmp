#include "util.h"
#include <cstdlib>
#include <stdio.h>
#include <ctime>
#include <iostream>

void checkCorrectness(int *arr, int arrSize) {
    for (int i = 0; i < arrSize - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            printf("\nError\n");
            return;
        }
    }
    printf("\nOk\n");
}

void parseArgs(int argc, char **argv, int& arrSize) {
    arrSize = 1000; // default size
    if(argc > 1) {
        arrSize = atoi(argv[1]);
    }
}

void initializeRandomArray(int *arr, int arrSize) {
    (time(NULL));
    for (int i = 0; i < arrSize; i++) {
        arr[i] = rand() % 1000000;
    }
}

void printArray(int *arr, int arrSize) {
    for (int i=0; i < arrSize; i++)
        printf("%d ", arr[i]);
    printf("\n");
}
