GCC=/scratch/am10150/offload/install/bin/gcc
omp:
	 $(GCC) -fopenmp omp.cpp utils.cpp -o omp_mul -lstdc++

seq:
	gcc seq.cpp utils.cpp -o seq_mul -lstdc++

cuda:
	nvcc cuda.cu utils.cpp  -o cuda_mul

cuda_tiled:
	nvcc cuda_tiled.cu utils.cpp  -o cuda_tiled_mul

clean:
	rm -rf omp_mul seq_mul cuda_mul cuda_tiled_mul
