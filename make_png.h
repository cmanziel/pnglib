#pragma once

#pragma warning (disable : 4996)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "zlib.h"
#include "compress.h"

void make_png(FILE* image, unsigned char* data, uint32_t width, uint32_t height, const char* path);