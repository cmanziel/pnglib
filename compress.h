#pragma once
#include "zlib_funs.h"

unsigned char* makeZLIB(unsigned char* in, size_t size, int& z_size);

unsigned char* comp(unsigned char* in, uLong size, size_t& z_size);