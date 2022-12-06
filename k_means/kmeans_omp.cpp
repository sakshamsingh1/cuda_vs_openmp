#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <random>
#include <omp.h>
#include <sys/time.h>
#include <iostream>
#include "helper.h"

using namespace std;

void get_cluster_colors(int cluster_colors[], int, int);
bool clusters_converged(int, int, Point2d*, Cluster*, bool*);

int main(int argc, const char** argv) {
		
	srand((unsigned) time(NULL));
	if(argc != 4){
		cout<<"usage: kmeans_omp <K_CLUSTERS> <INPUT_FILE_PATH> <OUTPUT_FILE_PATH> [<total_blobs> <points_per_blob>]"<<endl;
        exit(1);
    }

	const char *in_file, *out_file;
	int K_clusters = atoi(argv[1]);
	in_file = argv[2]; out_file = argv[3];
	int total_blobs = 10;
	int points_per_blob = 5000;
	
	// points in input PNG file
	if (argc > 4) {
		int total_blobs = atoi(argv[4]);
		int points_per_blob = atoi(argv[5]);
	}
	
	int idx = 0;
	int img_height = 600, img_width = 800;
	int total_img_points = img_height * img_width;
	// int total_blobs = 2 * K_clusters;
	int blob_radius = 150;
	int blob_centres[100][2]; 
	int total_blob_points = total_blobs * points_per_blob;
	Point2d* points = (Point2d*)calloc(total_blob_points, sizeof(Point2d));
	unsigned char* in_image = (unsigned char*)calloc(total_img_points*3, sizeof(unsigned char*));
	unsigned char* out_image = (unsigned char*)calloc(total_img_points*3, sizeof(unsigned char*));

	while(idx < total_img_points){
		in_image[3*idx] = 255;
		in_image[3*idx+1] = 255;
		in_image[3*idx+2] = 255;
		out_image[3*idx] = 255;
		out_image[3*idx+1] = 255;
		out_image[3*idx+2] = 255;
		idx++;
	}

	int cluster_colors[K_clusters][3];
	for (int i=0; i<K_clusters; i++) {
		get_cluster_colors(cluster_colors[i], K_clusters, i);
	}
	
	for (int i=0; i<total_blobs; i++) {
		blob_centres[i][0] = rand()%(img_width - blob_radius); 
		blob_centres[i][1] = rand()%(img_height - blob_radius); 
	}
	int p_idx = 0;
	for (int i=0; i<total_blobs; i++) {
		for (int j=0; j<points_per_blob; j++) {
			int x = blob_centres[i][0] + rand()%blob_radius;
			int y = blob_centres[i][1] + rand()%blob_radius;
			int pos = img_width*y + x;
			points[p_idx].x = x;
			points[p_idx].y = y;
			points[p_idx].cluster = -1;
	 		in_image[3*pos] = 0;
			in_image[3*pos+1] = 0;
			in_image[3*pos+2] = 0;
			p_idx++;	
		}
	}

	if (!(write_png(in_file, in_image, img_height, img_width, 3))) {
		cout<<"Failed to write the input .png file"<<endl;
		exit(1);
	}

	Cluster* clusters = (Cluster*)calloc(K_clusters, sizeof(Cluster));
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> uniform(0, total_blob_points - 1);
	
	struct timeval tv1, tv2;	
    struct timezone tz;	
    double elapsed;
	int i=0;
    gettimeofday(&tv1, &tz);	

	//Initialize clusters by assigning a random 2D Point to the cluster
	while (i < K_clusters) {
		Point2d *point = &points[uniform(gen)];
		clusters[i++] = Cluster(point->x, point->y, 0, (int*)calloc(total_blob_points, sizeof(int)));
	}

	bool visited[K_clusters*total_blob_points];
	bool converged;
	do {
		for (int i = 0; i < K_clusters; i++) {
			memset(visited, false, K_clusters*total_blob_points*sizeof(bool));
		}

		//Find the closest cluster
		#pragma omp shared(total_blob_points, K_clusters) 
		{
            #pragma omp target teams distribute parallel for map(to:total_blob_points, K_clusters)\
														map(tofrom:points[0:total_blob_points], clusters[0:K_clusters], visited[0:K_clusters*total_blob_points])\
														shared(total_blob_points, K_clusters)\
														schedule(auto) 
			for (int idx = 0; idx < total_blob_points; idx++) {
				int closest_cluster, dist, min_dist = INT_MAX;
				for (int i = 0; i < K_clusters; i++) {
					dist = 	square((points[idx].x - clusters[i].x)) + square((points[idx].y - clusters[i].y));  
					if (dist <= min_dist) {
						closest_cluster = i;
						min_dist = dist;
					}
				}
				points[idx].cluster = closest_cluster;
				visited[closest_cluster*total_blob_points + idx] = true;
			}
		}
		converged = clusters_converged(total_blob_points, K_clusters, points, clusters, visited); 
	} while (!converged); // Loop Until Convergence of Centroids

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

bool clusters_converged(int n, int K_clusters, Point2d* points, Cluster* clusters, bool visited[]) {
	for (int i = 0; i < K_clusters; ++i) {
		int total_cluster_points = 0, sum_x = 0, sum_y = 0;
		#pragma omp teams distribute \
			parallel for reduction(+:sum_x, sum_y, total_cluster_points)
		for (int j = 0; j < n; j++) {
			Point2d* point = &points[j];
			if (visited[i*n + j]) {
				sum_x += point->x;
				sum_y += point->y;
				total_cluster_points++;
			}
		}

		if (total_cluster_points > 0) {
			Cluster copy = clusters[i];
			clusters[i].x = sum_x/total_cluster_points;
			clusters[i].y = sum_y/total_cluster_points;
			if (clusters[i].x != copy.x || clusters[i].y != copy.y) {
				return false;
			}
		}
	}
	return true;
}