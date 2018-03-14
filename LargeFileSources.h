#ifndef __LARGEFILESOURCES_H__
#define __LARGEFILESOURCES_H__

#include <stdint.h>

class LargeIfstream {
	FILE* file;
	bool badF;
public:
	LargeIfstream(const char* filename);
	~LargeIfstream();
	bool bad();
	bool eof();
	void read(char* buffer, uint64_t size);
	void seekg(uint64_t pos, int from);
	uint64_t tellg();
	void clear();
	bool fail();

};

class LargeOfstream {
	FILE* file;
		bool badF;
public:
	LargeOfstream(const char* filename);
	~LargeOfstream();
	void flush();
	bool bad();
	uint64_t tellp();
	void write(const char* buffer, uint64_t size);
};

#endif
