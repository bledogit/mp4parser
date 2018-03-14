/**
 * 
 *  author	Jose Mortensen
 *  brief	Source 
 * 	
 */

#ifndef __MP4ISTREAMSOURCE__
#define __MP4ISTREAMSOURCE__

#include "Mp4Source.h"

#define _STL_LARGE_FILE_WORKAROUND_
#ifdef _STL_LARGE_FILE_WORKAROUND_
	#include "LargeFileSources.h"

	typedef LargeIfstream large_istream;
	typedef LargeOfstream large_ostream;
#else
	typedef std::istream large_istream;
	typedef std::ostream large_ostream;
#endif

	/**
	Class to read data from an istream
	*/
class Mp4IStreamSource : public Mp4Source {



	large_istream& is;

public:
	/** generic constructor
		@param stream  externally created stream
	*/
	Mp4IStreamSource(large_istream& stream);

	/**
		destructor
		destroys given stream
	*/
	~Mp4IStreamSource();

	/**
		gets a reference to stream
		*/
	large_istream& getIStream() {return is;}

	/// Reads a (size) of data from a file offset to a buffer
	uint64_t read(uint8_t* buffer, uint64_t offset, uint64_t size);
};


#endif
