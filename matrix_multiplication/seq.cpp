#include <iostream>
#include <sys/time.h>
#include "utils.h"

using namespace std;

int main(int argc, char** argv){
    int N;
    parseArgs(argc, argv, N);
    size_t allocSize = N*N*sizeof(float);
    float* A = (float*)malloc(allocSize);
    float* B = (float*)malloc(allocSize);
    float* C = (float*)malloc(allocSize);

    initializeArray(A, N);
    initializeArray(B, N);

    int i,j,k;
    struct timeval tv1, tv2;
    struct timezone tz;
    double elapsed; 
    
    gettimeofday(&tv1, &tz);

    // sequential kernel start
    for(i=0;i<N;i++)
    {
        for(j=0;j<N;j++)
        {
            for(k=0;k<N;k++)
            {
                C[i*N + j] += A[i*N + k] * B[k*N + j];
            }
        }
    }

    gettimeofday(&tv2, &tz);
    /*
    bool all_good = checkCorrectness(C, N);
    if(all_good)
        cout << "Correctness check passed" << endl;
    else
        cout << "Correctness check failed" << endl;

    elapsed = (double) (tv2.tv_sec-tv1.tv_sec) + (double) (tv2.tv_usec-tv1.tv_usec) * 1.e-6;
    printf("elapsed time = %f seconds.\n", elapsed); */
}
