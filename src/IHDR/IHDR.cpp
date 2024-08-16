#include "pch.h"
#include "IHDR.h"

IHDR read_IHDR(FILE* image)
{
	IHDR ihdr;

	ihdr.image_width = image_get_width(image);
	ihdr.image_height = image_get_height(image);
	ihdr.bit_depth = image_get_bit_depth(image);
	ihdr.color_type = image_get_color_type(image);

	uint8_t channels_per_pixel = image_get_channels_per_pixel(image);

	ihdr.image_byte_size = (ihdr.image_width * channels_per_pixel + 1) * ihdr.image_height * (ihdr.bit_depth / 8);

	return ihdr;
}

uInt image_get_width(FILE* image)
{
	uInt width;

	fseek(image, IHDR_SIGNATURE_LOCATION + 4, SEEK_SET);
	fread(&width, sizeof(unsigned int), 1, image);

	convert_endianness(&width);

	return width;
}

uInt image_get_height(FILE* image)
{
	uInt height;

	fseek(image, IHDR_SIGNATURE_LOCATION + 8, SEEK_SET);
	fread(&height, sizeof(unsigned int), 1, image);

	convert_endianness(&height);

	return height;
}

uint8_t image_get_bit_depth(FILE* image)
{
	uint8_t bit_depth;

	fseek(image, IHDR_SIGNATURE_LOCATION + 12, SEEK_SET);
	fread(&bit_depth, sizeof(unsigned char), 1, image);

	return bit_depth;
}

uint8_t image_get_color_type(FILE* image)
{
	uint8_t color_type;

	fseek(image, IHDR_SIGNATURE_LOCATION + 13, SEEK_SET);
	fread(&color_type, sizeof(unsigned char), 1, image);

	return color_type;
}

uint8_t image_get_channels_per_pixel(FILE* image)
{
	uint8_t color_type = image_get_color_type(image);

	switch (color_type)
	{
	case GREYSCALE:
		return 1;
	case INDEXED_COLOR:
		return 1;
	case GREYSCALE_ALPHA:
		return 2;
	case TRUECOLOR:
		return 3;
	case TRUECOLOR_ALPHA:
		return 4;
	}

	return 3;
}