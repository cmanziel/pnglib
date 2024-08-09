# pnglib

Static library to encode or decode PNG files by compressing data into zlib datastreams or decompressing from them.

As explained in the PNG specification https://www.w3.org/TR/png-3/ a PNG file has to contain these three types of chunks of data.
- IHDR: image header, contains general information about the image representation
- IDAT: contains the actual compressed pixel data returned from zlib. It can be spread across multile IDAT chunks. In this implementation there will be one.
 - IEND: chunk at the end of the image

When encoding a PNG file the library calls the zlib's compression routines, sets the necessary fields for every chunk (like signatures and CRCs) and writes to the image file opened.

When decoding a PNG file the library searches all the IDAT chunks in the file, concatenates their data to get the whole zlib datastream, decompresses it to the original pixel data and returns it.