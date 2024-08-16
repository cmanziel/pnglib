#pragma once

#include "pch.h"
#include "make_png.h"
#include "IHDR/IHDR.h"

FILE* create_image(unsigned char* data, const char* path, int width, int height);
unsigned char* decompress_image(FILE* image);

// concatenate the uncompressed image data that still contains the scanline byte filter at the start of every scanline
unsigned char* concatenate_filtered_data(unsigned char* filtered_data, unsigned int width, unsigned int height, uint8_t num_of_channels, uint8_t bit_depth);

void reconstruct_filtered_data(unsigned char* filtered_data, size_t width, size_t height, uint8_t num_of_channels, uint8_t bit_depth);
void reconstruct_sub(unsigned char* filtered, unsigned int scanline_size, uint8_t num_of_channels, uint8_t bit_depth);
void reconstruct_up(unsigned char* filtered, unsigned int scanline_size, unsigned int scanline_index, uint8_t num_of_channels, uint8_t bit_depth);
void reconstruct_average(unsigned char* filtered, unsigned int scanline_size, unsigned int scanline_index, uint8_t num_of_channels, uint8_t bit_depth);

unsigned int search_IDAT_start(FILE* image);

IDAT* search_IDATs(FILE* image);

uLong pixel_coords_to_index(FILE* image, unsigned int x, unsigned int y);

// return the size
uLong image_get_IDAT_size(FILE* image);
unsigned char* image_get_IDAT_data(FILE* image);

//uLong image_get_pixel_data_size()
//{
//	return img
//}