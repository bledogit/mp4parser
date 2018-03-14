/**
 * 
 *  author	Jose Mortensen
 *  brief	Iso Mp4 Box parser
 * 	
 */

#include "stdafx.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <algorithm>

#include "Mp4File.h"
#include "Mp4FileImpl.h"
#include "Mp4Boxes.h"
#include "MediaInformationBoxes.h"
#include "Mp4SampleBoxes.h"


const Mp4File::TrackType Mp4File::MP4FILE_VIDEO = "vmhd";
const Mp4File::TrackType Mp4File::MP4FILE_SOUND = "smhd";
const Mp4File::TrackType Mp4File::MP4FILE_META = "mdhd";
const Mp4File::TrackType Mp4File::MP4FILE_AMF0 = "amf0";
const Mp4File::TrackType Mp4File::MP4FILE_ANY = "";


Mp4File& Mp4File::create() {
	Mp4File* file = new Mp4FileImpl();
	return *file;
}

void Mp4File::destroy(Mp4File& mp4file)
{
	delete &mp4file;
}

bool Mp4File::isMp4File(const char* filename) 
{
	bool ret = false;
	LargeIfstream file(filename);

	if (file.bad())
		throw std::runtime_error("Bad File");


	while (true) {
		// read box header
		uint32_t _size = Box::read32(file);
		uint64_t _largesize = _size;
		int _consumed = 8; // size + type

		if (file.eof()) return false;

		if (_size == 1) {
			_largesize = Box::read64(file);
			_consumed += 8;
		} else if (_size == 0)
			throw std::runtime_error ("Box not supported");

		// create box based on header
		char type[5] = {0,0,0,0,0};
		file.read((char*)type, 4);
		if (file.eof()) return false;
		if (strncmp(type, "ftyp", 4) == 0) {
			Ftyp ftyp;
			ftyp.sizexx = _size;
			ftyp.largesize = _largesize;
			ftyp.consumed = _consumed;
			ftyp.read(file);
			//std::cout << "Found Box: " << ftyp << std::endl;
			return true;
		} else {
			// skip box;
			//std::cout << "Found Box: " << type << std::endl;
			file.seekg(_largesize-_consumed, std::ios_base::cur);
		}
	}
	return false;

}