#include <cstdio>
#include <random>
#include <iostream>
#include <cstdlib>
#include <sys/time.h>
#include "helper.h"

using namespace std;

#define THREADS_PER_BLOCK 512

/*****************************************************************/

void get_cluster_colors(int cluster_colors[], int, int);
/*** Kernel Definitions ***/
__global__ void clustering(int, int, Point2d*, const Cluster* __restrict__);
__global__ void recenter_sum(int, int, Point2d*, uint4*);
__global__ void convergence_check(Cluster*, uint4*, bool*);
/**** end of the kernel declaration ***/

/*****************************************************************/

int main(int argc, char * argv[]) {
	
	srand((unsigned) time(NULL));
	if(argc != 4 && argc != 6){
		cout<<"usage: kmeans_cuda <K_CLUSTERS> <INPUT_FILE_PATH> <OUTPUT_FILE_PATH>"<<endl;
        exit(1);
    }

	const char *in_file, *out_file;
	int K_clusters = atoi(argv[1]);
	in_file = argv[2]; out_file = argv[3];
	int idx = 0;
	unsigned int img_height, img_width, channels = 3;
	unsigned char* in_image;

	if (!read_png(in_file, &in_image, img_height, img_width, channels)) {
		exit(1);
	}
	int total_img_points = img_height * img_width;
	unsigned char* out_image = (unsigned char*)calloc(total_img_points*3, sizeof(unsigned char*));
	int total_blob_points = get_total_blob_points(in_image, img_height, img_width);
	Point2d* points = (Point2d*)calloc(total_blob_points, sizeof(Point2d));
	int i=0, pt_idx = 0;
	while(i < total_img_points){
		if (in_image[3*i] == 0 && in_image[3*i+1] == 0 && in_image[3*i+2] == 0) {
			points[pt_idx].x = i%img_width;
			points[pt_idx].y = i/img_width;
			points[pt_idx].cluster = -1;
			pt_idx++;
		}
		i++;
	}

	while(idx < total_img_points) {
		out_image[3*idx] = 255;
		out_image[3*idx+1] = 255;
		out_image[3*idx+2] = 255;
		idx++;
	}

	int cluster_colors[K_clusters][3];
	for (int i=0; i<K_clusters; i++) {
		get_cluster_colors(cluster_colors[i], K_clusters, i);
	}

	Cluster* clusters = (Cluster*)calloc(K_clusters, sizeof(Cluster));
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> uniform(0, total_blob_points - 1);
	
	struct timeval tv1, tv2;	
    struct timezone tz;	
    double elapsed;
	i=0;
    gettimeofday(&tv1, &tz);	

	//Initialize clusters by assigning a random 2D Point to the cluster
	while (i<K_clusters) {
		Point2d *point = &points[uniform(gen)];
		clusters[i++] = Cluster(point->x, point->y, 0, (int*)calloc(total_blob_points, sizeof(int)));
	}

    dim3 total_blocks((total_blob_points + THREADS_PER_BLOCK - 1) / THREADS_PER_BLOCK);
    dim3 threads_per_block(THREADS_PER_BLOCK);

	//Allocate device memory
	Point2d* d_points;
	Cluster* d_cluster;
	size_t point_size = sizeof(Point2d), cluster_size = sizeof(Cluster);
	bool* d_converged;
	uint4* d_sum;

	cudaMalloc((void**)&d_points, total_blob_points * point_size);
	cudaMalloc((void**)&d_cluster, K_clusters * cluster_size);
	cudaMalloc((void**)&d_converged, sizeof(bool));
	cudaMalloc((void**)&d_sum, sizeof(uint4));

	if(!d_points && !d_cluster && !d_sum && !d_converged){
        cout<<"array cannot be allocated"<<endl;
        exit(1);
    }

	cudaMemcpy(d_cluster, clusters, K_clusters * cluster_size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_points, points, total_blob_points * point_size, cudaMemcpyHostToDevice);

	bool all_converged = true, converged;
	do {
		clustering<<<total_blocks , threads_per_block>>>(total_blob_points, K_clusters, d_points, d_cluster);
		all_converged = true;
		for (int i = 0; i < K_clusters; i++) {
			cudaMemset(d_sum, 0, 4 * sizeof(int));
			recenter_sum<<<total_blocks , threads_per_block>>>(total_blob_points, i, d_points, d_sum);
			convergence_check<<<1, 1>>>(&d_cluster[i], d_sum, d_converged);
			cudaMemcpy(&converged, d_converged, sizeof(bool), cudaMemcpyDeviceToHost);
			all_converged &= converged;
		}
	} while (!all_converged); // Loop Until Convergence of Centroids

	// copy device memory to host
	cudaMemcpy(points, d_points, total_blob_points * point_size, cudaMemcpyDeviceToHost);
	cudaMemcpy(clusters, d_cluster, K_clusters * cluster_size, cudaMemcpyDeviceToHost);

	// Print the final cluster centre points and the color associated to each cluster
	/*
	for (int i=0; i<K_clusters; i++) {
		cout<<"cluster_rgb_colors: "<<i+1<<cluster_colors[i][0]<<" .. "<<cluster_colors[i][1]<<" .. "<<cluster_colors[i][2]<<endl; ///
		cout<<"final cluster centre-point"<<i+1<<": "<<(int)clusters[i].x<<" "<<(int)clusters[i].y<<endl;	///
	}
	*/

	// free device memory
	cudaFree(d_cluster);
	cudaFree(d_points);
	cudaFree(d_sum);

 	gettimeofday(&tv2, &tz);	
    elapsed = (double) (tv2.tv_sec-tv1.tv_sec) + (double) (tv2.tv_usec-tv1.tv_usec) * 1.e-6;	
    printf("elapsed time = %f seconds.\n", elapsed);

	idx = 0;
	while(idx < total_blob_points){
		int cluster_idx = points[idx].cluster;
		int pos = img_width*points[idx].y + points[idx].x;
		out_image[3*pos] = cluster_colors[cluster_idx][0];
		out_image[3*pos+1] = cluster_colors[cluster_idx][1];
		out_image[3*pos+2] = cluster_colors[cluster_idx][2];
		idx++;
	}

	if (!(write_png(out_file, out_image, img_height, img_width, 3))) {
		cout<<"Failed to write the output .png file"<<endl;
		exit(1);
	}

	delete[] clusters;
	delete[] points;

	return 0;
}

void get_cluster_colors(int cluster_colors[], int K_clusters, int cluster_idx) {
	cluster_idx++;
	int idx = 2;
	if (K_clusters < 7) {
	  	for(int power=4; power>0; power/=2) {
	    	cluster_colors[idx--] = (cluster_idx/power) * 255;
	    	cluster_idx = cluster_idx%power;
		}
	} else {
	  	for(int i=0;i<3;i++) {
	    	cluster_colors[i] = (rand()%K_clusters) * (255/K_clusters);
	  	}
	}
}

__global__ void clustering(int total_img_points, int K_clusters, Point2d* points, const Cluster* __restrict__ clusters) {
	int idx = threadIdx.x + blockDim.x * blockIdx.x;
	if (idx < total_img_points){
		Point2d point = points[idx];
		int closest_cluster, dist, min = INT_MAX, i=0;

		while (i < K_clusters){
			dist = square((point.x - clusters[i].x)) + square((point.y - clusters[i].y));
			if (dist < min) {
				min = dist;
				closest_cluster = i;
			}
			i++;
		}
		points[idx].cluster = closest_cluster;
	}
}

__global__ void recenter_sum(int total_img_points, int cluster, Point2d* points, uint4* sumc) {
	int idx = threadIdx.x + blockDim.x * blockIdx.x;
	if (idx < total_img_points){
		Point2d point = points[idx];
		if (point.cluster == cluster) {
			atomicAdd(&sumc->x, point.x);
			atomicAdd(&sumc->y, point.y);
			atomicAdd(&sumc->w, 1);
		}
	}

}

__global__ void convergence_check(Cluster* cluster, uint4* sumc, bool* converged) {
	uint32_t total_cluster_points = sumc->w ;
	*converged = false;
	if (total_cluster_points > 0) {
		Cluster copy = *cluster;
		cluster->x = (sumc->x)/total_cluster_points;
		cluster->y = (sumc->y)/total_cluster_points;
		if (cluster->x == copy.x && cluster->y == copy.y)
			*converged=true;
	}
}
