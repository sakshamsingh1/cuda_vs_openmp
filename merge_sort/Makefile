omp:
	/scratch/sk8974/offload/install/bin/gcc -fopenmp merge_openmp.cpp util.cpp -o mergesort_omp -lstdc++

seq:
	gcc merge_cpu.cpp util.cpp -o mergesort_seq -lstdc++

cuda:
	nvcc merge_cuda.cu util.cpp -o mergesort_cuda

clean:
	rm -rf mergesort_cuda mergesort_omp mergesort_seq
