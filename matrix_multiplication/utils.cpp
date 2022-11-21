#include <iostream>

void initializeArray(float* array, int N){
    // all elements of array are initialized to 1.0 (so that correctness can be checked easily)
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            array[i*N + j] = 1.0;
        }
    }
}

void parseArgs(int argc, char **argv, int &N){
    N = 1000;
    if(argc > 1)
        N = atoi(argv[1]);
}

bool checkCorrectness(float* array, int N){
    // the product of 2 arrays that we initialized must be equal to N
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            if(array[i*N + j] != N)
                return false;
        }
    }
    return true;
}

void printArr(float* array, int N){
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            std::cout << array[i*N + j] << " ";
        }
        std::cout << std::endl;
    }
}