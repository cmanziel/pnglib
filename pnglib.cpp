// pnglib.cpp : Defines the functions for the static library.

#include "pch.h"
#include "framework.h"
#include <stdio.h>
#include <stdint.h>
#include "make_png.h"

#include <string.h>

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

unsigned int search_IDAT_start(FILE* image)
{
	const char* to_find = "IDAT\0";
	char IDAT_sig[5];
	unsigned int byte_num = 0;

	// reset file pointer at the start in case it was previously moved by calls like fread
	fseek(image, 0, SEEK_SET);

	fread(IDAT_sig, sizeof(unsigned char), 4, image);

	IDAT_sig[4] = '\0';

	while (strcmp(to_find, IDAT_sig) != 0)
	{
		byte_num++;

		// go back 3 bytes from the current pointer position to read bytes one after the other
		fseek(image, -3, SEEK_CUR);

		//cicle of freads until "IDAT" is found
		fread(IDAT_sig, sizeof(unsigned char), 4, image);

		IDAT_sig[4] = '\0';
	}

	return byte_num;
}

uLong image_get_IDAT_size(FILE* image)
{
	uLong idat_size;

	//// fseek to start of the IDAT chunk
	//// 33: first byte in the idat chunk
	//fseek(image, 33, SEEK_SET);
	//// 4 bytes for the chunk's size's value
	//fread(&idat_size, sizeof(unsigned char), 4, image);

	unsigned int idat_start = search_IDAT_start(image);

	fseek(image, idat_start, SEEK_SET);

	// the length field start four bytes before the IDAT chunk type byte index
	fseek(image, -4, SEEK_CUR);

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

	unsigned int idat_start = search_IDAT_start(image);

	// idat_start + 4: idat_start is the byte number for the first byte of teh chunk type field
	// chuynk data start 4 bytes after it
	fseek(image, idat_start + 4, SEEK_SET);

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
unsigned char* decompress_image(FILE* image)
{
	uLong pixel_data_size = image_get_size(image);

	uLong idat_size = image_get_IDAT_size(image);

	unsigned char* idat_data = image_get_IDAT_data(image);

	//// get idat data, send it to the decomp function
	//unsigned char* idat_data = (unsigned char*)malloc(idat_size);

	//if (idat_data == NULL)
	//{
	//	printf("decompressed data buffer not allocated\n");
	//	return NULL;
	//}

	//fread(idat_data, sizeof(unsigned char), idat_size, image);

	unsigned char* decomp_data = decomp(idat_data, pixel_data_size, idat_size);

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