
kmeans_seq.out:kmeans_sequential.cpp
	g++ -lpng -std=c++11 kmeans_sequential.cpp -o kmeans_sequential

kmeans_omp.out:kmeans_omp.cpp
	g++ -fopenmp -lpng -std=c++11 kmeans_omp.cpp -o kmeans_omp

kmeans_cuda.out:kmeans_cuda.cu
	nvcc -lpng -std=c++11 kmeans_cuda.cu -o kmeans_cuda

clean:
	rm -rf *.out
