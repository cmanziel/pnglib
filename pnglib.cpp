// pnglib.cpp : Defines the functions for the static library.

#include "pch.h"
#include "framework.h"
#include <stdio.h>
#include <stdint.h>
#include "make_png.h"

#pragma warning (disable : 4996)

FILE* create_image(unsigned char* data, const char* path, int width, int height)
{
	FILE* image = fopen(path, "wb");

	// in here, compress data with zlib and 
	make_png(image, data, width, height, path);

	fclose(image);

	return image;
}
