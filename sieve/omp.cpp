#include <iostream>
#include <omp.h>
#include <sys/time.h>
#include <math.h>
#include "utils.h"

using namespace std;


int main(int argc, char** argv) 
{
    int N;
    parseArgs(argc, argv, N);
    bool* is_composite = (bool*)calloc(N+1, sizeof(bool));

    struct timeval tv1, tv2;
    struct timezone tz;
    double elapsed; 
    
    gettimeofday(&tv1, &tz);
    
    int lastNumberSqrt = ceil(sqrt(N));
    #pragma omp target data map (from: is_composite[0:N]) 
    for (int i = 2; i <= lastNumberSqrt; i += 1){
      if (!is_composite[i]){
        #pragma omp target teams distribute parallel for
        for (int j = i*i; j <= N; j += i){
          is_composite[j] = 1;
        }
      }
    }
    
  
    gettimeofday(&tv2, &tz);
    elapsed = (double) (tv2.tv_sec-tv1.tv_sec) + (double) (tv2.tv_usec-tv1.tv_usec) * 1.e-6;
    printf("elapsed time = %f seconds.\n", elapsed);
    printArr(is_composite, N);

}