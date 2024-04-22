#pragma once
#include "zlib_funs.h"

unsigned char* makeZLIB(unsigned char* in, size_t size, int& z_size);

unsigned char* comp(unsigned char* in, size_t size, size_t& z_size);