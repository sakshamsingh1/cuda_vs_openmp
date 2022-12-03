#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <random>
#include <vector>
#include <omp.h>
#include "utils.h"

void rgb_color_code(int rgb[], int, int);

int recenter(int n, int K_clusters, Pixel* pixels, Cluster* clusters, bool* visited) {
	for (int i = 0; i < K_clusters; ++i) {
		Cluster *tmp = &clusters[i];
		int count = 0;
		int sum_x, sum_y;
		sum_x = sum_y = 0;
		#pragma omp teams distribute \
			parallel for reduction(+:sum_x, sum_y, count)
		for (int j = 0; j < n; ++j) {
			Pixel* pixel = &pixels[j];
			bool flag = visited[i*n + j];
			sum_x += pixel->x*flag;
			sum_y += pixel->y*flag;
			count+=flag;
		}

		if (count > 0) {
			Cluster copy = clusters[i];
			clusters[i].x = sum_x / count;
			clusters[i].y = sum_y / count;

			if (copy.x != clusters[i].x || copy.y != clusters[i].y) {
				return 0;
			}
		}
	}

	return 1;
}

int main(int argc, const char** argv) {
		
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
		printf("fail to write input png file\n");
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
	}

	//Used as a 2d array
	bool visited[K_clusters*n_pixels];

	do {
		// #pragma omp target teams distribute parallel for
		for (int i = 0; i < K_clusters; i++) {
			memset(visited, 0, K_clusters*n_pixels*sizeof(bool));
		}
		//Find the Minimum Distance Cluster
		#pragma omp shared(n_pixels, K_clusters) 
		{
            #pragma omp target teams distribute parallel for map(to:n_pixels, K_clusters)\
														shared(n_pixels, K_clusters)\
														map(tofrom:pixels[0:n_pixels], clusters[0:K_clusters], visited[0:K_clusters*n_pixels])\
														schedule(auto) 
			for (int idx = 0; idx < n_pixels; ++idx) {
				int min_dist = INT_MAX, min_cluster;
				for (int j = 0; j < K_clusters; ++j) {
					int dist = 	square((pixels[idx].x - clusters[j].x)) + 
							square((pixels[idx].y - clusters[j].y));  
					
					if (dist <= min_dist) {
						min_dist = dist;
						min_cluster = j;
					}
				}
				pixels[idx].cluster = min_cluster;
				visited[min_cluster*n_pixels + idx] = 1;
			}
		}
	} while (recenter(n_pixels, K_clusters, pixels, clusters, visited) == 0);

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
