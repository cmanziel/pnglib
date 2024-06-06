#pragma once

#include "pch.h"
#include "make_png.h"

FILE* create_image(unsigned char* data, const char* path, int width, int height);
unsigned char* decompress_image(FILE* image);

uInt image_get_width(FILE* image);
uInt image_get_height(FILE* image);

uLong image_get_size(FILE* image);

unsigned int search_IDAT_start(FILE* image);

uLong pixel_coords_to_index(FILE* image, unsigned int x, unsigned int y);

// return the size
uLong image_get_IDAT_size(FILE* image);
unsigned char* image_get_IDAT_data(FILE* image);

//uLong image_get_pixel_data_size()
//{
//	return img
//}