#pragma once

#include "pch.h"

#include <stdint.h>
#include <stdio.h>
#include <zlib.h>
#include <stdlib.h>
#include "compress.h"

//unsigned char* makeZLIB(unsigned char* in, size_t size, int& z_size) {
//
//    unsigned char def_out[CHUNK];
//
//    size_t out_size = 0;
//
//    int ret = def(in, def_out, size, out_size);
//    uint32_t csum;
//
//    if (ret != Z_OK) {
//        printf("error in compression: %d\n", ret);
//        return NULL;
//    }
//
//    // adler checksum
//    csum = adler32(0L, in, size);
//    unsigned char* csum_bytes = convert_crc(csum);
//
//    // size of ZLIB data: 2 bytes for CMF and FLG (see zpipe.txt + rfc 1950) + size of deflate compressed data + sizeof of checksum data
//    z_size = 2 + out_size + sizeof(csum);
//
//    //printf("%d\n", z_size);
//
//    unsigned char* zlib_out = (unsigned char*)malloc(z_size);
//    //unsigned char* zlib_out = (unsigned char*)malloc(17);
//
//    if (zlib_out == NULL) {
//        printf("memory not allocated");
//        return zlib_out;
//    }
//
//    // fill CMF and FLG bytes
//    zlib_out[0] = 0x08;
//    zlib_out[1] = 0xD7;
//
//    // loop through def out and fill zlib_data array with the deflate compressed data
//    // the offset in zlib data is 2 bytes (CMF and FLG bytes come before the compressed blocks of data)
//    for (int i = 0; i < out_size; i++) {
//        zlib_out[i + 2] = def_out[i];
//        printf("%x\n", def_out[i]);
//    }
//
//    int j = 2 + out_size;
//    for (int i = 0; i < sizeof(csum); i++) {
//        zlib_out[j + i] = csum_bytes[i];
//    }
//
//    return zlib_out;
//}

// return only the output of def without cmf, flg and adler32
unsigned char* comp(unsigned char* in, uLong pixel_data_size, uLong& z_size) {

    //unsigned char def_out[CHUNK];
    int ret;

    // in the function def, def_out points to the memory allocated with the size of the value return by deflateBound()
    unsigned char* zlib_out = NULL;

    z_size = compressBound(pixel_data_size);
    zlib_out = (unsigned char*)malloc(z_size);
    
    ret = compress(zlib_out, &z_size, in, pixel_data_size);

    if (ret != Z_OK) {
        printf("error in compression\n");
        return NULL;
    }

    return zlib_out;
}