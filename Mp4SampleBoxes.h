/**
 * 
 *  author	Jose Mortensen
 *  brief	Iso Mp4 Sample Boxes
 * 	
 */

#ifndef __MP4SAMPLEBOXES_H__
#define __MP4SAMPLEBOXES_H__

#include <vector>
#include "Mp4IStreamSource.h"

/**
	aligned(8) class SampleTableBox extends Box(‘stbl’) {}
*/
class Stbl : public Box {
public:
	Stbl () : Box("stbl") {
		name = "Sample Table Box";
	}
};

/**
aligned(8) class SampleDescriptionBox (unsigned int(32) handler_type) extends FullBox('stsd', 0, 0){
	int i ;
	unsigned int(32) entry_count;
	for (i = 1 ; i <= entry_count ; i++){
		switch (handler_type){
			case ‘soun’: // for audio tracks
			AudioSampleEntry();
			break;
			case ‘vide’: // for video tracks
			VisualSampleEntry();
			break;
			case ‘hint’: // Hint track
			HintSampleEntry();
			break;
			case ‘meta’: // Metadata track
			MetadataSampleEntry();
			break; 
			}
		}
	}
}
*/
class Stsd : public FullBox {
	
	
	uint32_t entry_count;
	std::vector<Box*> boxes;

public:
	Stsd () : FullBox("stsd") {
		name = "SampleDescription";
		entry_count = 0;
		increaseSize(4);
	}

	void addBox(Box* box) {
		entry_count ++;
		increaseSize(box->largesize);
		boxes.push_back(box);
	}

	virtual void read(large_istream& is) {
		FullBox::read(is);
		entry_count = Box::read32(is);
		consumed+=4;
		uint32_t count = entry_count;
		while(count--) {
			Box* box = Mp4FileImpl::readBox(is);
			boxes.push_back(box);
			consumed += box->largesize;
		}
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		Box::write32(os, entry_count);
		uint32_t count = entry_count;
		while(count--) {
			Box* box = boxes[count];
			Mp4FileImpl::writeBox(os, *box);
		}
	}

	~Stsd() {
		for(std::size_t i = 0; i < boxes.size(); i++) {
			Box* box = boxes[i];
			delete box;
		}
	}
};


/**
class BitRateBox extends Box(‘btrt’){
	unsigned int(32) bufferSizeDB;
	unsigned int(32) maxBitrate;
	unsigned int(32) avgBitrate;
}
*/
class Btrt : public Box {
	
public:
	uint32_t bufferSizeDB;
	uint32_t maxBitrate;
	uint32_t avgBitrate;

	Btrt () : Box("btrt") {
		name = "Bitrate";
		bufferSizeDB = 0;
		maxBitrate = 0;
		avgBitrate = 0;
	}

	virtual void read(large_istream& is) {
		Box::read(is);
		bufferSizeDB = Box::read32(is);
		maxBitrate = Box::read32(is);
		avgBitrate = Box::read32(is);
		consumed += 12;
	}

	virtual void write(large_ostream& os) {
		Box::write(os);
		Box::write32(os, bufferSizeDB);
		Box::write32(os, maxBitrate);
		Box::write32(os, avgBitrate);
	}
};


/**
aligned(8) abstract class SampleEntry (unsigned int(32) format) extends Box(format){
	const unsigned int(8)[6] reserved = 0;
	unsigned int(16) data_reference_index;
}
 */

class SampleEntry : public Box {
	
public:
	uint8_t reserved[6];
	uint16_t data_reference_index;

	SampleEntry (std::string type) : Box(type) {
		memset(reserved, 0, 6);
		data_reference_index = 1;
		increaseSize(8);
	}

	virtual void read(large_istream& is) {
		is.read((char*)&reserved, 6);
		data_reference_index = Box::read16(is);
		consumed += 8;
	}

	virtual void write(large_ostream& os) {
		Box::write(os);
		os.write((char*)reserved, 6);
		Box::write16(os, data_reference_index);
	}
};

/**
class MetaDataSampleEntry(codingname) extends SampleEntry (codingname) {
}
*/
class MetaDataSampleEntry : public SampleEntry {
	
public:
	MetaDataSampleEntry (std::string condingname) : SampleEntry(condingname) {
	}
};

/**
class TextMetaDataSampleEntry() extends MetaDataSampleEntry (‘mett’) {
	string content_encoding; // optional
	string mime_format;
	BitRateBox (); // optional
}
*/
class Mett : public MetaDataSampleEntry {
	std::string content_encoding;
	std::string mime_format;
	Box* bitrate;
public:
	Mett () : MetaDataSampleEntry("mett") {
		name = "TextMetaDataSampleEntry";
		bitrate = 0;
		increaseSize(2);
	}

	void setContentEncoding(std::string content) {
		content_encoding = content;
		increaseSize(content_encoding.length()); // + 1 already accounted
	}

	void setMime(std::string mime) {
		mime_format = mime;
		increaseSize(mime_format.length()); // + 1 already accounted
	}

	virtual void read(large_istream& is) {
		SampleEntry::read(is);
		content_encoding = Box::readStr(is);
		mime_format = Box::readStr(is);
		consumed += content_encoding.length() + mime_format.length() + 2;
		if (largesize-consumed) {
			bitrate = Mp4FileImpl::readBox(is);
		}
	}

	void write(large_ostream& os) {
		MetaDataSampleEntry::write(os);
		Box::writeStr(os, content_encoding);
		Box::writeStr(os, mime_format);
		if (bitrate)
			Mp4FileImpl::writeBox(os, *bitrate);
	}
};

/**
	Common class to access chunck offsets
	*/
class Mp4Co {
public:
	virtual void addOffset(uint64_t offset) = 0;
	virtual uint64_t getOffset(uint32_t item) = 0;
	virtual void setOffset(uint64_t offset, uint32_t item) = 0;
	virtual uint32_t getEntryCount() = 0;
};

/**
aligned(8) class ChunkOffsetBox extends FullBox(‘stco’, version = 0, 0) {
	unsigned int(32) entry_count;
	for (i=1; i <= entry_count; i++) {
		unsigned int(32) chunk_offset;
	}
}
*/
class Stco : public FullBox, public Mp4Co {

public:
	uint32_t entry_count;
	std::vector<uint32_t> chunk_offsets;

	Stco () : FullBox("stco") {
		name = "ChunkOffset";
		entry_count = 0;
		increaseSize(4);
	}

	void addOffset(uint64_t offset)
	{
		entry_count++;
		chunk_offsets.push_back((uint32_t)offset);
		increaseSize(4);
	}

	uint64_t getOffset(uint32_t item) {
		return chunk_offsets[item];
	}

	uint32_t getEntryCount() { return entry_count;}

	void setOffset(uint64_t offset, uint32_t item) {
		chunk_offsets[item] = (uint32_t)offset;
	}

	void read(large_istream& is) {
		FullBox::read(is);

		entry_count = Box::read32(is);
		consumed += 4;

		for (uint32_t i = 0; i < entry_count; i++) {
			chunk_offsets.push_back(Box::read32(is));
			consumed += 4;
		}

		if (largesize-consumed) 
			throw std::runtime_error("Wrong Stco count");
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		Box::write32(os, entry_count);
		for (uint32_t i = 0; i < entry_count; i++) {
			Box::write32(os, chunk_offsets[i]);
		}
		
	}
};

/**
aligned(8) class ChunkLargeOffsetBox extends FullBox(‘co64’, version = 0, 0) {
	unsigned int(32) entry_count;
	for (i=1; i <= entry_count; i++) {
		unsigned int(64) chunk_offset;
	}
}
*/
class Co64 : public FullBox, public Mp4Co  {
	uint32_t entry_count;
	std::vector<uint64_t> chunk_offsets;
public:
	Co64 () : FullBox("co64") {
		name = "ChunkLargeOffset";
		entry_count = 0;
		increaseSize(4);

	}

	void addOffset(uint64_t offset)
	{
		entry_count++;
		chunk_offsets.push_back(offset);
		increaseSize(8);
	}

	uint64_t getOffset(uint32_t item) {
		return chunk_offsets[item];
	}
	
	uint32_t getEntryCount() { return entry_count;}

	void setOffset(uint64_t offset, uint32_t item) {
		chunk_offsets[item] = offset;
	}

	virtual void read(large_istream& is) {
		FullBox::read(is);

		entry_count = Box::read32(is);
		consumed += 4;

		for (uint32_t i = 0; i < entry_count; i++) {
			chunk_offsets.push_back(Box::read64(is));
			consumed += 8;
		}

		if (largesize-consumed) 
			throw std::runtime_error("Wrong Co64 count");
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		Box::write32(os, entry_count);
		for (uint32_t i = 0; i < entry_count; i++) {
			Box::write64(os, chunk_offsets[i]);
		}
		
	}
};

/**
aligned(8) class TimeToSampleBox extends FullBox(’stts’, version = 0, 0) {
	unsigned int(32) entry_count;
	for (int i=0; i < entry_count; i++) {
	unsigned int(32) sample_count;
	unsigned int(32) sample_delta;
	}
}
*/

class Stts : public FullBox {
public:

	struct SttsSample {
		uint32_t sample_count;
		uint32_t sample_delta;
	};


	uint32_t entry_count;
	std::vector<SttsSample> samples;

	Stts () : FullBox("stts") {
		name = "TimeToSample";
		entry_count = 0;
		increaseSize(4);
	}

	void addSttsSample(uint32_t count, uint32_t delta) {
		SttsSample sample;
		sample.sample_count = count;
		sample.sample_delta = delta;
		samples.push_back(sample);
		entry_count++;
		increaseSize(8);
	}

	virtual void read(large_istream& is) {
		FullBox::read(is);

		entry_count = Box::read32(is);
		consumed += 4;
		SttsSample sample;
		for (uint32_t i = 0; i < entry_count; i++)  {
			sample.sample_count = Box::read32(is);
			sample.sample_delta = Box::read32(is);
			consumed += 8;
			samples.push_back(sample);
		}
		if (largesize-consumed) 
			throw std::runtime_error("Wrong STTS count");
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		Box::write32(os, entry_count);
		SttsSample sample;
		for (uint32_t i = 0; i < entry_count; i++)  {
			sample = samples[i];
			Box::write32(os, sample.sample_count);
			Box::write32(os, sample.sample_delta);
		}
		
	}
};

/**

aligned(8) class SampleToChunkBox extends FullBox(‘stsc’, version = 0, 0) {
	unsigned int(32) entry_count;
	for (i=1; i <= entry_count; i++) {
		unsigned int(32) first_chunk;
		unsigned int(32) samples_per_chunk;
		unsigned int(32) sample_description_index;
	}
}

*/

class Stsc : public FullBox {
public:
	struct StscSample {
		uint32_t first_chunk;
		uint32_t samples_per_chunk;
		uint32_t sample_description_index;
	};

	uint32_t entry_count;
	std::vector<StscSample> samples;


	Stsc () : FullBox("stsc") {
		name = "SampleToChunk";
		entry_count = 0;
		increaseSize(4);
	}

	void addStscSample(uint32_t first_chunk, uint32_t samples_per_chunk, uint32_t sample_description_index) {
		StscSample sample;
		sample.first_chunk = first_chunk;
		sample.samples_per_chunk = samples_per_chunk;
		sample.sample_description_index = sample_description_index;
		samples.push_back(sample);
		entry_count++;
		increaseSize(12);
	}

	virtual void read(large_istream& is) {
		FullBox::read(is);

		entry_count = Box::read32(is);
		consumed += 4;
		StscSample sample;
		for (uint32_t i = 0; i < entry_count; i++) {
			sample.first_chunk = Box::read32(is);
			sample.samples_per_chunk = Box::read32(is);
			sample.sample_description_index = Box::read32(is);
			consumed += 12;
			samples.push_back(sample);
		}
		if (largesize-consumed) 
			throw std::runtime_error("Wrong STCS count");
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		Box::write32(os, entry_count);
		StscSample sample;
		for (uint32_t i = 0; i < entry_count; i++) {
			sample = samples[i];
			Box::write32(os, sample.first_chunk);
			Box::write32(os, sample.samples_per_chunk);
			Box::write32(os, sample.sample_description_index);
		}
		
	}
};

/**
aligned(8) class SampleSizeBox extends FullBox(‘stsz’, version = 0, 0) {
	unsigned int(32) sample_size;
	unsigned int(32) sample_count;
	if (sample_size==0) {
		for (i=1; i <= sample_count; i++) {
			unsigned int(32) entry_size;
		}
	}
}
*/

class Stsz : public FullBox {

public:

	uint32_t sample_size;
	uint32_t entry_count;
	std::vector<uint32_t> sizes;


	Stsz () : FullBox("stsz") {
		name = "SampleSizeBox";
		sample_size = 0;
		entry_count = 0;
		increaseSize(8);
	}

	uint32_t getSize(std::size_t entry) {
		if (entry >= entry_count)
			throw std::runtime_error ("Stsc Invalid Entry");

		if (sample_size!=0) {
			return sample_size;
		} else {
			return sizes[entry];
		}
	}

	void addStszSample(uint32_t sample_size) {
		entry_count++;
		sizes.push_back(sample_size);
		increaseSize( 4);
	}

	virtual void read(large_istream& is) {
		FullBox::read(is);
		sample_size = Box::read32(is);
		entry_count = Box::read32(is);
		consumed += 8;
		if (sample_size==0) {
			for (uint32_t i = 0; i < entry_count; i++) {
				uint32_t size = Box::read32(is);
				consumed += 4;
				sizes.push_back(size);
			}
		}
		if (largesize-consumed) 
			throw std::runtime_error("Wrong STSZ count");
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		Box::write32(os, sample_size);
		Box::write32(os, entry_count);
		if (sample_size==0) {
			for (uint32_t i = 0; i < entry_count; i++) {
				uint32_t size = sizes[i];
				Box::write32(os, size);
			}
		}
		
	}
};

/**
aligned(8) class CompositionOffsetBox extends FullBox(‘ctts’, version = 0, 0) {
	unsigned int(32) entry_count;
	int i;
	for (i=0; i < entry_count; i++) {
		unsigned int(32) sample_count;
		unsigned int(32) sample_offset;
	}
}
*/
class Ctts : public FullBox {

public:
	struct CttsSample {
		uint32_t sample_count;
		uint32_t sample_offset;
	};


	uint32_t entry_count;
	std::vector<CttsSample> samples;

	Ctts () : FullBox("ctts") {
		name = "CompositionOffset";
		entry_count = 0;
		increaseSize(4);
	}

	void addCttsSample(uint32_t sample_count, uint32_t sample_offset) {
		entry_count++;
		CttsSample sample;
		sample.sample_count = sample_count;
		sample.sample_offset = sample_offset;
		samples.push_back(sample);
		increaseSize(8);
	}


	virtual void read(large_istream& is) {
		FullBox::read(is);

		entry_count = Box::read32(is);
		consumed += 4;
		CttsSample sample;
		for (uint32_t i = 0; i < entry_count; i++) {
			sample.sample_count = Box::read32(is);
			sample.sample_offset = Box::read32(is);
			consumed += 8;
			samples.push_back(sample);
		}
		if (largesize-consumed) 
			throw std::runtime_error("Wrong CTTS count");
	}

	void write(large_ostream& os) {
		FullBox::write(os);
		Box::write32(os, entry_count);
		CttsSample sample;
		for (uint32_t i = 0; i < entry_count; i++) {
			sample = samples[i];
			Box::write32(os, sample.sample_count);
			Box::write32(os, sample.sample_offset);
		}
		
	}
};




/**
class AMF0() extends SampleEntry (‘amf’) {
}
*/
class Amf0 : public SampleEntry {
public:
	Amf0 () : SampleEntry("amf0") {
	}

	virtual void read(large_istream& is) { 
		SampleEntry::read(is);
	}

	void write(large_ostream& os) {
		SampleEntry::write(os);
	}
};

#endif //__MP4SAMPLEBOXES_H__

