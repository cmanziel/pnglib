#pragma once

#include "pch.h"
#include "decompress.h"

unsigned char* decomp(unsigned char* in, uLong pixel_data_size, uLong def_size)
{
	// call uncompress function

	int ret;

	unsigned char* uncomp_data = (unsigned char*)malloc(pixel_data_size);

	ret = uncompress(uncomp_data, &pixel_data_size, in, def_size);

	if (ret != Z_OK)
	{
		printf("error during decompression:\n");

		switch (ret)
		{
		case Z_MEM_ERROR:
			printf("not enough memory\n");
		break;
		case Z_BUF_ERROR:
		{
			printf("output buffer not big enough\n");
			// return the uncompressed buffer filled until this point
			return uncomp_data;
		}
		break;
		case Z_DATA_ERROR:
			printf("incorrect input\n");
		break;
		}

		uncomp_data = NULL;
	}

	return uncomp_data;
}