/**
 * 
 *  author	Jose Mortensen
 *  brief	Iso Mp4 Box parser Implementation
 * 	
 */

#ifndef __MP4FILEIMPL_H__
#define __MP4FILEIMPL_H__


#include <stdint.h>
#include <list>
#include <vector>
#include "Mp4File.h"
#include "Mp4IStreamSource.h"


class Box;
class Trak;
class Mp4Co;



class Mp4FileImpl : public Mp4File {

	struct Track;

	struct ChunkIndexedItem {
		uint32_t chunkIndex;
		uint64_t chunkTime;
		uint64_t chunkOffset;
		uint32_t sampleDelta;
		uint32_t sampleCount;
		uint32_t sampleSize;

		uint64_t chunkOffsetOut;
		Track* track;
		double time;

	};

	struct Track {
		Trak* trak;
		Mp4Co* mp4co;
		uint64_t duration;
		uint64_t timescale;
		std::vector<ChunkIndexedItem> chunks;
		Mp4Source* source;
		int currentItem;
	};

	/// contains a list of chunks to be saved to disk.
	std::vector<ChunkIndexedItem> sortedOutput;
	/// ISO file top level boxes
	std::list<Box*> boxes;
	/// ISO file tracks
	std::vector<Track> tracks;
	/// Track timescale
	int timescale;

private:
	/// Finds MOOV atom
	Box* findMoov();
	/// Finds MDAT atom
	Box* findMdat();
	/// Finds FTYP atom
	Box* findFtyp();
	/// Finds track number of the requested type
	Box* findTrack(int number, const TrackType& type);
	/**
		finds next atom of the requested type.  It returns achor is it 
		matches the given type
		*/
	Box* findAtom(Box* anchor, std::string type);
	/// Builds, or Rebuilds chunk offsets
	void buildChunkTable();
	/// Creates a new sizes table
	Box* buildStsz(Mp4SampleSource& samples);
	/// Create a new Time to Sample table
	Box* buildStts(Mp4SampleSource& samples);
	/// Create a new Composition time table
	Box* buildCtts(Mp4SampleSource& _samples);
	/// Creates a new sample to chunk table
	Box* buildStsc();
	/// Create a new chunk offset table. if large is true, returns co64, otherwise stco
	Box* buildChunk(Mp4SampleSource& samples, bool large);
	/// Builds the metadata descriptor
	Box* buildMetaDescriptor();
	/// Builds amf0 descriptor
	Box* buildAmf0Descriptor();
	/// Build trak handler
	Box* buildHandler(std::string name, std::string type);
	/// Gets current time from 1904 per Iso recomendation
	uint64_t getTime();

	/// My very own assert function.  throws std::exception if fails.
	void mp4assert(bool condition, std::string msg);
	/// Builds track header
	Box* buildTrackHeader(Mp4SampleSource& samples, int trackId);
	/// Builds Media Header
	Box* buildMediaHeader(Mp4SampleSource& samples);
	/// Compute chunk offset from associated stream
	uint64_t computeChunkOffsets(uint64_t offset);
	/// compute Header Sizes.
	uint64_t computeHeaderSize();
	/// write mdat body
	void writeMdat(large_ostream& os, uint64_t bytes);

public:
	/// creates a Iso Box of the given type
	static Box* factory(std::string type);
	/// writes a given iso box and its children to the given output stream
	static void writeBox(large_ostream& os, Box& box);
	/// reads the next iso box found on the stream, including children.
	static Box* readBox(large_istream& is);
	
	/// Yes, the constructor
	Mp4FileImpl();
	/// Yes, the destructor
	~Mp4FileImpl();
	/// Parses ISO file from input stream.  Must be able to seek.
	void parse(Mp4Source& is);
	/// Insert / creates track from samples.
	uint64_t insertTrack(Mp4Source& source, TrackType type);
	/// writes current content to output stream
	void write(const char* filename);
	/// gets duration of parsed file
	double getDuration();
	/// std::out
	std::ostream& print (std::ostream& os) const;



};

#endif //__MP4FILE_H__
