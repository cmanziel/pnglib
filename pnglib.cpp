// pnglib.cpp : Defines the functions for the static library.

#include "pch.h"
#include "framework.h"
#include <stdio.h>
#include <stdint.h>
#include "make_png.h"

#pragma warning (disable : 4996)

uInt image_get_width(FILE* image)
{
	uInt width;

	fseek(image, 16, SEEK_SET);
	fread(&width, sizeof(unsigned int), 1, image);

	convert_endianness(&width);

	return width;
}

uInt image_get_height(FILE* image)
{
	uInt height;

	fseek(image, 20, SEEK_SET);
	fread(&height, sizeof(unsigned int), 1, image);

	convert_endianness(&height);

	return height;
}

// size of all the filtered scanlines data
uLong image_get_size(FILE* image)
{
	int width, height;

	width = image_get_width(image);
	height = image_get_height(image);

	return (width * CHANNELS_PER_PIXEL + 1) * height * sizeof(unsigned char);
}

// return index in the image data based on pixel coordinates: image data is made of scanlines of pixel data with a filter method byte at the beginning
uLong pixel_coords_to_index(FILE* image, unsigned int x, unsigned int y)
{
	return (y + 1) + y * (image_get_width(image) * CHANNELS_PER_PIXEL) + (x * CHANNELS_PER_PIXEL);
}

uLong image_get_IDAT_size(FILE* image)
{
	uLong idat_size;

	// fseek to start of the IDAT chunk
	// 33: first byte in the idat chunk
	fseek(image, 33, SEEK_SET);
	// 4 bytes for the chunk's size's value
	fread(&idat_size, sizeof(unsigned char), 4, image);

	convert_endianness(&idat_size);

	return idat_size;
}

unsigned char* image_get_IDAT_data(FILE* image)
{
	uLong idat_size = image_get_IDAT_size(image);
	unsigned char* idat_data = (unsigned char*)malloc(idat_size);

	if (idat_data == NULL)
	{
		printf("buffer for IDAT data not allocated\n");
		return NULL;
	}

	// fseek to start of the IDAT chunk data field
	// 33: first byte in the idat chunk
	// 4 bytes for size and 4 bytes for type: 41
	fseek(image, 41, SEEK_SET);

	fread(idat_data, sizeof(unsigned char), idat_size, image);

	return idat_data;
}


FILE* create_image(unsigned char* data, const char* path, int width, int height)
{
	FILE* image = fopen(path, "wb");

	// in here, compress data with zlib and 
	make_png(image, data, width, height, path);

	fclose(image);

	return image;
}

// decomrpess the content of the image's IDAT (or IDATs) chunk
unsigned char* decompress_image(const char* path)
{
	FILE* image = fopen(path, "rb");

	uLong pixel_data_size = image_get_size(image);

	uLong idat_size;

	//// 33: first byte in the idat chunk
	//fseek(image, 33, SEEK_SET);
	//// 4 bytes for the chunk's size's value
	//fread(&idat_size, sizeof(unsigned char), 4, image);

	idat_size = image_get_IDAT_size(image);

	// get idat data, send it to the decomp function
	unsigned char* idat_data = (unsigned char*)malloc(idat_size);

	if (idat_data == NULL)
	{
		printf("decompressed data buffer not allocated\n");
		return NULL;
	}

	fread(idat_data, sizeof(unsigned char), idat_size, image);

	unsigned char* decomp_data = decomp(idat_data, pixel_data_size, idat_size);

	fclose(image);
	free(idat_data);

	return decomp_data;
}


//uLong image_get_IDAT_size()
//{
//	return img.IDATs[0]->c_length;
//}

//uLong image_get_pixel_data_size()
//{
//	return img.
//}