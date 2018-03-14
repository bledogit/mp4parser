/**
 * 
 *  author	Jose Mortensen
 *  brief	Iso Mp4 Box parser
 * 	
 */

#ifndef __MP4BOXES_H__
#define __MP4BOXES_H__

#include <string.h>
#include <istream>
#include <list>
#include "Mp4Box.h"

/**
	Placeholder for unknown boxes.  This structure
	keeps the content of the unknown boxes intact, so it
	can be written back to the file.
*/
class UnkownBox : public Box {
	char* contents;
	uint64_t count;

public:
	UnkownBox(std::string atype) : Box ( atype) {
		name = "Unknown Box";
	}

	void read(large_istream& is) {
		count = largesize-consumed;
		contents = new char[(int)( largesize-consumed)];
		is.read(contents, largesize-consumed);
		consumed = largesize;
	}

	void write(large_ostream& os) {
		Box::write(os);
		os.write(contents, count);
	}

	~UnkownBox() { 
		delete [] contents;
	}
};

/**
aligned(8) class FullBox(unsigned int(32) boxtype, unsigned int(8) v, bit(24) f) extends Box(boxtype) {
	unsigned int(8) version = v;
	bit(24) flags = f;
}
*/
class FullBox : public Box {

public: 
	uint8_t version;
	uint32_t flags;
	FullBox(std::string atype, uint8_t _version = 0, uint32_t _flags = 0) : Box (atype),
	version(_version), flags(_flags) {
		increaseSize(4);
	}

	virtual void setDataSize(uint64_t newSize) {
		Box::setDataSize(newSize + 4);
	}

	virtual void print(std::ostream& os, std::string pre) const {
		os	<< pre << name.c_str() << " [" << type.c_str() << "] size = " << largesize
		    << " version = " << (int) version << " flags = " << flags << std::endl ;
		
		if (children)
			children->print(os, pre.append("   ").c_str());

		if (siblings)
			siblings->print(os, pre);
	}

	virtual void read(large_istream& is) {
		flags = Box::read32(is);
		version = flags >> 24;
		flags &= 0xffffff;
		consumed += 4;
	}

	virtual void write(large_ostream& os) {
		Box::write(os);
		uint32_t data = flags || (version << 24);
		Box::write32(os, data);
	}
};

/**
	aligned(8) class MediaInformationBox extends Box(‘minf’) {}
*/
class Minf : public Box {
public:
	
	Minf () : Box("minf") {
		name = "MediaInformation";
	}
};




/**
	aligned(8) class MovieExtendsBox extends Box(‘mvex’){ }
*/
class Mvex : public Box {
public:
	
	Mvex () : Box("Mvex") {
		name = "MovieExtendsBox";
	}
};


/**
aligned(8) class MovieExtendsHeaderBox extends FullBox(‘mehd’, version, 0) { 
if (version==1) {
      unsigned int(64)  fragment_duration;
   } else { // version==0
      unsigned int(32)  fragment_duration;
   }
}
*/

class Mehd : public FullBox {
public:
	uint64_t creation_time;

	Mehd () : FullBox("Mehd") {
		name = "MovieExtendsHeaderBox";
	}

	void read(large_istream& is) {
		FullBox::read(is);
		if (version == 1) {
			creation_time = Box::read64(is);
			consumed += 8;
		} else if (version == 0) {
			creation_time = Box::read32(is);
			consumed += 4;
		} else {
			throw std::runtime_error("Unknown MovieExtendsHeaderBox Version");
		}
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		if (version == 1) {
			Box::write64(os, creation_time);
		} else if (version == 0) {
			Box::write32(os, (uint32_t)creation_time);
		} else {
			throw std::runtime_error("Unknown MovieExtendsHeaderBox Version");
		}
	}

};


/**
	aligned(8) class MediaBox extends Box(‘mdia’) {}
*/
class Mdia : public Box {
public:
	
	Mdia () : Box("mdia") {
		name = "Media";
	}
};



/** 
aligned(8) class FileTypeBox extends Box(‘ftyp’) {
	unsigned int(32) major_brand;
	unsigned int(32) minor_version;
	unsigned int(32) compatible_brands[]; // to end of the box
	}
*/
class Ftyp : public Box {
public:
	std::string major_brand;
	uint32_t minor_version;
	std::vector<std::string> compatible_brands; // to end of the box

	Ftyp () : Box("ftyp") {
		name = "FileType";
		major_brand = "unkn";
		minor_version = 0;
	}

	void addCompatible(std::string brand) {
		bool repeated = false;
		for (std::size_t i = 0; i < compatible_brands.size(); i++)
			if (compatible_brands[i].compare(brand) == 0)
				repeated = true;

		if (!repeated) {
			compatible_brands.push_back(brand);
			
		}
	}

	void pushMajor(std::string brand) {
		addCompatible(major_brand);
		major_brand = brand;
		sizexx = (uint32_t)(largesize = 16+compatible_brands.size()*4);
	}

	virtual void read(large_istream& is) {
		char brand[5];
		Box::readChar(is,brand, 4);
		major_brand = brand;
		minor_version = Box::read32(is);
		consumed += 8;
		while (consumed < largesize) {
			Box::readChar(is,brand, 4);
			addCompatible(brand);
			consumed += 4;
		}
	}

	void write(large_ostream& os) {
		Box::write(os);
		os.write(major_brand.c_str(), 4);
		Box::write32(os, minor_version);
		for (std::size_t i = 0; i < compatible_brands.size(); i++)
			os.write(compatible_brands[i].c_str(), 4);
	}

	void print(std::ostream& os, std::string pre) const {
		Box::print(os, pre);
		os << pre.c_str() << " mayor= " << major_brand.c_str() << " minor= " << minor_version << " brands= ";
		for (std::size_t i = 0; i < compatible_brands.size(); i++)
			os << compatible_brands[i].c_str() << " ";
	}
};

/**
aligned(8) class MovieBox extends Box(‘moov’){}
*/
class Moov : public Box {
public:
	
	Moov () : Box("moov") {
		name = "Movie";
	}
};

/**
	Trak Box
	aligned(8) class TrackBox extends Box(‘trak’) {}
	*/
class Trak : public Box {
	
public:
	Trak () : Box("trak") {
		name = "Track";
	}

};

/**
	aligned(8) class MediaDataBox extends Box(‘mdat’) { bit(8) data[]; }
*/
class Mdat : public Box {
public:
	uint64_t offset;
	uint64_t bytes;

	Mdat () : Box("mdat") {
		name = "MediaData";

	}

	void setDataSize(uint64_t newSize) {
		bytes = newSize;
		Box::setDataSize(newSize);
	}

	// Skip media 
	void read(large_istream& is) {
		offset = is.tellg();
		bytes = largesize - 8;
		is.seekg(largesize-consumed, std::ios_base::cur);
		consumed = largesize;
	}
	
	virtual void print(std::ostream& os, std::string pre) const {
		Box::print(os, pre);
		os << pre.c_str() << "Offset= " << offset << " Bytes: " << bytes << std::endl;
	}

	void write(large_ostream& os) {
		Box::write(os); // writes only the header
	}
};


/**
	aligned(8) class TrackHeaderBox extends FullBox(‘tkhd’, version, flags){
		if (version==1) {
			unsigned int(64) creation_time;
			unsigned int(64) modification_time;
			unsigned int(32) track_ID;
			const unsigned int(32) reserved = 0;
			unsigned int(64) duration;
		} else { // version==0
			unsigned int(32) creation_time;
			unsigned int(32) modification_time;
			unsigned int(32) track_ID;
			const unsigned int(32) reserved = 0;
			unsigned int(32) duration;
		}

		const unsigned int(32)[2] reserved = 0;
		template int(16) layer = 0;
		template int(16) alternate_group = 0;
		template int(16) volume = {if track_is_audio 0x0100 else 0};
		const unsigned int(16) reserved = 0;
		template int(32)[9] matrix=
		{ 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 };
		// unity matrix
		unsigned int(32) width;
		unsigned int(32) height;
	}
	*/
class Tkhd : public FullBox {
public:
	uint64_t creation_time;
	uint64_t modification_time;
	uint32_t track_ID;
	uint32_t reserved;
	uint64_t duration;

	uint32_t reserved2[2];
	uint16_t layer;
	uint16_t alternate_group;
	uint16_t volume;
	uint16_t reserved3;
	uint32_t matrix[9];
	uint32_t width;
	uint32_t height;

	Tkhd () : FullBox("tkhd") {
		reserved = 0;
		reserved2[0] = 0;
		reserved2[1] = 0;
		reserved3 = 0;
		width = height = layer = alternate_group = volume = 0;
		uint32_t unity[] = { 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 };
		memcpy(matrix, unity, sizeof(uint32_t[9]));
		name = "TrackHeader";
		if (version == 1)
			increaseSize(92);
		else
			increaseSize(80);
	}

	void read(large_istream& is) {
		FullBox::read(is);
		if (version == 1) {
			creation_time = Box::read64(is);
			modification_time = Box::read64(is);
			track_ID = Box::read32(is);
			reserved = Box::read32(is);
			duration = Box::read64(is);
			consumed += 32;
		} else if (version == 0) {
			creation_time = Box::read32(is);
			modification_time = Box::read32(is);
			track_ID = Box::read32(is);
			reserved = Box::read32(is);
			duration = Box::read32(is);
			consumed += 20;
		} else {
			throw std::runtime_error("Unknown TrackHeaderBox Version");
		}
		
		reserved2[0] = Box::read32(is);
		reserved2[1] = Box::read32(is);
		layer = Box::read16(is);
		alternate_group = Box::read16(is);
		volume = Box::read16(is);
		reserved3 = Box::read16(is);
		for (int i = 0; i < 9; i++)
			matrix[i] = Box::read32(is);
		width = Box::read32(is);
		height = Box::read32(is);

		consumed += 60;
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		if (version == 1) {
			Box::write64(os, creation_time);
			Box::write64(os, modification_time);
			Box::write32(os, track_ID);
			Box::write32(os, reserved);
			Box::write64(os, duration);
		} else if (version == 0) {
			Box::write32(os, (uint32_t)creation_time);
			Box::write32(os, (uint32_t)modification_time);
			Box::write32(os, track_ID);
			Box::write32(os, reserved);
			Box::write32(os, (uint32_t)duration);
		} else {
			throw std::runtime_error("Unknown TrackHeaderBox Version");
		}

		Box::write32(os, reserved2[0]);
		Box::write32(os, reserved2[1]);
		Box::write16(os, layer);
		Box::write16(os, alternate_group);
		Box::write16(os, volume);
		Box::write16(os, reserved3);
		for (int i = 0; i < 9; i++)
			Box::write32(os, matrix[i]);
		Box::write32(os, width);
		Box::write32(os, height);
	}
};

/**
aligned(8) class MediaHeaderBox extends FullBox(‘mdhd’, version, 0) {
if (version==1) {
unsigned int(64) creation_time;
unsigned int(64) modification_time;
unsigned int(32) timescale;
unsigned int(64) duration;
} else { // version==0
unsigned int(32) creation_time;
unsigned int(32) modification_time;
unsigned int(32) timescale;
unsigned int(32) duration;
}
bit(1) pad = 0;
unsigned int(5)[3] language; // ISO-639-2/T language code
unsigned int(16) pre_defined = 0;
}
*/
class Mdhd : public FullBox {
public:
	uint64_t creation_time;
	uint64_t modification_time;
	uint32_t timescale;
	uint64_t duration;
	uint16_t language;
	uint16_t pre_defined;


	Mdhd () : FullBox("mdhd") {
		name = "MediaHeader";
		pre_defined = 0;
		if (version == 1)
			increaseSize(28+4);
		else 
			increaseSize(16+4);
	}

	void read(large_istream& is) {
		FullBox::read(is);
		if (version == 1) {
			creation_time = Box::read64(is);
			modification_time = Box::read64(is);
			timescale = Box::read32(is);
			duration = Box::read64(is);
			consumed += 28;
		} else if (version == 0) {
			creation_time = Box::read32(is);
			modification_time = Box::read32(is);
			timescale = Box::read32(is);
			duration = Box::read32(is);
			consumed += 16;
		} else {
			throw std::runtime_error("Unknown TrackHeaderBox Version");
		}
		
		language = Box::read16(is);
		pre_defined = Box::read16(is);

		consumed += 4;
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		if (version == 1) {
			Box::write64(os, creation_time);
			Box::write64(os, modification_time);
			Box::write32(os, timescale);
			Box::write64(os, duration);
		} else if (version == 0) {
			Box::write32(os,  (uint32_t)creation_time);
			Box::write32(os,  (uint32_t)modification_time);
			Box::write32(os, timescale);
			Box::write32(os,  (uint32_t)duration);
		} else {
			throw std::runtime_error("Unknown TrackHeaderBox Version");
		}

		Box::write16(os, language);
		Box::write16(os, pre_defined);
	}
};

/**
aligned(8) class HandlerBox extends FullBox(‘hdlr’, version = 0, 0) {
	unsigned int(32) pre_defined = 0;
	unsigned int(32) handler_type;
	const unsigned int(32)[3] reserved = 0;
	string name;
}
*/
class Hdlr : public FullBox {
public:
	uint32_t pre_defined;		//4
	std::string handler_type;	//4
	uint32_t reserved[3];		//12

	std::string name_field;		// 1

	Hdlr () : FullBox("hdlr") {
		name = "Handler";
		pre_defined = 0;
		handler_type = "null";
		reserved[0] = 0;
		reserved[1] = 0;
		reserved[2] = 0;
		name_field = "";
		increaseSize(20+1); 

	}

	void setHandlerType(std::string type) {
		handler_type = type;
	}
	
	void setHandlerName(std::string name) {
		name_field = name;
		increaseSize(name_field.length());
	}
	
	void read(large_istream& is) {
		FullBox::read(is);
		pre_defined = Box::read32(is);
		char buffer[5];
		buffer[4] = 0;
		Box::readChar(is, buffer, 4);
		handler_type = buffer;

		reserved[0] = Box::read32(is);
		reserved[1] = Box::read32(is);
		reserved[2] = Box::read32(is);
		 
		consumed += 20;

		name_field = Box::readStr(is);
		
		consumed += name_field.length() + 1;
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		Box::write32(os, pre_defined);
		os.write(handler_type.c_str(), 4);
		Box::write32(os, reserved[0]);
		Box::write32(os, reserved[1]);
		Box::write32(os, reserved[2]);
		
		Box::writeStr(os, name_field);
	}

};


/**
aligned(8) class MovieHeaderBox extends FullBox(‘mvhd’, version, 0) {
	if (version==1) {
		unsigned int(64) creation_time;
		unsigned int(64) modification_time;
		unsigned int(32) timescale;
		unsigned int(64) duration;
	} else { // version==0
		unsigned int(32) creation_time;
		unsigned int(32) modification_time;
		unsigned int(32) timescale;
		unsigned int(32) duration;
	}
	template int(32) rate = 0x00010000; // typically 1.0
	template int(16) volume = 0x0100; // typically, full volume
	const bit(16) reserved = 0;
	const unsigned int(32)[2] reserved = 0;
	template int(32)[9] matrix =
	{ 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 };
	// Unity matrix
	bit(32)[6] pre_defined = 0;
	unsigned int(32) next_track_ID;
}
*/


class Mvhd : public FullBox {
public:
	uint64_t creation_time;
	uint64_t modification_time;
	uint32_t timescale;
	uint64_t duration;

	uint32_t rate;
	uint16_t volume;
	uint16_t reserved;
	uint32_t reserved2[2];
	uint32_t matrix[9];

	uint32_t pre_defined[6];
	uint32_t next_track_ID;

	Mvhd () : FullBox("mvhd") {
		name = "MovieHeader";
		rate = 0x00010000; // typically 1.0
		volume = 0x0100; // typically, full volume
		reserved = 0;
		memset(&reserved2, 0, sizeof(reserved2));

		uint32_t unity[] = { 0x00010000,0,0,0,0x00010000,0,0,0,0x40000000 };
		memcpy(matrix, unity, sizeof(uint32_t[9]));

		memset(&pre_defined, 0, sizeof(pre_defined));
	}

	void read(large_istream& is) {
		FullBox::read(is);
		if (version == 1) {
			creation_time = Box::read64(is);
			modification_time = Box::read64(is);
			timescale = Box::read32(is);
			duration = Box::read64(is);
			consumed += 28;
		} else if (version == 0) {
			creation_time = Box::read32(is);
			modification_time = Box::read32(is);
			timescale = Box::read32(is);
			duration = Box::read32(is);
			consumed += 16;
		} else {
			throw std::runtime_error("Unknown TrackHeaderBox Version");
		}

		rate = Box::read32(is);
		volume = Box::read16(is);
		reserved = Box::read16(is);
		reserved2[0] = Box::read32(is);
		reserved2[1] = Box::read32(is);
		
		for (int i =0 ; i < 9; i++)
			matrix[i] = Box::read32(is);

		for (int i =0 ; i <6; i++)
			pre_defined[i] = Box::read32(is);

		next_track_ID = Box::read32(is);
		consumed += 20*32;
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		if (version == 1) {
			Box::write64(os, creation_time);
			Box::write64(os, modification_time);
			Box::write32(os, timescale);
			Box::write64(os, duration);
		} else if (version == 0) {
			Box::write32(os,  (uint32_t)creation_time);
			Box::write32(os,  (uint32_t)modification_time);
			Box::write32(os, timescale);
			Box::write32(os,  (uint32_t)duration);
		} else {
			throw std::runtime_error("Unknown TrackHeaderBox Version");
		}

		Box::write32(os, rate);
		Box::write16(os, volume);
		Box::write16(os, reserved);
		Box::write32(os, reserved2[0]);
		Box::write32(os, reserved2[1]);

		for (int i =0 ; i < 9; i++)
			Box::write32(os, matrix[i]);
		
		for (int i =0 ; i < 6; i++)
			Box::write32(os, pre_defined[i]);

		Box::write32(os, next_track_ID);

	}

};
#endif //__MP4BOXES_H__
