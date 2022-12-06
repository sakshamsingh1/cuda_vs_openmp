// generate input file
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "helper.h"

using namespace std;

int main(int argc, char * argv[]) {

	srand((unsigned) time(NULL));
	if(argc != 2){
		cout<<"usage: input_generator <INPUT_FILE_PATH>"<<endl;
        exit(1);
    }

	const char *in_file;
	in_file = argv[1];
	int total_blobs = 10;
	int points_per_blob = 5000;

	int idx = 0, img_height = 600, img_width = 800;
	int total_img_points = img_height * img_width;
	// int total_blobs = 2 * K_clusters;
	int blob_radius = 150;
	int blob_centres[100][2];
	int total_blob_points = total_blobs * points_per_blob;
	unsigned char* in_image = (unsigned char*)calloc(total_img_points*3, sizeof(unsigned char*));

	while(idx < total_img_points){
		in_image[3*idx] = 255;
		in_image[3*idx+1] = 255;
		in_image[3*idx+2] = 255;
		idx++;
	}
	
	for (int i=0; i<total_blobs; i++) {
		blob_centres[i][0] = rand()%(img_width - blob_radius); 
		blob_centres[i][1] = rand()%(img_height - blob_radius); 
	}
	for (int i=0; i<total_blobs; i++) {
		for (int j=0; j<points_per_blob; j++) {
			int x = blob_centres[i][0] + rand()%blob_radius;
			int y = blob_centres[i][1] + rand()%blob_radius;
			int pos = img_width*y + x;
	 		in_image[3*pos] = 0;
			in_image[3*pos+1] = 0;
			in_image[3*pos+2] = 0;
		}
	}

	if (!(write_png(in_file, in_image, img_height, img_width, 3))) {
		cout<<"Failed to write the input .png file"<<endl;
		exit(1);
	}

	return 0;
}