#include "util.h"
#include <sys/time.h>
#include <iostream>

using namespace std;

void mergeSort(int*, int, int);
void merge(int*, int, int, int);

int main(int argc, char **argv) {
    int N;
    parseArgs(argc, argv, N);

    struct timeval tv1, tv2;
    struct timezone tz;
    double elapsed;

    // initialize array
    size_t allocSize = N * sizeof(int);
    int *arr = (int*) malloc(allocSize);

    initializeArray(arr, N);

    // sort
    gettimeofday(&tv1, &tz);
    mergeSort(arr, 0, N-1);
    gettimeofday(&tv2, &tz);
    //printArray(arr, arrSize);

    elapsed = (double) (tv2.tv_sec-tv1.tv_sec) + (double) (tv2.tv_usec-tv1.tv_usec) * 1.e-6;
    printf("elapsed time = %f seconds.\n", elapsed);
    return 0;
}

void merge(int *arr, int left, int mid, int right)
{
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int *L = (int *)malloc(sizeof(int) * n1);
    int *R = (int *)malloc(sizeof(int) * n2);

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


void mergeSort(int *arr, int left, int right)
{
    if (left < right)
    {
        int mid = (left + right) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}
