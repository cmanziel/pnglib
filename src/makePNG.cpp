#include "pch.h"
#include "make_png.h"

/*
	problem: if the size of the data is greater then CHUNK the rest of the data after chunk bytes are compressed is not compressed
	def function should process the data CHUNK bytes a time
*/

/* TODO:
	* modify def function to be able to compress data whcih size is greater than CHUNK
*/

void make_png(FILE* image, unsigned char* data, uint32_t width, uint32_t height, const char* path)
{
	// PNG signature for every non corrupted png file
	unsigned char signature[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

	fwrite(signature, 1, sizeof(signature), image);

	chunk* IHDR, * IDAT, * IEND;

	IHDR = (chunk*)malloc(sizeof(chunk));
	IDAT = (chunk*)malloc(sizeof(chunk));
	IEND = (chunk*)malloc(sizeof(chunk));

	if (IHDR == NULL || IDAT == NULL || IEND == NULL) {
		printf("chunk not allocated\n");
		return;
	}

	// set every chunk's field depending on its type and the write it to the image file
	set_IHDR(image, IHDR, width, height);
	set_IDAT(image, IDAT, data, width, height);
	set_IEND(image, IEND);

	free(IHDR);
	free(IDAT);
	free(IEND);
}

void set_type(unsigned char* chunk_type, const char* type_name)
{
	for (int i = 0; i < 4; i++)
	{
		chunk_type[i] = type_name[i];
	}
}

void fill_data(unsigned char* png_data, unsigned char* data, int& byte_count, int start, int end) {
	for (int i = start; i < end; i++) {
		png_data[byte_count] = data[i];
		byte_count++;
	}
}

void set_IHDR(FILE* image, chunk* IHDR, uint32_t width, uint32_t height)
{
	int IHDR_n_bytes = 0;

	IHDR->c_length = 0x0000000D;
	set_type(IHDR->c_type, "IHDR");

	IHDR->c_data = (unsigned char*)malloc(IHDR->c_length);

	if (IHDR->c_data == NULL) {
		printf("chunk data field not allocated");
		return;
	}

	unsigned char width_bytes[sizeof(uint32_t)];
	unsigned char height_bytes[sizeof(uint32_t)];

	to_8bit_array(width, width_bytes);
	to_8bit_array(height, height_bytes);

	IHDR->c_data[IHDR_n_bytes] = width_bytes[0]; IHDR_n_bytes++;
	IHDR->c_data[IHDR_n_bytes] = width_bytes[1]; IHDR_n_bytes++;
	IHDR->c_data[IHDR_n_bytes] = width_bytes[2]; IHDR_n_bytes++;
	IHDR->c_data[IHDR_n_bytes] = width_bytes[3]; IHDR_n_bytes++;

	IHDR->c_data[IHDR_n_bytes] = height_bytes[0]; IHDR_n_bytes++;
	IHDR->c_data[IHDR_n_bytes] = height_bytes[1]; IHDR_n_bytes++;
	IHDR->c_data[IHDR_n_bytes] = height_bytes[2]; IHDR_n_bytes++;
	IHDR->c_data[IHDR_n_bytes] = height_bytes[3]; IHDR_n_bytes++;

	IHDR->c_data[IHDR_n_bytes] = 0x08; IHDR_n_bytes++;
	IHDR->c_data[IHDR_n_bytes] = 0x02; IHDR_n_bytes++;
	IHDR->c_data[IHDR_n_bytes] = 0x00; IHDR_n_bytes++;
	IHDR->c_data[IHDR_n_bytes] = 0x00; IHDR_n_bytes++;
	IHDR->c_data[IHDR_n_bytes] = 0x00;

	unsigned char* checksum_data = (unsigned char*)malloc(IHDR->c_length + sizeof(IHDR->c_type));

	if (checksum_data == NULL) {
		printf("checksum dat not allocated");
		return;
	}

	checksum_data[0] = IHDR->c_type[0];
	checksum_data[1] = IHDR->c_type[1];
	checksum_data[2] = IHDR->c_type[2];
	checksum_data[3] = IHDR->c_type[3];

	for (int i = 4; i < IHDR->c_length + 4; i++)
	{
		checksum_data[i] = IHDR->c_data[i - 4];
	}

	IHDR->checksum = crc32(0L, checksum_data, IHDR->c_length + sizeof(IHDR->c_type));

	unsigned char length_bytes[sizeof(uint32_t)];
	unsigned char checksum_bytes[sizeof(uint32_t)];

	to_8bit_array(IHDR->c_length, length_bytes);
	to_8bit_array(IHDR->checksum, checksum_bytes);

	fwrite(length_bytes, sizeof(length_bytes), 1, image);
	fwrite(IHDR->c_type, sizeof(IHDR->c_type), 1, image);
	fwrite(IHDR->c_data, 1, IHDR->c_length, image);
	fwrite(checksum_bytes, sizeof(checksum_bytes), 1, image);

	free(checksum_data);
	free(IHDR->c_data);
}

void set_IDAT(FILE* image, chunk* IDAT, unsigned char* pixel_data, uint32_t width, uint32_t height)
{
	// allocate the total number of pixels, with compression the data written to the file is smaller
	uLong zlib_size = 0;

	uLong pixel_data_size = (width * CHANNELS_PER_PIXEL + 1) * height;

	unsigned char* zlib_data = comp(pixel_data, pixel_data_size, zlib_size);

	IDAT->c_data = zlib_data;
	IDAT->c_length = zlib_size;
	set_type(IDAT->c_type, "IDAT");

	//for (int i = 0; i < IDAT->c_length; i++)
	//{
	//	printf("%d\n", IDAT->c_data[i]);
	//}

	unsigned char* checksum_data = (unsigned char*)malloc(IDAT->c_length + sizeof(IDAT->c_type));

	if (checksum_data == NULL) {
		printf("checksum data not allocated");
		return;
	}

	checksum_data[0] = IDAT->c_type[0];
	checksum_data[1] = IDAT->c_type[1];
	checksum_data[2] = IDAT->c_type[2];
	checksum_data[3] = IDAT->c_type[3];

	for (int i = 4; i < IDAT->c_length + 4; i++)
	{
		checksum_data[i] = IDAT->c_data[i - 4];
	}

	IDAT->checksum = crc32(0L, checksum_data, IDAT->c_length + sizeof(IDAT->c_type));

	unsigned char length_bytes[sizeof(uint32_t)];
	unsigned char checksum_bytes[sizeof(uint32_t)];

	to_8bit_array(IDAT->c_length, length_bytes);
	to_8bit_array(IDAT->checksum, checksum_bytes);

	fwrite(length_bytes, sizeof(length_bytes), 1, image);
	fwrite(IDAT->c_type, sizeof(IDAT->c_type), 1, image);
	fwrite(IDAT->c_data, 1, IDAT->c_length, image);
	fwrite(checksum_bytes, sizeof(uint32_t), 1, image);

	free(checksum_data);
	// IDAT->c_data is allocated in the comp function which calls the zlib's def function
	free(IDAT->c_data);
}

void set_IEND(FILE* image, chunk* IEND)
{
	IEND->c_length = 0x00000000;
	set_type(IEND->c_type, "IEND");

	IEND->checksum = crc32(0L, IEND->c_type, sizeof(IEND->c_type));

	unsigned char length_bytes[sizeof(uint32_t)];
	unsigned char checksum_bytes[sizeof(uint32_t)];

	to_8bit_array(IEND->c_length, length_bytes);
	to_8bit_array(IEND->c_length, checksum_bytes);

	fwrite(length_bytes, sizeof(length_bytes), 1, image);
	fwrite(IEND->c_type, sizeof(IEND->c_type), 1, image);
	fwrite(checksum_bytes, sizeof(uint32_t), 1, image);
}