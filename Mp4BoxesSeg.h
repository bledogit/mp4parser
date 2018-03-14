
#ifndef __MP4BOXESSEG_H__
#define __MP4BOXESSEG_H__

#include <vector>
#include "Mp4Box.h"
/**

aligned(8) class SegmentIndexBox extends FullBox(‘sidx’, version, 0) { 
	unsigned int(32) reference_ID;
	unsigned int(32) timescale;
	if (version==0) {
		unsigned int(32) earliest_presentation_time;
		unsigned int(32) first_offset; 
	}
	else {
		unsigned int(64) earliest_presentation_time; 
		unsigned int(64) first_offset;
	}
	unsigned int(16) reserved = 0; 
	unsigned int(16) reference_count; 
	for(i=1; i <= reference_count; i++) {
		bit (1) reference_type
		unsigned int(31) referenced_size
		unsigned int(32) subsegment_duration
		bit(1) starts_with_SAP
		unsigned int(3) SAP_type
		unsigned int(28) SAP_delta_time
	}
}
*/

class Sidx : public FullBox {
public:
	uint32_t reference_ID;
	uint32_t timescale;
	uint64_t earliest_presentation_time;
	uint64_t first_offset;
	uint32_t reference_count;
	uint16_t reserved;

	std::vector<uint32_t> reference;
	std::vector<uint32_t> subsegment_duration;
	std::vector<uint32_t> sap;

	Sidx () : FullBox("sidx") {
		name = "SegmentIndexBox";
	}

	void read(large_istream& is) {
		FullBox::read(is);
		reference_ID = Box::read32(is);
		timescale = Box::read32(is);

		consumed += 8;
		if (version == 0) {
			earliest_presentation_time = Box::read32(is);
			first_offset = Box::read32(is);
			consumed += 8;
		} else {
			earliest_presentation_time = Box::read64(is);
			first_offset = Box::read64(is);
			consumed += 16;
		}

		reserved = Box::read16(is);
		reference_count =  Box::read16(is);
		consumed += 4;

		for(int i=1; i <= reference_count; i++) {
			reference.push_back(Box::read32(is));
			subsegment_duration.push_back(Box::read32(is));
			sap.push_back(Box::read32(is));
			consumed += 12;
		}
	}

	void write(large_ostream& os) {
		FullBox::write(os);

		if (version == 0) {
			Box::write32(os, (uint32_t) earliest_presentation_time);
			Box::write32(os, (uint32_t) first_offset);
		} else {
			Box::write64(os, earliest_presentation_time);
			Box::write64(os, first_offset);
		}

		Box::write16(os, reserved);
		Box::write16(os, reference_count);

		for(int i = 1; i <= reference_count; i++) {
			Box::write32(os, reference[i]);
			Box::write32(os, subsegment_duration[i]);
			Box::write32(os, sap[i]);
		}
	}

	virtual void print(std::ostream& os, std::string pre) const {
		os	<< pre << name.c_str() << " [" << type.c_str() << "] size = " << largesize
		    << " version = " << (int) version << " flags = " << flags << " reference_count = " << reference_count << std::endl ;
		
		for(int i = 1; i <= reference_count; i++) {
			uint32_t reference_type = reference[i] >> 15;
			uint32_t referenced_size = reference[i] & 0x7f;

			os	<< pre << "  " << reference_type << ", " << referenced_size << ", " << subsegment_duration[i] << std::endl;
		}

		if (children)
			children->print(os, pre.append("   ").c_str());

		if (siblings)
			siblings->print(os, pre);
	}
};

#endif
