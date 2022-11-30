#include <iostream>

void parseArgs(int argc, char **argv, int &N){
    N = 1000;
    if(argc > 1)
        N = atoi(argv[1]);
}

void printArr(bool* is_composite, int N){
    for(int i = 2; i < N; i++){
        if(!is_composite[i])
            std::cout << i << std::endl;
    }
}