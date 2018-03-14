/**
 * 
 *  author	Jose Mortensen
 *  brief	Iso Mp4 Box 
 * 	
 */
#include "stdafx.h"
#include "Mp4File.h"
#include "Mp4Box.h"
#include "Mp4FileImpl.h"
#include "Mp4IStreamSource.h"

//#include "intrin.h"
/*
#define htons _byteswap_ushort
#define htonl _byteswap_ulong
#define ntohs _byteswap_ushort
#define ntohl _byteswap_ulong
#define ntohll _byteswap_uint64
#define hllton _byteswap_uint64
*/

#define _byteswap_ushort htons 
#define _byteswap_ulong htonl 
#define _byteswap_uint64 ntohll 

Box::Box(std::string atype) {
	type = atype;
	siblings = 0;
	children = 0;
	consumed = 0;
	sizexx = 8;
	largesize = 8;

	//std::cout << "creating box " << this << " " << type.c_str() << std::endl;
}

Box::~Box() {
	if (children) delete children;
	if (siblings) delete siblings;
	//std::cout << "deleting box " << this << " " << type.c_str() << std::endl;
}


void Box::setDataSize(uint64_t newSize) {

	newSize = newSize + 8;

	sizexx = (uint32_t)(largesize = newSize);
	if (largesize > 0xffffffff)
		sizexx = 1;
}

void Box::increaseSize(uint64_t delta) {
	sizexx = (uint32_t)(largesize += delta);
	if (largesize > 0xffffffff)
		sizexx = 1;
}

Box* Box::add(Box* box) {
	if (children) {
		Box* last = children;
		while (last->siblings) {
			last = last->siblings;
		}
		last->siblings = box;
	} else {
		children = box;
	}

	increaseSize(box->largesize);

	return this;
}

void Box::readChar(large_istream& is, char* data, int size) {
	is.read((char*)data, size);
	data[size]=0;
}

uint32_t Box::read32(large_istream& is) {
	uint32_t ret = 0;
	is.read((char*)&ret,4);
	ret = _byteswap_ulong(ret);
	return ret;
}

uint16_t Box::read16(large_istream& is) {
	uint16_t ret = 0;
	is.read((char*)&ret,2);
	ret = _byteswap_ushort(ret);
	return ret;
}


uint64_t Box::read64(large_istream& is) {
	uint64_t ret = 0;
	is.read((char*)&ret, 8);
	ret = _byteswap_uint64(ret);
	return ret;
}

void Box::write32(large_ostream& os, uint32_t data) {
	data = _byteswap_ulong(data);
	os.write((char*)&data, 4);

}

void Box::write16(large_ostream& os, uint16_t data) {
	data = _byteswap_ushort(data);
	os.write((char*)&data, 2);
}

void Box::write64(large_ostream& os, uint64_t data) {
	data = _byteswap_uint64(data);
	os.write((char*)&data, 8);
}

std::string Box::readStr(large_istream& is) {
	char byte;
	std::string res;
	do {
		is.read(&byte, 1);
		if (byte!=0) res.push_back(byte);
	} while(byte!=0);
	return res;
}

void Box::writeStr(large_ostream& os, std::string str) {
	os.write(str.c_str(), str.size()+1);
}

void Box::write(large_ostream& os) {
	if (sizexx != 0)
		write32(os, sizexx);

	os.write(type.c_str(), 4);

	if (sizexx ==1)
		write64(os, largesize);

}

void Box::read(large_istream& is) {
	if (largesize-consumed) {
		children = Mp4FileImpl::readBox(is);
		consumed += children->largesize;
	}

	Box* child = 0;
	while (largesize-consumed) {
		if (children->siblings == 0) {
			children->siblings = Mp4FileImpl::readBox(is);
			child = children->siblings;
		} else {
			child->siblings = Mp4FileImpl::readBox(is);
			child = child->siblings;
		}
		consumed += child->largesize;
	}
}



void Box::print(std::ostream& os, std::string pre) const {
	os	<< pre << name.c_str() << " [" << type.c_str() << "] size = " << largesize << std::endl ;
	if (children)
		children->print(os, std::string("   ").append(pre).c_str());

	if (siblings)
		siblings->print(os, pre);
}

std::string Box::isoLanguage(uint16_t encoded) {
	std::string lang;
	char byte = (encoded & 0x1f) + 0x60;
	lang.append(&byte, 1);
	byte = ((encoded>>5) & 0x1f) + 0x60;
	lang.append(&byte, 1);
	byte = ((encoded>>10) & 0x1f) + 0x60;
	lang.append(&byte, 1);
	return lang;
}
	

uint16_t Box::isoLanguage(std::string lang) {
	uint16_t code = 0;
	if (lang.length() != 3) throw std::runtime_error ("Bad ISO 639 lenguage code");
	char byte = lang.at(0);
	code |= (byte - 0x60);
	code <<=5;
	byte = lang.at(1);
	code |= (byte - 0x60);
	code <<=5;
	byte = lang.at(2);
	code |= (byte - 0x60);

	return code;
}


std::ostream& operator<<(std::ostream& os, const Box& box) {
	box.print(os, "+--");
	return os;
}
