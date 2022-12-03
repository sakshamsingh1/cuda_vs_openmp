#include <stdint.h>
#include <png.h>

// K-Means pixel point color for an RGB image
struct Pixel { // Point2D
	uint32_t x;   
	uint32_t y;   
	int cluster=0; // the cluster id
};

struct Point3d {
	uint32_t x;
	uint32_t y;
	uint32_t z;
	Point3d(uint32_t x, uint32_t y, uint32_t z):x(x), y(y), z(z){}
};

#define square(X) X*X

// K-Means cluster centeral points
struct Cluster {
	uint32_t x;
	uint32_t y;
	uint32_t z;
	size_t size; // the number of points in the cluster
	int* pixels; // the point ids in the cluster
	Cluster(uint32_t x, uint32_t y, uint32_t z, size_t size, int* pixels):x(x), y(y), z(z), size(size), pixels(pixels){}
	Cluster():x(0), y(0), z(0), size(0){}
};


int read_png(const char* filename, unsigned char** image, unsigned int& height, unsigned int& width, unsigned int& channels) {
	unsigned char sig[8];

	FILE* fp = fopen(filename, "rb");
	if (!fp) {
		printf("Unable to read the input PNG File \n");
		return 1;
	}

	fread(sig, 1, 8, fp);
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info_ptr = png_create_info_struct(png_ptr);

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	png_read_info(png_ptr, info_ptr);

 	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	png_uint_32 color_type = png_get_color_type(png_ptr, info_ptr);
	png_uint_32 bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	if (color_type != PNG_COLOR_TYPE_RGB) {
		// our k-means implementation only support png image with color type RGB (3 channels)
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return 2;
	}

	if (bit_depth != 8) {
		// our k-means implementation only support png image with bit depth 8
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return 2;
	}

	channels = png_get_channels(png_ptr, info_ptr);
	if (channels != 3) {
		// our k-means implementation only support png image with channels equals 3
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return 2;
	}

	size_t row_bytes = width * channels;

	*image = (unsigned char*)calloc(height, row_bytes);
	if ((*image) == NULL) {
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return 3;
	}

	png_bytep* image_ptr = (png_bytep*)calloc(height, sizeof(png_bytep));
	for (unsigned int i = 0; i < height; ++i) {
		image_ptr[i] = (*image) + i * row_bytes;
	}

	png_read_image(png_ptr, image_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	free(image_ptr);
	fclose(fp);
	return 0;
}

int write_png(const char* filename, unsigned char* image, unsigned int height, unsigned int width, unsigned int channels) {
	FILE* fp = fopen(filename, "wb");

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr) {
		return 1;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_write_struct(&png_ptr, NULL);
		return 1;
	}

	png_init_io(png_ptr, fp);
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_set_filter(png_ptr, 0, PNG_NO_FILTERS);
	png_write_info(png_ptr, info_ptr);
	png_set_compression_level(png_ptr, 1);

	png_bytep* image_ptr = (png_bytep*)calloc(height, sizeof(png_bytep));
	size_t row_bytes = width * channels;

	for (int i = 0; i < height; ++i) {
		image_ptr[i] = image + i * row_bytes;
	}

	png_write_image(png_ptr, image_ptr);
	png_write_end(png_ptr, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);
	return 0;
}