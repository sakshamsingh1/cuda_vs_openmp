#include "util.h"
#include <cstdlib>
#include <stdio.h>
#include <iostream>

bool checkCorrectness(int *arr, int arrSize) {
    for (int i = 0; i < arrSize - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return 0;
        }
    }
    return 1;
}

void parseArgs(int argc, char **argv, int& arrSize) {
    arrSize = 1000;
    if(argc > 1) {
        arrSize = atoi(argv[1]);
    }
}

void initializeArray(int *arr, int N) {
    for (int i = 0; i < N; i++) {
        arr[i] = rand()%10;
    }
}

void printArray(int *arr, int N) {
    for (int i=0; i < N; i++)
        printf("%d ", arr[i]);
    printf("\n");
}
