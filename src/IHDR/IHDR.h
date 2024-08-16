#ifndef IHDR_H
#define IHDR_H

#include "../zlib_funs.h"

#define IHDR_SIGNATURE_LOCATION 12

enum color_types {
	GREYSCALE, TRUECOLOR = 2, INDEXED_COLOR, GREYSCALE_ALPHA, TRUECOLOR_ALPHA = 6
};

enum filter_types {
	NONE, SUB, UP, AVERAGE, PAETH
};

typedef struct {
	uint32_t image_width;
	uint32_t image_height;
	uint32_t image_byte_size;
	uint8_t bit_depth;
	uint8_t color_type;
	uint8_t compression_method;
	uint8_t filter_method;
	uint8_t interlace_method;
} IHDR;

IHDR read_IHDR(FILE* image);
uint32_t image_get_width(FILE* image);
uint32_t image_get_height(FILE* image);
uint8_t image_get_bit_depth(FILE* image);
uint8_t image_get_color_type(FILE* image);
uint8_t image_get_channels_per_pixel(FILE* image);

#endif