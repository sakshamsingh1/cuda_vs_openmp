#include <cstdio>
#include <random>
#include <iostream>	///
#include <cstdlib>
#include "utils.h"

using namespace std;	///
#define square(X) X*X
#define THREADS_PER_BLOCK 512

/*****************************************************************/

/*** Kernel Definitions ***/
__global__ void find_cluster(int, int, Pixel*, const Cluster* __restrict__);
__global__ void recenter1(int, int, Pixel*, uint4*);
__global__ void recenter2(Cluster*, uint4*, bool*);
/**** end of the kernel declaration ***/
void rgb_color_code(int rgb[], int, int);

/*****************************************************************/

int main(int argc, char * argv[]) {
	
	srand((unsigned) time(NULL));
	if(argc != 4){
        fprintf(stderr, "usage: kmeans_sequential <IN_PATH> <OUT_PATH> <K_CLUSTERS> \n");
        exit(1);
    }

	const char *inPath, *outPath;
	inPath = argv[1]; outPath = argv[2];
	int K_clusters = atoi(argv[3]);
	
	unsigned char* image;

	int idx = 0;
	int img_height = 600, img_width = 800;
	int total_pixels = img_height * img_width;
	int total_blobs = 10;
	// int total_blobs = 2 * K_clusters;
	int blob_radius = 150;
	int blob_centres[100][2]; // change
	int pixels_per_blob = 5000;
	int n_pixels = total_blobs * pixels_per_blob;
	Pixel* pixels = (Pixel*)calloc(n_pixels, sizeof(Pixel));
	unsigned char* in_image = (unsigned char*)calloc(total_pixels*3, sizeof(unsigned char*));

	while(idx < total_pixels){
		in_image[3*idx] = 255;
		in_image[3*idx+1] = 255;
		in_image[3*idx+2] = 255;
		idx++;
	}
	int rgb[K_clusters][3];
	for (int i=0; i<K_clusters; i++) {
		rgb_color_code(rgb[i], K_clusters, i);
	}
	
	for (int i=0; i<total_blobs; i++) {
		blob_centres[i][0] = rand()%(img_width - blob_radius); 
		blob_centres[i][1] = rand()%(img_height - blob_radius); 
	}
	int p_idx = 0;
	for (int i=0; i<total_blobs; i++) {
		for (int j=0; j<pixels_per_blob; j++) {
			int x = blob_centres[i][0] + rand()%blob_radius;
			int y = blob_centres[i][1] + rand()%blob_radius;
			int pos = img_width*y + x;
			pixels[p_idx].x = x;
			pixels[p_idx].y = y;
			pixels[p_idx].cluster = -1;
	 		in_image[3*pos] = 0;
			in_image[3*pos+1] = 0;
			in_image[3*pos+2] = 0;
			p_idx++;	
		}
	}

	if ((write_png(inPath, in_image, img_height, img_width, 3)) != 0) {
		cout<<"fail to write input png file"<<endl;
		// printf("fail to write input png file\n");
		exit(1);
	}

	int i=0;

	Cluster* clusters = (Cluster*)calloc(K_clusters, sizeof(Cluster));
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> uniform(0, n_pixels - 1);
	i=0;
	
	//Initialize Cluster and Assign a Random Pixel to the Cluster
	while (i<K_clusters) {
		Pixel *pixel = &pixels[uniform(gen)];
		clusters[i++] = Cluster(pixel->x, pixel->y, 0, 0, (int*)calloc(n_pixels, sizeof(int)));
		cout<<"cluster centres: "<<pixel->x<<" .. "<<pixel->y<<endl; ///
	}
 	cout<<endl;	///

	//Define Blocks and Threads per block
    dim3 numBlocks((n_pixels + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK);
    dim3 threadsPerBlock(THREADS_PER_BLOCK);

	// allocate device memory
	Pixel* d_pixels;
	Cluster* d_cluster;
	size_t sz_pixel= sizeof(Pixel), sz_cluster = sizeof(Cluster);
	uint4* d_sum;	//To store sum and count
	bool* d_converged;

	cudaMalloc((void**)&d_pixels, n_pixels * sz_pixel);
	cudaMalloc((void**)&d_cluster, K_clusters * sz_cluster);
	cudaMalloc((void**)&d_sum, sizeof(uint4));
	cudaMalloc((void**)&d_converged, sizeof(bool));

	if(!d_pixels && !d_cluster && !d_sum && !d_converged){
        printf("cannot allocate array\n");
        exit(1);
    }

	cudaMemcpy(d_pixels, pixels, n_pixels * sz_pixel, cudaMemcpyHostToDevice);
	cudaMemcpy(d_cluster, clusters, K_clusters * sz_cluster, cudaMemcpyHostToDevice);


	bool thread_converged = true;
	bool converged;
	int itr = 0;
	do{
		itr++;
		find_cluster<<<numBlocks , threadsPerBlock>>>(n_pixels, K_clusters, d_pixels, d_cluster);
		thread_converged = true;
		for (int i = 0; i < K_clusters; ++i) {
			cudaMemset(d_sum, 0, 4 * sizeof(int));

			recenter1<<<numBlocks , threadsPerBlock>>>(n_pixels, i, d_pixels, d_sum);
			recenter2<<<1, 1>>>(&d_cluster[i], d_sum, d_converged);

			cudaMemcpy(&converged, d_converged, sizeof(bool), cudaMemcpyDeviceToHost);

			thread_converged &= converged;
		}
	} while (!thread_converged || itr > 50000);

	// copy device memory back to host
	cudaMemcpy(pixels, d_pixels, n_pixels * sz_pixel, cudaMemcpyDeviceToHost);
	cudaMemcpy(clusters, d_cluster, K_clusters * sz_cluster, cudaMemcpyDeviceToHost);

	for (int i=0; i<K_clusters; i++) {
		// cout<<"rgb: "<<i<<rgb[i][0]<<" .. "<<rgb[i][1]<<" .. "<<rgb[i][2]<<endl; ///
		cout<<"final cluster "<<i+1<<": "<<(int)clusters[i].x<<" "<<(int)clusters[i].y<<" "<<(int)clusters[i].z<<endl<<endl;	///
	}

	// free device memory
	cudaFree(d_pixels);
	cudaFree(d_cluster);
	cudaFree(d_sum);

	idx = 0;
	unsigned char* out_image = (unsigned char*)calloc(total_pixels*3, sizeof(unsigned char*));
	while(idx < total_pixels){
		out_image[3*idx] = 255;
		out_image[3*idx+1] = 255;
		out_image[3*idx+2] = 255;
		idx++;
	}
	idx = 0;
	while(idx < n_pixels){
		int cluster_idx = pixels[idx].cluster;
		int pos = img_width*pixels[idx].y + pixels[idx].x;
		out_image[3*pos] = rgb[cluster_idx][0];
		out_image[3*pos+1] = rgb[cluster_idx][1];
		out_image[3*pos+2] = rgb[cluster_idx][2];
		idx++;
	}

	if ((write_png(outPath, out_image, img_height, img_width, 3)) != 0) {
		printf("fail to write output png file\n");
		exit(1);
	}

	delete[] clusters;
	delete[] pixels;

	return 0;
}

void rgb_color_code(int rgb[], int K_clusters, int cluster_idx) {
	cluster_idx++;
	int idx = 2;
	if (K_clusters < 7) {
	  	for(int power=4; power>0; power/=2) {
	    	rgb[idx--] = (cluster_idx/power) * 255;
	    	cluster_idx = cluster_idx%power;
		}
	} else {
	  	for(int i=0;i<3;i++) {
	    	rgb[i] = (rand()%K_clusters) * (255/K_clusters);
	  	}
	}
}

__global__ void find_cluster(int total_pixels, int K_clusters, Pixel* pixels, const Cluster* __restrict__ clusters) {
	int idx = threadIdx.x + blockDim.x * blockIdx.x;
	if (idx < total_pixels){
		Pixel pixel = pixels[idx];
		int min = INT_MAX, min_cluster, dist, j=0;

		while(j<K_clusters){
			dist = square((pixel.x - clusters[j].x))+ square((pixel.y - clusters[j].y));
			if (dist < min) {
				min = dist;
				min_cluster = j;
			}
			j++;
		}
		pixels[idx].cluster = min_cluster;
	}
}

__global__ void recenter1(int total_pixels, int cluster, Pixel* pixels, uint4* sumc) {
	int idx = threadIdx.x + blockDim.x * blockIdx.x;
	if (idx < total_pixels){
		Pixel pixel = pixels[idx];
		if (pixel.cluster == cluster) {
			atomicAdd(&sumc->x, pixel.x);
			atomicAdd(&sumc->y, pixel.y);
			atomicAdd(&sumc->w, 1);
		}
	}

}

__global__ void recenter2(Cluster* cluster, uint4* sumc, bool* converged) {
	uint32_t points = sumc->w ;
	*converged = false;
	if (points > 0) {
		Cluster copy = *cluster;
		cluster->x = (sumc->x) / (points);
		cluster->y = (sumc->y) / (points);
		if (cluster->x == copy.x && 
			cluster->y == copy.y)
			*converged=true;
	}
}
