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
#include <ostream>
#include <sstream>

#include "Mp4FileImpl.h"
#include "Mp4Boxes.h"
#include "Mp4BoxesSeg.h"
#include "MediaInformationBoxes.h"
#include "Mp4SampleBoxes.h"
#include "Mp4SampleSourceImpl.h"
#include "Mp4IStreamSource.h"

//#include "PassThroughTagProcessor_C.h"
//int ProcessPassThroughTag(pTag, len, changelevel == 1) > 0 success;

Mp4FileImpl::Mp4FileImpl() : timescale (1000) {
}

Mp4FileImpl::~Mp4FileImpl() {
	std::list<Box*>::const_iterator it = boxes.begin();
	while(it != boxes.end()) {
		delete *it;
		it++;
	}
}

Box* Mp4FileImpl::factory(std::string type) {
	Box* box;
	
	if (type.compare("ftyp")==0) {
		box = new Ftyp();
	} else if (type.compare("moov")==0) {
		box = new Moov();
	} else if (type.compare("trak")==0) {
		box = new Trak();
	} else if (type.compare("minf")==0) {
		box = new Minf();
	} else if (type.compare("mdia")==0) {
		box = new Mdia();
	} else if (type.compare("stbl")==0) {
		box = new Stbl();
	} else if (type.compare("stco")==0) {
		box = new Stco();
	} else if (type.compare("co64")==0) {
		box = new Co64();
	} else if (type.compare("mdat")==0) {
		box = new Mdat();
	} else if (type.compare("tkhd")==0) {
		box = new Tkhd();
	} else if (type.compare("vmhd")==0) {
		box = new Vmhd();
	} else if (type.compare("smhd")==0) {
		box = new Smhd();
	} else if (type.compare("nmhd")==0) {
		box = new Nmhd();
	} else if (type.compare("dinf")==0) {
		box = new Dinf();
	} else if (type.compare("dref")==0) {
		box = new Dref();
	} else if (type.compare("url ")==0) {
		box = new Url_();
	} else if (type.compare("stsd")==0) {
		box = new Stsd();
	} else if (type.compare("mett")==0) {
		box = new Mett();
	} else if (type.compare("amf0")==0) {
		box = new Amf0();
	} else if (type.compare("btrt")==0) {
		box = new Btrt();
	} else if (type.compare("stts")==0) {
		box = new Stts();
	} else if (type.compare("stsc")==0) {
		box = new Stsc();
	} else if (type.compare("stsz")==0) {
		box = new Stsz();
	} else if (type.compare("ctts")==0) {
		box = new Ctts();
	} else if (type.compare("mdhd")==0) {
		box = new Mdhd();
	} else if (type.compare("hdlr")==0) {
		box = new Hdlr();
	} else if (type.compare("mvhd")==0) {
		box = new Mvhd();
	} else if (type.compare("mvex")==0) {
		box = new Mvex();
	} else if (type.compare("mehd")==0) {
		box = new Mehd();
	} else if (type.compare("sidx")==0) {
		box = new Sidx();



	} else if (type.compare("free")==0) {
		box = new UnkownBox(type);
	} else if (type.compare("udta")==0) {
		box = new UnkownBox(type);
	} else {
		box = new UnkownBox(type);
	}

	return box;
}

Box* Mp4FileImpl::readBox(large_istream& is) {
	
	uint64_t pos = is.tellg();

	// read box header
	uint32_t _size = Box::read32(is);
	uint64_t _largesize = _size;
	char type[5] = {0,0,0,0,0};
	is.read((char*)type, 4);

	int _consumed = 8; // size + type

	if (is.eof()) return 0;

	if (_size == 1) {
		_largesize = Box::read64(is);
		_consumed += 8;
	} else if (_size == 0)
		throw std::runtime_error ("Box not supported");

	// create box based on header

	Box* box = factory(type);
	box->sizexx = _size;
	box->largesize = _largesize;
	box->consumed = _consumed;
	box->read(is);	
	return box;
}

void Mp4FileImpl::writeBox(large_ostream& os, Box& box) {

	box.write(os);

	if (box.children) 
		Mp4FileImpl::writeBox(os, *box.children);
	
	if (box.siblings)
		Mp4FileImpl::writeBox(os, *box.siblings);
	
}

void Mp4FileImpl::buildChunkTable() {

	// builds chunk table from mp4 data
	// the objective is to preserve the same chunk distribution as the
	// source mp4 file, so no other table has to be modified from the 
	// original source file but the stco/co64.

	int trackNumber = 0;


	for (std::size_t t = 0; t < tracks.size(); t++) {
		
		Track& track = tracks[t];
		Trak* trak = track.trak;

		if (!trak->children)
			throw std::runtime_error("Bad track, no children nodes");

		Mdhd* mdhd = dynamic_cast<Mdhd*>(findAtom(trak->children, "mdhd"));
		if (mdhd == 0)
			throw std::runtime_error("track does not contain media header");

		std::string xx = Box::isoLanguage(mdhd->language);
		track.duration = mdhd->duration;
		track.timescale = mdhd->timescale;
		
		Mp4Co* mp4co = dynamic_cast<Mp4Co*>(findAtom(trak->children, "stco"));
		if (mp4co==0)
			   mp4co = dynamic_cast<Mp4Co*>(findAtom(trak->children, "co64"));

		Stsc* stsc = dynamic_cast<Stsc*>(findAtom(trak->children, "stsc"));
		Stts* stts = dynamic_cast<Stts*>(findAtom(trak->children, "stts"));
		Stsz* stsz = dynamic_cast<Stsz*>(findAtom(trak->children, "stsz"));

		if ((mp4co == 0) || (stsc == 0) || (stts == 0) || (stsz == 0))
			throw std::runtime_error("track does not contain data offsets");


		track.mp4co = mp4co;

		uint32_t laststsc = 0;
		uint32_t laststts = 0;
		uint32_t sampleCount = 0;
		uint32_t lastSttsSampleCount = 0;
		uint32_t sampleDelta = 0;
		double chunk_time = 0;

		// walk the stco / co64 table
		for (uint32_t i = 0; i < mp4co->getEntryCount(); i++)
		{
			// find sample chunk description for this chunk
			if ((laststsc + 1) < stsc->samples.size()) {
				// if the table is not at the end, peek into it.
				if ((i+1) == stsc->samples[laststsc+1].first_chunk) {
					laststsc ++;
				}
			}

			uint32_t sampleCount1 = sampleCount;
			sampleCount += stsc->samples[laststsc].samples_per_chunk;

			// find chunk size by adding all sizes for this chunk
			uint32_t chunkSize = 0;
			mp4assert(sampleCount <= stsz->entry_count, "stsz entry count out of boundaries"); 
			for (uint32_t s = sampleCount1; s < sampleCount; s++) {
				chunkSize += stsz->getSize(s);
			}

			// find sample timing
			if (sampleCount > lastSttsSampleCount) {
				mp4assert(laststts < stts->entry_count, "stts entry count out of boundaries");
				lastSttsSampleCount += stts->samples[laststts].sample_count;
				sampleDelta = stts->samples[laststts].sample_delta;
				laststts ++;
			}

			chunk_time += sampleDelta * stsc->samples[laststsc].samples_per_chunk ;

			// construct a chunk information entry.
			ChunkIndexedItem item;
			item.chunkIndex = i;
			item.chunkOffset = mp4co->getOffset(i);
			item.sampleDelta = sampleDelta;
			item.chunkTime = chunk_time;
			item.sampleSize = chunkSize;
			item.sampleCount = stsc->samples[laststsc].samples_per_chunk;
			item.time = chunk_time/track.timescale;
			item.track = &track;
			track.chunks.push_back(item);
			
			std::stringstream ss;
			ss << " track: " << t << " first: " << stsc->samples[laststsc].first_chunk << " chunk: " << " " << i+1 << " time: " << item.time << " c=" << sampleCount1 << " d=" << sampleDelta;
			observer->processMessage(Mp4File::VERBOSE, ss.str().c_str());
		}
	}

	

}

void Mp4FileImpl::parse(Mp4Source& is) {

	try {
		if (observer == 0)
			throw std::runtime_error("Mp4 File Observer needs to be set");

		observer->processMessage(Mp4File::INFO, "Parsing Mp4 file");

		Mp4IStreamSource& source = dynamic_cast<Mp4IStreamSource&> (is);

		if (!&source) throw std::runtime_error("Bad Input Source");
		// read boxes until it finds the end of file or an error
		
		Box* box;
		while(box=Mp4FileImpl::readBox(source.getIStream())) {
			boxes.push_back(box);
		}

		// building internal structure for all tracks available.
		int trackNumber = 0;
		Trak* trak = dynamic_cast<Trak*>(findTrack(trackNumber++, MP4FILE_ANY));
	
		while (trak && trak->children) {
		
			Track track;
			track.currentItem = 0;
			track.source = &is;
			track.trak = trak;
			tracks.push_back(track);
			trak = dynamic_cast<Trak*>(findTrack(trackNumber++, MP4FILE_ANY));
		}
	} catch (std::runtime_error ex)
	{
		if (observer == 0) throw;
		observer->processException(ex.what());
	}
}

uint64_t Mp4FileImpl::computeChunkOffsets(uint64_t startOffset)
{
	// compute new chunk offsets for the file, given a start Offset
	// for the MDAT atom.
	
	observer->processMessage(Mp4File::DEBUG, "Computing Chunk Offsets");

	// initialize state
	for (std::size_t i = 0; i < tracks.size(); i++) {
		tracks[i].currentItem = 0;
	}

	int trackNo = 0;
	uint64_t offset = startOffset;

	// rebuild stco tables by re-interleaving existing chunks based on ther
	// calculated time.
	// This function will build a "sortedOutput" of chunks to be written to
	// file in sequential order.

	do {
		trackNo = -1;
		double time = 9999999999.0;

		// find candidate next chunk 
		for (std::size_t i =0; i < tracks.size(); i++) {
			std::size_t item = tracks[i].currentItem;
			
			if (item >= tracks[i].chunks.size())
				continue;

			double chunkTime = tracks[i].chunks[item].time;
			if (chunkTime < time) {
				trackNo = i;
				time = chunkTime;
			}
		}

		// if chunk found, recalculate chunk offsets.
		if (trackNo != -1) {

			//std::stringstream ss;
			//ss << "Output: track=" << trackNo << " time=" << time << " offset=" << offset;
			//observer->processMessage(Mp4File::VERBOSE, ss.str().c_str());

			// write chunk to output structure
			Mp4Co* mp4co = tracks[trackNo].mp4co;
			std::size_t item = tracks[trackNo].currentItem;
			mp4co->setOffset(offset, item);
			tracks[trackNo].chunks[item].chunkOffsetOut = offset;
			sortedOutput.push_back(tracks[trackNo].chunks[item]);

			offset += tracks[trackNo].chunks[item].sampleSize;
			tracks[trackNo].currentItem++;
		}
	} while (trackNo != -1);



	std::stringstream ss;
	ss << "Computed Last Offset = " << offset << " Size= " << offset - startOffset;
	observer->processMessage(Mp4File::DEBUG, ss.str().c_str());
	
	return offset;
}

uint64_t Mp4FileImpl::computeHeaderSize() {

	std::list<Box*>::const_iterator it = boxes.begin();
	uint64_t size = 0;
	while(it != boxes.end()) {
		if ((*it)->type.compare("mdat") != 0) size += (*it)->largesize;
		else break; // leave any box after mdat out of the offset
		it++;

	}
	
	std::stringstream ss;
	ss << "Computed Size= " << size;
	observer->processMessage(Mp4File::DEBUG, ss.str().c_str());
	
	return size;
}

void Mp4FileImpl::writeMdat(large_ostream& os, uint64_t bytes) {
	
	uint64_t size = 0;
	
	std::stringstream ss;
	ss << "Writting @" << os.tellp();
	observer->processMessage(Mp4File::DEBUG, ss.str().c_str());

	// restamp mdat size
	Mdat* mdat = dynamic_cast<Mdat*>(findMdat());
	mdat->setDataSize(bytes);

	// write box header
	writeBox(os, *mdat);

	
	// writting track data (mdat)
	
	uint32_t chunkDataSize = 64*1024;
	uint8_t* chunkData = new uint8_t[chunkDataSize];
	

	for (std::size_t i = 0; i < sortedOutput.size(); i++) {
		ChunkIndexedItem source = sortedOutput[i];
		if (source.sampleSize > chunkDataSize) {
			delete [] chunkData;
			chunkData = new uint8_t [source.sampleSize];
			chunkDataSize = source.sampleSize;
		}

		source.track->source->read(chunkData, source.chunkOffset, source.sampleSize);
		//std::size_t
		int64_t whereOut = os.tellp();
		int64_t ooo =  os.tellp();
		if (whereOut == -1) 
			throw std::runtime_error("Can not write to file");

		whereOut -= source.chunkOffsetOut;
		if (whereOut != 0) {
		mp4assert(whereOut == 0, "wrong offset");
		}
		os.write((char*)chunkData, source.sampleSize);
		
	}

	delete [] chunkData;

}

void Mp4FileImpl::write(const char* filename) {
	
	//std::ofstream ofs(filename, std::ios::binary);
	LargeOfstream ofs(filename);
	large_ostream& os = ofs;

	if (observer == 0)
		throw std::runtime_error("Mp4 File Observer needs to be set");

	if (os.bad())
		throw std::runtime_error("Bad Output Stream");


	observer->processMessage(Mp4File::INFO, "Writting Mp4 File to output stream");


	// compute chunk timming table
	buildChunkTable();

	// obtain size of all atoms in the header until mdat.
	uint64_t offset = computeHeaderSize();
	
	int mdatHeaderSize = 8;
	if (findMdat()->sizexx == 1)
		mdatHeaderSize = 16;

	offset += mdatHeaderSize; //MDAT header size

	// compute new chunk offsets
	uint64_t diff = computeChunkOffsets(offset) - offset;
	
	
	std::stringstream ss;
	ss << "Writting Data @" << offset << " ammount = " << diff;
	observer->processMessage(Mp4File::DEBUG, ss.str().c_str());


	// writes header boxes to disk.
	std::list<Box*>::const_iterator it = boxes.begin();
	
	while(it != boxes.end()) {

		std::stringstream ss;
		ss << "writting " <<  (*it)->type.c_str();
		observer->processMessage(Mp4File::DEBUG, ss.str().c_str());
		if ((*it)->type.compare("mdat") == 0) 
			writeMdat(os, diff);
		else 
			writeBox(os, *(*it));
		it++;
	}
	

	os.flush();
}

Box*  Mp4FileImpl::findMoov() {
	std::list<Box*>::const_iterator it = boxes.begin();
	while(it != boxes.end()) {
		Box* box = *it++;
		if (box->type.compare("moov") == 0) {
			return box;
		}
	}
	return 0;
}

Box*  Mp4FileImpl::findMdat() {
	std::list<Box*>::const_iterator it = boxes.begin();
	while(it != boxes.end()) {
		Box* box = *it++;
		if (box->type.compare("mdat") == 0) {
			return box;
		}
	}
	return 0;
}

Box*  Mp4FileImpl::findFtyp() {
	std::list<Box*>::const_iterator it = boxes.begin();
	while(it != boxes.end()) {
		Box* box = *it++;
		if (box->type.compare("ftyp") == 0) {
			return box;
		}
	}
	return 0;
}

Box* Mp4FileImpl::findTrack(int number, const TrackType& type) {
	Box* moov = findMoov();
	if (moov==0)
		throw std::runtime_error("Can not find moov");

	int ntracks = 0;
	Box* trak = moov;
	do {
		trak = findAtom(trak, "trak");
		if (trak && findAtom(trak, type) && (number == ntracks)) {
			return trak;	
		}
		if (trak) ntracks++;
		trak = trak?trak->siblings:0;
	} while (trak);
	return trak;
}

Box* Mp4FileImpl::findAtom(Box* anchor, std::string type) {
	Box* box = 0;
	if (anchor->type == type || type == Mp4FileImpl::MP4FILE_ANY) {
		return anchor;
	}

	if (anchor->children) {
		box = findAtom(anchor->children, type);
		if (box) return box;
	}

	if (anchor->siblings) {
		box = findAtom(anchor->siblings, type);
	}

	return box;
}

void Mp4FileImpl::mp4assert(bool condition, std::string msg) {
	if (!condition)
		throw std::runtime_error(msg.c_str());
}

double Mp4FileImpl::getDuration() {
	Mvhd* mvhd = dynamic_cast<Mvhd*>(findAtom(findMoov(), "mvhd"));
	double ret = (double)(mvhd->duration / mvhd->timescale);
	return ret;
}

Box* Mp4FileImpl::buildStsz(Mp4SampleSource& _samples) {
	Stsz* stsz = new Stsz();

	Mp4SampleSourceImpl& samples = dynamic_cast<Mp4SampleSourceImpl&>(_samples);

	Mp4SampleSourceImpl::SampleListIter it = samples.getSamples().begin();
	while(it != samples.getSamples().end()) {
		stsz->addStszSample((*it)->m_bufferLength);
		it++;
	}

	return stsz;
}

Box* Mp4FileImpl::buildStts(Mp4SampleSource& _samples) {
	Stts* stts = new Stts();
	Mp4SampleSourceImpl& samples = dynamic_cast<Mp4SampleSourceImpl&>(_samples);
	
	Mp4SampleSourceImpl::SampleListIter it = samples.getSamples().begin();
	
	double lasttime = 0;
	double delta;
	while(it != samples.getSamples().end()) {
		delta = (*it)->m_time - lasttime;
		stts->addSttsSample(1, (uint32_t)(delta*timescale));
		lasttime += delta;
		it++;
	}

	return stts;
}

Box* Mp4FileImpl::buildStsc() {
	Stsc* stsc = new Stsc();
	stsc->addStscSample(1,1,1);
	return stsc;

}

Box* Mp4FileImpl::buildCtts(Mp4SampleSource& _samples) {
	Ctts* ctts = new Ctts();
	ctts->addCttsSample(_samples.getNumberOfSamples(), timescale);
	return ctts;
}

Box* Mp4FileImpl::buildChunk(Mp4SampleSource& _samples, bool large) {
	Mp4Co* co;
	if (large) co = new Co64();
	else co = new Stco();
	
	Mp4SampleSourceImpl& samples = dynamic_cast<Mp4SampleSourceImpl&>(_samples);

	Mp4SampleSourceImpl::SampleListIter it = samples.getSamples().begin();

	int i = 0;
	while(it != samples.getSamples().end()) {
		co->addOffset(i++); 
		it++;
	}

	return dynamic_cast<Box*>(co);
}

Box* Mp4FileImpl::buildMetaDescriptor() {
	Stsd* stsd = new Stsd();
	Mett* mett = new Mett();
	mett->setContentEncoding("text/plain");
	mett->setMime("base64");
	stsd->addBox(mett);
	return stsd;
}

Box* Mp4FileImpl::buildAmf0Descriptor() {
	Stsd* stsd = new Stsd();
	Box* amf = new Amf0();
	stsd->addBox(amf);
	return stsd;
}

uint64_t Mp4FileImpl::getTime() {
	time_t t = std::time(0);
	return t+64*365*24*3600;
}

Box* Mp4FileImpl::buildTrackHeader(Mp4SampleSource& samples, int trackId) {
	Tkhd* tkhd = new Tkhd();

	tkhd->creation_time = getTime();
	tkhd->modification_time = getTime();
	tkhd->duration = (uint64_t)samples.getDuration()*timescale;
	tkhd->track_ID = trackId;

	return tkhd;
}

Box* Mp4FileImpl::buildMediaHeader(Mp4SampleSource& _samples) {
	Mdhd* mdhd = new Mdhd();
	Mp4SampleSourceImpl& samples = dynamic_cast<Mp4SampleSourceImpl&>(_samples);

	mdhd->creation_time = getTime();
	mdhd->modification_time = getTime();
	mdhd->duration = (uint64_t)samples.getSamples().back()->m_time*timescale;
	mdhd->timescale = timescale;
	mdhd->language = 0;
			
	return mdhd;
}

Box* Mp4FileImpl::buildHandler(std::string name = "Timed Metadata Handler", std::string type = "data") {
	Hdlr* hdlr = new Hdlr();
	hdlr->setHandlerType(type);
	hdlr->setHandlerName(name);
	return hdlr;
}

uint64_t Mp4FileImpl::insertTrack(Mp4Source& source, TrackType type) {

	if (observer == 0)
		throw std::runtime_error("Mp4 File Observer needs to be set");

	if (type == Mp4FileImpl::MP4FILE_AMF0) {
		Ftyp* ftyp = dynamic_cast<Ftyp*>( findFtyp());
		if (!ftyp)
			throw std::runtime_error("Bad file, No FTYP atom");
		ftyp->pushMajor("f4v ");
	}

	// count tracks
	int trackNumber = 0;
	Box* box;
	Box* lastTrack = 0;
	while (box = findTrack(trackNumber++, MP4FILE_ANY))
	{
		lastTrack = box;
	}

	std::stringstream ss;
	ss << "Inserting Track no = " << trackNumber;
	observer->processMessage(Mp4File::INFO, ss.str().c_str());

	if ((type == Mp4FileImpl::MP4FILE_META) || 
		(type == Mp4FileImpl::MP4FILE_AMF0)) {

		Mp4SampleSource& samples = dynamic_cast<Mp4SampleSource&>(source);
		if (samples.getNumberOfSamples() == 0)
			throw std::runtime_error("No source data to insert");

		// Build Metadata Track
		Trak* trak = new Trak();
		trak->add(buildTrackHeader(samples, tracks.size() + 1));

		Mdia* mdia = new Mdia();
	
		mdia->add(buildMediaHeader(samples));
	
		if (type == Mp4FileImpl::MP4FILE_AMF0)
			mdia->add(buildHandler("Timed Metadata Handler", "data"));
		else
			mdia->add(buildHandler("Nielsen Timed Medatata Handler", "meta"));
	
		Box* minf = new Minf();
		minf->add(new Nmhd());

		Box* dinf = new Dinf();
		Dref* dref = new Dref();
		Url_* url = new Url_();
	
		dref->addDataEntry(url);
		dinf->add(dref);
		minf->add(dinf);

		Box* stbl = new Stbl();

		if (type == Mp4FileImpl::MP4FILE_AMF0)
			stbl->add(buildAmf0Descriptor());
		else
			stbl->add(buildMetaDescriptor());
		
		stbl->add(buildStts(samples));
		stbl->add(buildStsc());
		//stbl->add(buildCtts(samples));
		stbl->add(buildStsz(samples));
		stbl->add(buildChunk(samples, true));

		minf->add(stbl);

		mdia->add(minf);
		trak->add(mdia);

		trak->siblings = lastTrack->siblings;
		lastTrack->siblings = trak;
		Box* moov = findMoov();
		moov->increaseSize(trak->largesize);

		std::stringstream ss;
		ss << "Inserting Track Data size = " << trak->largesize;
		observer->processMessage(Mp4File::DEBUG, ss.str().c_str());

		Track track;
		track.trak = trak;
		track.source = &source;
		tracks.push_back(track);

		return trak->largesize;
	} else {
		throw std::runtime_error ("Track type not supported");
	}

	return 0;
}

std::ostream& Mp4FileImpl::print (std::ostream& os) const {
	std::list<Box*>::const_iterator it = boxes.begin();
	while(it != boxes.end()) {
		os << *(*it) << std::endl;
		it++;
	}
	return os;
}

