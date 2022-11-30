#include <iostream>
#include <sys/time.h>
#include <math.h>
#include "utils.h"

using namespace std;

int main(int argc, char** argv){
    int N;
    parseArgs(argc, argv, N);
    bool* is_composite = (bool*)calloc(N+1, sizeof(bool));

    struct timeval tv1, tv2;
    struct timezone tz;
    double elapsed; 
    
    gettimeofday(&tv1, &tz);
    // Sieve of Eratosthenes
    int i = 2;
    while(i <= ceil(sqrt(N))){
        if(!is_composite[i]){
            int j = i * i;
            while(j <= N){
                is_composite[j] = true;
                j += i;
            }
        }
        i++;
    }
    gettimeofday(&tv2, &tz);
    elapsed = (double) (tv2.tv_sec-tv1.tv_sec) + (double) (tv2.tv_usec-tv1.tv_usec) * 1.e-6;
    printf("elapsed time = %f seconds.\n", elapsed);

    return 0;
}