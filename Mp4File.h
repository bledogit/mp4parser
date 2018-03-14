/**
 * 
 *  author	Jose Mortensen
 *  brief	Iso Mp4 Box parser Interface
 * 	
 */

#ifndef __MP4FILE_H__
#define __MP4FILE_H__

#if defined (NS_DYNAMIC)
	#if defined (NS_MP4FILE_EXPORT)
	#define NS_MP4FILE_API	__declspec(dllexport)
	#else
	#define NS_MP4FILE_API	__declspec(dllimport)
	#endif
#else
	#define NS_MP4FILE_API	
#endif

#include <stdint.h>

/*
/// stdint unsigned 8bits
typedef unsigned char       uint8_t;
/// stdint unsigned 16 btis
typedef unsigned short      uint16_t;
/// stdint unsigned 16 btis
typedef unsigned int        uint32_t;
/// stdint unsigned 16 btis
typedef unsigned __int64    uint64_t;
*/
#include "Mp4Sample.h"
#include "Mp4Source.h"


class Mp4FileImpl;
class Mp4FileObserver;

/**
	Mp4File is a module to manipulate ISO 14496-12 compatible files.
	*/
class NS_MP4FILE_API Mp4File {

public:

	/// Track Type
	typedef char TrackType[];
	/// Track type that contains video frames
	static const TrackType MP4FILE_VIDEO;
	/// Track type that contains audio frames
	static const TrackType MP4FILE_SOUND;
	/// Track type that contains timed metadata frames
	static const TrackType MP4FILE_META;
	/// Track type that contains AMF0 frames
	static const TrackType MP4FILE_AMF0;
	/// Any track
	static const TrackType MP4FILE_ANY;
	
	/// Log level
	typedef enum {
		INFO = 0,
		DEBUG = 1,
		VERBOSE = 2
	} LogLevel;


	/// observer that receives log and exception mesages from this module
	Mp4FileObserver* observer;

public:

	/// Yes, the constructor
	Mp4File() : observer(0)  {};
	/// Yes, the destructor
	virtual ~Mp4File() {};
	/** Parses ISO file from input stream.  Must be able to seek.
		@param is  Input source
	*/
	virtual void parse(Mp4Source& is) = 0;
	/** Insert / creates track from samples.
		@param source Source of information. Must be a Mp4SampleSource
		@param type   Track type to use on the insert
		return track size in bytes
		*/
	virtual uint64_t insertTrack(Mp4Source& source, TrackType type) = 0;
	/** writes current content to output file
		@param outputFile output file
		*/
	virtual void write(const char* outputFile) = 0;
	/** prints file tree
		@param os output stream
		@return given output stream
		*/
	virtual std::ostream& print(std::ostream& os) const = 0;
	/** gets duration of parsed file
		@return file duration in seconds
		*/
	virtual double getDuration() = 0;
	/** sets observer
		@param observer Sets parser observer
		*/
	virtual void setObserver(Mp4FileObserver* observer) {this->observer = observer;};
	
	/** Mp4File factory
		return An Mp4File impemented instance
		*/
	static Mp4File& create();
	/** destroy created file parser
		@param mp4file instance
		*/
	static void destroy(Mp4File& mp4file);
	
	/** check if a file is an mp4 file
		@param filename File Name
		*/
	static bool isMp4File(const char* filename);


	/** To use with std::ostream (cout) */
	friend std::ostream& operator<<(std::ostream& os, const Mp4File& file) {
		file.print(os);
		return os;
	}
};

/**
	Mp4File Parser observer
	*/
class NS_MP4FILE_API Mp4FileObserver {
public:


	/**
	Processes information messages
	@param level Log Level (used for filtering message kinds)
	@param message  Message
	*/
	virtual void processMessage(Mp4File::LogLevel level, const char* message) {}; 

	/**
	Processes exceptions.
	Defaults to throw a std::exception if not overriden.
	@param message  Error Message. 
	*/
	virtual void processException(const char* message) {
		std::string errorMessage = std::string(message);
		throw std::runtime_error(errorMessage);
	}
};
#endif //__MP4FILE_H__
