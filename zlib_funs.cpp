#include "pch.h"
#include "zlib_funs.h"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

int def(unsigned char* in, unsigned char* out, size_t size, size_t& out_size)
{
    int ret, flush;
    z_stream strm;

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK)
        return ret;

    strm.avail_in = size; //bbl
    strm.next_in = in;

    flush = Z_FINISH;

    strm.avail_out = size;
    strm.next_out = out;

    ret = deflate(&strm, flush);
    assert(ret != Z_STREAM_ERROR);

    out_size = size - strm.avail_out;
    assert(strm.avail_in == 0);

    (void)deflateEnd;
    return Z_OK;
}


/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
int inf(unsigned char* in, unsigned char* out, size_t size, size_t& out_size)
{
    int ret;
    unsigned have;
    z_stream strm;

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = size;
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            out_size = CHUNK - strm.avail_out;
            //if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
            //    (void)inflateEnd(&strm);
            //    return Z_ERRNO;
            //}
        } while (strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

void to_8bit_array(uint32_t num, unsigned char* arr)
{
    for (int i = 0; i < sizeof(num); i++)
    {
        arr[i] = num >> 8 * (sizeof(num) - 1 - i);
    }
}

uint32_t adler(unsigned char* data, size_t len) {
    uint32_t a = 1, b = 0;
    size_t index;
    const uint32_t MOD_ADLER = 65521;

    // Process each byte of the data in order
    for (index = 0; index < len; ++index)
    {
        a = (a + data[index]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }

    return (b << 16) | a;
}


uint32_t crc_checksum(const unsigned char* s, size_t n) {
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < n; i++) {
        char ch = s[i];
        for (size_t j = 0; j < 8; j++) {
            uint32_t b = (ch ^ crc) & 1;
            crc >>= 1;
            if (b) crc = crc ^ 0xedb88320L;
            ch >>= 1;
        }
    }

    return crc;
}

unsigned char* convert_crc(unsigned long crc) {
    unsigned char b_crc[4];

    b_crc[0] = crc >> 24;
    b_crc[1] = crc >> 16;
    b_crc[2] = crc >> 8;
    b_crc[3] = crc;

    //for (int i = 0; i < 4; i++)
    //    printf("%x\n", b_crc[i]);

    return b_crc;
}