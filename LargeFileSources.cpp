#include "stdafx.h"
#include "Mp4File.h"
#include "Mp4IStreamSource.h"

LargeIfstream::LargeIfstream(const char* filename) {
	file = fopen(filename, "rb");
	badF = (file == 0);
}

LargeIfstream::~LargeIfstream() {
	if (file) fclose(file);
}


bool LargeIfstream::bad() {

	return badF || ( ferror(file)!=0);
}

bool LargeIfstream::eof() {
	return feof(file);
}

void LargeIfstream::read(char* buffer, uint64_t size) {
	badF = fread(buffer, size, 1, file) != 1;
}

void LargeIfstream::seekg(uint64_t pos, int from) {
	if (fseek(file, pos, from) != 0) badF = true;
	//if (_fseeki64(file, pos, from) != 0) badF = true;
}

uint64_t LargeIfstream::tellg() {
	uint64_t pos = ftell(file);
	//uint64_t pos = _ftelli64(file);
	return pos;
}	

void LargeIfstream::clear() {
	badF = true;
}

bool LargeIfstream::fail() {

	return ferror(file)!=0;
}


LargeOfstream::LargeOfstream(const char* filename) {
	file = fopen(filename, "wb");
	badF = (file == 0);
}

LargeOfstream::~LargeOfstream() {
	if (file) fclose(file);
}


void LargeOfstream::flush() {
	fflush(file);
};

bool LargeOfstream::bad() {
	return ( badF || (ferror(file)!=0));
}

uint64_t LargeOfstream::tellp() {
//	return _ftelli64(file);
	return ftell(file);
}

void LargeOfstream::write(const char* buffer, uint64_t size) {
	badF = fwrite(buffer, size, 1, file) != 1;
}

