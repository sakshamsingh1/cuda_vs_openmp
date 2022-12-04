#include "util.h"
#include <omp.h>
#include <iostream>
#include <sys/time.h>

using namespace std;

void merge(int *arr, int left, int mid, int right, int* tmp_array)
{
    
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int *L = &tmp_array[left];
    int *R = &tmp_array[mid + 1];

    for (i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    i = 0;
    j = 0;
    k = left;

    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
        {
            arr[k] = L[i];
            i++;
        }
        else
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1)
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    while (j < n2)
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}

/** Adapted from https://www.geeksforgeeks.org/iterative-merge-sort/ */
/* Iterative mergesort function to sort arr[0...n-1] */
void mergeSort(int* arr, int n, int* tmp_array)
{
   int curr_size;  // For current size of subarrays to be merged
                   // curr_size varies from 1 to n/2
   int left_start; // For picking starting index of left subarray
                   // to be merged
 
   // Merge subarrays in bottom up manner.  First merge subarrays of
   // size 1 to create sorted subarrays of size 2, then merge subarrays
   // of size 2 to create sorted subarrays of size 4, and so on.
   
   for (curr_size=1; curr_size<=n-1; curr_size = 2*curr_size)
   {
       // Pick starting point of different subarrays of current size
        #pragma omp parallel for
       for (left_start=0; left_start<n-1; left_start += 2*curr_size)
       {
           // Find ending point of left subarray. mid+1 is starting
           // point of right
           int mid = min(left_start + curr_size - 1, n-1);
 
           int right_end = min(left_start + 2*curr_size - 1, n-1);
 
           // Merge Subarrays arr[left_start...mid] & arr[mid+1...right_end]
           merge(arr, left_start, mid, right_end, tmp_array);
       }
   }
}

int main(int argc, char **argv) {
    int N;
    parseArgs(argc, argv, N);

    struct timeval tv1, tv2;
    struct timezone tz;
    double elapsed;

    // initialize array
    size_t allocSize = N * sizeof(int);
    int *arr = (int*) malloc(allocSize);
    int *tmp_array = (int*) malloc(allocSize);

    initializeArray(arr, N);
    // printArray(arr, N);
    gettimeofday(&tv1, &tz);
    #pragma omp target data map(tofrom: tmp_array[0:N]) map(tofrom:arr[0:N])
    #pragma omp target
    mergeSort(arr, N, tmp_array);

    gettimeofday(&tv2, &tz);
    elapsed = (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec) / 1000000.0;
    cout << "Time: " << elapsed << " seconds" << endl;
    // bool all_good = checkCorrectness(arr, N);
    // if (all_good) {
    //     cout << "Correct!" << endl;
    // } else {
    //     cout << "Incorrect!" << endl;
        // printArray(arr, N);
    // }

}