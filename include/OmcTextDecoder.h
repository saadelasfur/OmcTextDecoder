#ifndef OMC_TEXT_DECODER_H
#define OMC_TEXT_DECODER_H

#define VERSION "1.0"

#include <stddef.h>

void decodeBuffer(unsigned char* data, size_t len);
void encodeBuffer(unsigned char* data, size_t len);
unsigned char* decompressGzip(const unsigned char* src, size_t srcLen, size_t* destLen);
unsigned char* compressGzip(const unsigned char* src, size_t srcLen, size_t* destLen);
int isFileEncoded(const char* filename);
unsigned char* readFile(const char* filename, size_t* length);
int writeFile(const char* filename, const unsigned char* data, size_t length);
int isDirectory(const char* path);
void getParentDirectory(const char* path, char* parent);
void makeDirectoryRecursive(const char* path);

#endif
