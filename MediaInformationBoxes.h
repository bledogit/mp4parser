/**
 * 
 *  author	Jose Mortensen
 *  brief	Iso Mp4 Box parser
 * 	
 */

#ifndef __MEDIAINFORMATIONBOXES_H__
#define __MEDIAINFORMATIONBOXES_H__

#include <string.h>
#include <istream>
#include <list>
#include <vector>
#include "Mp4Box.h"
#include "Mp4FileImpl.h"

/**
aligned(8) class VideoMediaHeaderBox extends FullBox(‘vmhd’, version = 0, 1) {
	template unsigned int(16) graphicsmode = 0; // copy, see below
	template unsigned int(16)[3] opcolor = {0, 0, 0};
}
*/
class Vmhd : public FullBox {
	uint16_t graphicsmode;
	uint16_t opcolor[3];
public:
	Vmhd () : FullBox("vmhd") {
		name = "VideoMediaHeader";
		graphicsmode = 0;
		opcolor[0] = 0;
		opcolor[1] = 0;
		opcolor[2] = 0;
		increaseSize(4);
	}

	virtual void read(large_istream& is) {
		FullBox::read(is);

		graphicsmode = Box::read16(is);
		opcolor[0] = Box::read16(is);
		opcolor[1] = Box::read16(is);
		opcolor[2] = Box::read16(is);
		consumed += 8;

	}

	void write(large_ostream& os) {
		FullBox::write(os);
		Box::write16(os, graphicsmode);
		Box::write16(os, opcolor[0]);
		Box::write16(os, opcolor[1]);
		Box::write16(os, opcolor[2]);
	}
};


/**
aligned(8) class SoundMediaHeaderBox extends FullBox(‘smhd’, version = 0, 0) {
	template int(16) balance = 0;
	const unsigned int(16) reserved = 0;
}}
*/
class Smhd : public FullBox {
	uint16_t balance;
	uint16_t reserved;
public:
	Smhd () : FullBox("smhd") {
		name = "SoundMediaHeader";
		balance = 0;
		reserved = 0;
		increaseSize(4);
	}

	virtual void read(large_istream& is) {
		FullBox::read(is);
		balance = Box::read16(is);
		reserved = Box::read16(is);
		consumed += 4;
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		Box::write16(os, balance);
		Box::write16(os, reserved);
	}
};

/**
aligned(8) class NullMediaHeaderBox extends FullBox(’nmhd’, version = 0, flags) {}
*/
class Nmhd : public FullBox {
public:
	
	Nmhd () : FullBox("nmhd") {
		name = "NullMediaHeader";
	}
};	


/**
aligned(8) class DataInformationBox extends Box(‘dinf’) {}
*/
class Dinf : public Box {
public:
	
	Dinf () : Box("dinf") {
		name = "DataInformation";
	}
};

/**
aligned(8) class DataReferenceBox extends FullBox(‘dref’, version = 0, 0) {
	unsigned int(32) entry_count;
	for (i=1; i <= entry_count; i++) {
		DataEntryBox(entry_version, entry_flags) data_entry;
	}
}
*/

class Dref : public FullBox {
	uint32_t entry_count;
	std::vector<Box*> boxes;

public:
	Dref () : FullBox("dref") {
		entry_count = 0;
		name = "DataReference";
		increaseSize(4);
	}

	~Dref() {
		uint32_t count = entry_count;
		while(count--) {
			Box* box = boxes[count];
			delete box;
		}
	}

	void addDataEntry(Box* box) {
		boxes.push_back(box);
		entry_count ++;
		increaseSize(box->largesize);
	}

	void read(large_istream& is) {
		FullBox::read(is);
		entry_count = Box::read32(is);
		consumed+=4;
		for ( uint32_t i = 0; i < entry_count; i++)  {
			Box* box = Mp4FileImpl::readBox(is);
			boxes.push_back(box);
			consumed += box->largesize;
		}
		if (largesize-consumed)
			throw std::runtime_error("Wrong Dref Count");
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		Box::write32(os, entry_count);
		//uint32_t count = entry_count;
		for (uint32_t i = 0; i < entry_count; i++)  {
			Box* box = boxes[i];
			Mp4FileImpl::writeBox(os, *box);
		}
		
	}
};


class DataEntryBox : public FullBox {
public:
	DataEntryBox (std::string atype) : FullBox(atype) {
		name = "DataEntryUrl";
	}
};

/**
aligned(8) class DataEntryUrlBox (bit(24) flags) extends FullBox(‘url ’, version = 0, flags) {
	string location;
}
*/
class Url_ : public DataEntryBox {
	std::string location;
public:
	Url_ () : DataEntryBox("url ") {
		name = "DataEntryUrl";
	}

	void read(large_istream& is) {
		FullBox::read(is);
		if (largesize-consumed) {
			location = Box::readStr(is);
			consumed += location.length()+1;
		}
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		//Box::writeStr(os,  location);
	}
};

#endif //__MEDIAINFORMATIONBOXES_H__
