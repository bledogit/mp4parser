/**
 * 
 *  author	Jose Mortensen
 *  brief	Iso Mp4 Box parser
 * 	
 */

#ifndef __MP4BOX_H__
#define __MP4BOX_H__

#include "stdafx.h"

#include <stdint.h>
#include <string>
#include <list>
#include <iostream>
#include "Mp4IStreamSource.h"

/**
	Base Box
	aligned(8) class Box (unsigned int(32) boxtype, optional unsigned int(8)[16] extended_type) {
		unsigned int(32) size;
		unsigned int(32) type = boxtype;
		if (size==1) {
			unsigned int(64) largesize;
		} else if (size==0) {
			// box extends to end of file
		}
		if (boxtype==‘uuid’) {
			unsigned int(8)[16] usertype = extended_type;
		}
	}
*/
class Box {

public:

	uint32_t sizexx;
	uint64_t largesize;
	std::string name;
	std::string	 type;
	Box*     siblings;
	Box*     children;
	uint64_t consumed;

	Box(std::string type);

	virtual ~Box();

	Box* add(Box* box);

	virtual void setDataSize(uint64_t newSize);

	void increaseSize(uint64_t delta);

	static void readChar(large_istream& is, char* data, int size); 

	static uint16_t read16(large_istream& is);

	static uint32_t read32(large_istream& is);

	static uint64_t read64(large_istream& is);

	static std::string readStr(large_istream& is);

	static void write16(large_ostream& os, uint16_t data);
	
	static void write32(large_ostream& os, uint32_t data);

	static void write64(large_ostream& os, uint64_t data);

	static void writeStr(large_ostream& os, std::string str);

	static std::string isoLanguage(uint16_t encoded);
	
	static uint16_t isoLanguage(std::string lang);

	virtual void read(large_istream& is);

	virtual void write(large_ostream& os);

	virtual void print(std::ostream& os, std::string pre) const;

	friend std::ostream& operator<<(std::ostream& os, const Box& box);
};

#endif //__MP4BOX_H__
