#include <png.h>
#include <stdint.h>

#define square(x) x*x

// K-Means 2D point in an image
struct Point2d { // Point2D
	uint32_t x;   
	uint32_t y;   
	int cluster=0; // the cluster id
	Point2d(uint32_t x, uint32_t y, int c): x(x), y(y), cluster(c){}
};

// K-Means cluster central points
struct Cluster {
	uint32_t x;
	uint32_t y;
	size_t size; // the number of points in the cluster
	int* points; // the point ids in the cluster
	Cluster(uint32_t x, uint32_t y, size_t size, int* points):x(x), y(y), size(size), points(points){}
	Cluster():x(0), y(0), size(0){}
};

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

	for (int i = 0; i < height; i++) {
		image_ptr[i] = image + i * row_bytes;
	}

	png_write_image(png_ptr, image_ptr);
	png_write_end(png_ptr, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);
	return 0;
}