GCC=/scratch/am10150/offload/install/bin/gcc
omp:
	 $(GCC) -fopenmp omp.cpp utils.cpp -o omp_sieve -lstdc++ -lm
seq:
	gcc seq.cpp utils.cpp -o seq_sieve -lstdc++ -lm
cuda:
	nvcc cuda.cu utils.cpp -o cuda_sieve -lstdc++ -lm
clean:
	rm -rf seq_sieve omp_sieve cuda_sieve
