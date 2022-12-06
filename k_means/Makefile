seq:
	g++ -lpng -std=c++11 kmeans_sequential.cpp -o kmeans_seq

omp:
	/scratch/am10150/offload/install/bin/g++ -fopenmp -lpng -std=c++11 kmeans_omp.cpp -o kmeans_omp

cuda:
	nvcc -lpng -std=c++11 kmeans_cuda.cu -o kmeans_cuda

clean:
	rm -rf kmeans_seq kmeans_omp kmeans_cuda
