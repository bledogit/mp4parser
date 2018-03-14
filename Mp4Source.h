/**
 * 
 *  author	Jose Mortensen
 *  brief	Source 
 * 	
 */

#ifndef __MP4SOURCE_H__
#define __MP4SOURCE_H__


class Mp4SampleSource;

/**
	Base source class.
	This class interfaces the possible data sources for the Mp4File module.
	*/
class NS_MP4FILE_API Mp4Source {
public:
	/** 
		Reads a (size) of data from an offset to a buffer
		@param buffer  Buffer to return the read data
		@param offset  data offset from the beginning of the source
		@param size    amount of information to read
		@return bytes read.
	*/
	virtual uint64_t read(uint8_t* buffer, uint64_t offset, uint64_t size) = 0;

	/**
		Sample Source Factory
		@return creates and returns an Mp4SampleSource. Use destroy to release memory allocated by source.
	*/
	static Mp4SampleSource& createMp4SampleSource();
	/**
		File Source Factory. 
		@param filename File name associated to this source.
		@return creates and returns an Mp4Source. Use destroy to release memory allocated by source.
	*/
	static Mp4Source& createFileSource(const char* filename);

	/** 
		Destroy source
		@param source Source to destroy
	*/
	static void destroy(Mp4Source& source);
};

/**
	Class to source an array of frame buffers
*/
class NS_MP4FILE_API Mp4SampleSource : public Mp4Source {

public:

	/**
		Default constructor
		*/
	Mp4SampleSource(){};

	/** 
		Adds frame of information to this source. The supplied data is copied internally, and
		released when the source is destroyed.
		@param buffer byte buffer that contains the information
		@param size   size of the byte buffer
		@param time   presentation time of this buffer.
		*/
	virtual void push(uint8_t* buffer, int size, double time) = 0;

	/**
		Reads a frame from given offset
		@param buffer byte buffer that contains the information
		@param offset buffer offset, or frame number
		@param size   size of the byte buffer
		@return actual size of the returned buffer
		*/
	virtual uint64_t read(uint8_t* buffer, uint64_t offset, uint64_t size) = 0;
	
	/**
		Returns frame buffer duration
		@return the presentation time of the last frame in the buffer.
		*/
	virtual double getDuration() = 0;

	/**
		Get the number of samples in the source
		@return number of samples
		*/
	virtual int getNumberOfSamples() = 0;
};



#endif