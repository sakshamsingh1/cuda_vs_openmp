omp:
	/scratch/am10150/offload/install/bin/gcc -fopenmp omp.cpp util.cpp -o mergesort_omp -lstdc++

seq:
	gcc seq.cpp util.cpp -o mergesort_seq -lstdc++

cuda:
	nvcc cuda.cu util.cpp -o mergesort_cuda

clean:
	rm -rf mergesort_cuda mergesort_omp mergesort_seq
