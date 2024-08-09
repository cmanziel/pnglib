#pragma once

#pragma warning (disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "zlib.h"
#include "compress.h"
#include "decompress.h"

#define CHANNELS_PER_PIXEL 3

// TODO: move the fwrites in the main loop and free the chunks in the main loop

typedef struct {
	uint32_t c_length;
	unsigned char c_type[4];
	unsigned char* c_data;
	uint32_t checksum;
} chunk;

typedef struct {
	uLong size;
	uLong location; // byte number for the start of "IDAT" chunk signature
	unsigned char* data;
} IDAT;

void set_type(unsigned char* chunk_type, const char* type_name);
void set_IHDR(FILE* image, chunk* IHDR, uint32_t widht, uint32_t height);
void set_IDAT(FILE* image, chunk* IDAT, unsigned char* pixel_data, uint32_t width, uint32_t height);
void set_IEND(FILE* image, chunk* IEND);

void make_png(FILE* image, unsigned char* data, uint32_t width, uint32_t height, const char* path);