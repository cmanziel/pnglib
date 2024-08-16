#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include "zlib.h"
#include <assert.h>

#define CHUNK 50000

int def(unsigned char* in, unsigned char* out, uLong size, uLong& out_size);

int inf(unsigned char* in, unsigned char* out, size_t size, size_t& out_size);

uint32_t adler(unsigned char* data, size_t len);

uint32_t crc_checksum(const unsigned char* s, size_t n);

unsigned char* convert_crc(unsigned long crc);

void to_8bit_array(uint32_t num, unsigned char* arr);

template <typename T>
void convert_endianness(T* num)
{
    if (sizeof(T) == 4)
        *num = (*num >> 24 & 0x000000FF | *num >> 8 & 0x0000FF00 | *num << 8 & 0x00FF0000 | *num << 24 & 0xFF000000);
}