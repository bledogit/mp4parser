/**
 * 
 *  author	Jose Mortensen
 *  brief	Generic sample buffer
 * 	
 */


#ifndef __DATASAMPLE_H__
#define __DATASAMPLE_H__

#include <iostream>

/**
	Interface for data Samples used on the Mp4File module
	*/
class NS_MP4FILE_API Mp4Sample {

public:
	/// data buffer
	uint8_t*	m_buffer;
	/// data buffer length
	size_t		m_bufferLength;
	/// presentation time (seconds)
	double		m_time;

public:
	/**
		Constructor
		@param buffer  Data Buffer. Data buffer is cloned internally.
		@param length  Data Buffer Length
		@param time    Presentation time in seconds
		*/
	Mp4Sample ( const uint8_t* buffer, int length, double time);

	/**
		Destructor, releases alocated memory
		*/
	~Mp4Sample ();

	/// std ostream utility
	friend std::ostream& operator<<(std::ostream& os, const Mp4Sample& info) {
		os	<< "time=" << info.m_time
			<< "buffer=" << (int*)info.m_buffer;
		return os;
	}
};

#endif // __DATASAMPLE_H__
