// pnglib.cpp : Defines the functions for the static library.

#include "pch.h"
#include "framework.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "make_png.h"
#include "pnglib.h"

#include <string.h>

#pragma warning (disable : 4996)

unsigned int IDAT_num = 0;

// return index in the image data based on pixel coordinates: image data is made of scanlines of pixel data with a filter method byte at the beginning
uLong pixel_coords_to_index(FILE* image, unsigned int x, unsigned int y)
{
	return (y + 1) + y * (image_get_width(image) * CHANNELS_PER_PIXEL) + (x * CHANNELS_PER_PIXEL);
}

unsigned int search_IDAT_start(FILE* image)
{
	// search for IDATs, iterate through file till EOF and return the number of IDAT chunks found

	const char* to_find = "IDAT\0";
	char IDAT_sig[5];
	unsigned int byte_num = 0;

	// reset file pointer at the start in case it was previously moved by calls like fread
	fseek(image, 0, SEEK_SET);

	fread(IDAT_sig, sizeof(unsigned char), 4, image);

	IDAT_sig[4] = '\0';

	while (strcmp(to_find, IDAT_sig) != 0 && !feof(image))
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

IDAT* search_IDATs(FILE* image)
{
	IDAT* IDATs = (IDAT*)(malloc(sizeof(IDAT) * 50));

	if (IDATs == NULL)
	{
		printf("memory for IDAT chunks not allocated\n");
		return NULL;
	}

	const char* signature = "IDAT\0";
	char IDAT_sig[5];
	uLong byte_num = 0;
	unsigned int chunk_num = 0;

	// reset file pointer at the start in case it was previously moved by calls like fread
	fseek(image, 0, SEEK_SET);

	while (!feof(image) && strcmp("IEND\0", IDAT_sig) != 0)
	{
		//cicle of freads until "IDAT" is found
		fread(IDAT_sig, sizeof(unsigned char), 4, image);

		IDAT_sig[4] = '\0';

		if (strcmp(signature, IDAT_sig) == 0)
		{
			IDAT chunk;

			chunk.location = byte_num;

			// chunk's size field is four bytes before the signature so go back 8 bytes because the signature was just read
			fseek(image, -8, SEEK_CUR);
			fread(&chunk.size, sizeof(unsigned char), 4, image);

			convert_endianness(&chunk.size);

			// now pointer is again at signature, skip its four bytes so that pointer is at the start of the data field
			fseek(image, 4, SEEK_CUR);
			// allicate size number of bytes
			chunk.data = (unsigned char*)malloc(chunk.size);

			if (chunk.data == NULL)
			{
				printf("error allocating IDAT data buffer");
				return NULL;
			}

			int read = fread(chunk.data, sizeof(unsigned char), chunk.size, image);

			if (read != chunk.size)
			{
				printf("IDAT chunk not read completely\n");
				return NULL;
			}

			IDATs[chunk_num] = chunk;
			chunk_num++;

			// when the signature is found then the number of bytes after it are chunk.size + 4 for the signature before + 4 for the CRC
			byte_num += chunk.size + 4 + 4;

			// go over CRC bytes
			fseek(image, 4, SEEK_CUR);
		}
		else // if signature wasn't found advance by one byte
		{
			byte_num++;
			// go back 3 bytes from the current pointer position to read bytes one after the other
			fseek(image, -3, SEEK_CUR);
		}
	}

	IDAT_num = chunk_num;

	return IDATs;
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
	IHDR ihdr = read_IHDR(image);
	uLong pixel_data_size = ihdr.image_byte_size;

	IDAT* idat_chunks = search_IDATs(image);

	uLong compressed_data_size = 0;
	unsigned char* compressed_data = NULL;

	// concatenate all idats data, IDAT_num is a global variable set in search_IDATs()
	for (int i = 0; i < IDAT_num; i++)
	{
		IDAT chunk = idat_chunks[i];

		unsigned char* block = (unsigned char*)realloc(compressed_data, compressed_data_size + chunk.size);

		// if the new block of memory was allocated successfully then make compressed_data point to block in case the new block of memory was moved for reallocation
		if (block != NULL)
			compressed_data = block;
		else if (compressed_data == NULL) // if block is null and also compressed data is null it means the first allocation failed
		{
			printf("error allocating new chunk of data\n");
			return NULL;
		}

		for (int j = 0; j < chunk.size; j++)
		{
			compressed_data[compressed_data_size + j] = chunk.data[j];
		}

		compressed_data_size += chunk.size;
	}

 	unsigned char* decomp_data = decomp(compressed_data, &pixel_data_size, compressed_data_size);

	free(idat_chunks);
	free(compressed_data);

	return decomp_data;
}

// image_size: number of width * height pixels * number of channels * sizeof(unsigned char)
unsigned char* concatenate_filtered_data(unsigned char* filtered_data, unsigned int width, unsigned int height, uint8_t num_of_channels, uint8_t bit_depth)
{
	unsigned int unfiltered_size = width * height * num_of_channels * (bit_depth / 8);
	unsigned char* raw_data = (unsigned char*)malloc(unfiltered_size);

	if (raw_data == NULL)
	{
		printf("not enough memory\n");
		return NULL;
	}

	unsigned int raw_index = 0;

	unsigned int i = 0;
	while (i < (width * num_of_channels * (bit_depth / 8) + 1) * height) // loop through all the filtered data
	{
		if (i % (width * num_of_channels * (bit_depth / 8) + 1) == 0)
			i++;
		else
		{
			// reconstruction can be done here based on the filter type read from the true condition of this if-else
			for (int j = 0; j < num_of_channels * (bit_depth / 8); j++)
			{
				raw_data[raw_index++] = filtered_data[i++];
			}
		}
	}

	return raw_data;
}

//reconstruct scanline by scanline and read each filter byte at the start of them
void reconstruct_filtered_data(unsigned char* filtered_data, size_t width, size_t height, uint8_t num_of_channels, uint8_t bit_depth)
{
	unsigned int scanline_size = width * num_of_channels * (bit_depth / 8) + 1;
	uint8_t filter_type = 0;

	for (int y = 0; y < height; y++)
	{
		// index of the first byte of a scanline: the filter type byte
		unsigned int scanline_start = y * scanline_size;
		filter_type = filtered_data[scanline_start];

		// reconstruct the filtered scanline according to its filter_type byte
		switch (filter_type)
		{
		case NONE:
		break;

		case SUB:
			// +1 sets the pointer to the start of the pixel_data after the filter type byte
			reconstruct_sub(filtered_data + scanline_start + 1, scanline_size, num_of_channels, bit_depth);
		break;

		case UP:
			reconstruct_up(filtered_data + scanline_start + 1, scanline_size, y, num_of_channels, bit_depth);
		break;

		case AVERAGE:
			reconstruct_average(filtered_data + scanline_start + 1, scanline_size, y, num_of_channels, bit_depth);
		break;

		case PAETH:
		break;
		}
	}
}

void reconstruct_sub(unsigned char* filtered, unsigned int scanline_size, uint8_t num_of_channels, uint8_t bit_depth)
{
	uint8_t a_byte_offset = num_of_channels * bit_depth / 8;

	for (int i = 0; i < scanline_size - 1; i++)
	{
		uint8_t a_byte = i - a_byte_offset < 0 ? 0 : filtered[i - a_byte_offset];

		// the unsigned type already performs the modulo 256 on the result of the sum so the value doesn't overflow a byte
		filtered[i] = filtered[i] + a_byte;
	}
}

void reconstruct_up(unsigned char* filtered, unsigned int scanline_size, unsigned int scanline_index, uint8_t num_of_channels, uint8_t bit_depth)
{
	// up byte: this byte index - the size of a scanline of pixels
	int b_byte_offset = scanline_size;

	for (int i = 0; i < scanline_size - 1; i++)
	{
		uint8_t b_byte = scanline_index - 1 < 0 ? 0 : filtered[i - b_byte_offset];

		filtered[i] = filtered[i] + b_byte;
	}
}

void reconstruct_average(unsigned char* filtered, unsigned int scanline_size, unsigned int scanline_index, uint8_t num_of_channels, uint8_t bit_depth)
{
	uint8_t a_byte_offset = num_of_channels * bit_depth / 8;
	int b_byte_offset = scanline_size;

	for (int i = 0; i < scanline_size - 1; i++)
	{
		// average operatioon should be done without overflow for the a_byte + b_byte sum, so declare these two values at least as short
		short a_byte = i - a_byte_offset < 0 ? 0 : filtered[i - a_byte_offset];
		short b_byte = scanline_index == 0 ? 0 : filtered[i - b_byte_offset];

		uint8_t average = (a_byte + b_byte) / 2;

		//filtered[i] = filtered[i] + average;
		filtered[i] = filtered[i] + average;
	}
}

//uLong image_get_IDAT_size()
//{
//	return img.IDATs[0]->c_length;
//}

//uLong image_get_pixel_data_size()
//{
//	return img.
//}