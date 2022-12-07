# OpenMP vs CUDA

This repository contains benchmarking programs for OpenMP and CUDA. The 4 programs are :

1. Matrix Multiplication
2. Sieve of Eratosthenes
3. K-Means Clustering
4. Merge Sort

The programs were benchmarked on NYU's Greene cluster. The cluster specs included 32GB RAM, Intel Xeon Platinum 8268 CPU @ 2.90GHz, and Nvidia Quadro RTX8000 GPU.

Also, gcc was compiled with CUDA using this [link](https://kristerw.blogspot.com/2017/04/building-gcc-with-support-for-nvidia.html).

To compile the program, navigate to the respective program directory and then run the following command :

`make cuda omp seq `

To run the program, navigate to the respective program directory and then follow the instructions below for each program: 

### Matrix multiplication
`./seq_mul <N>` : Sequential matrix multiplication

`./omp_mul <N>` : OpenMP matrix multiplication

`./cuda_mul <N>` : CUDA matrix multiplication

### Sieve of Eratosthenes
`./seq_sieve <N>` : Sequential Sieve of Eratosthenes

`./omp_sieve <N>` : OpenMP Sieve of Eratosthenes

`./cuda_sieve <N>` : CUDA Sieve of Eratosthenes

### K-Means Clustering
`./kmeans_seq <K_CLUSTERS> input.png seq.png` : Sequential K-Means Clustering

`./kmeans_omp <K_CLUSTERS> input.png omp.png` : OpenMP K-Means Clustering

`./kmeans_cuda <K_CLUSTERS> input.png cuda.png` : CUDA K-Means Clustering

### Merge Sort
`./seq_merge <N>` : Sequential Merge Sort

`./omp_merge <N>` : OpenMP Merge Sort

`./cuda_merge <N>` : CUDA Merge Sort