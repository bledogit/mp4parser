/**
 * 
 *  author	Jose Mortensen
 *  brief	Source 
 * 	
 */

#include "Mp4Source.h"
#include <vector>

/**
	Class to source an array of prefilled buffers
*/
class Mp4SampleSourceImpl : public Mp4SampleSource {

public:

	typedef std::vector<Mp4Sample*> SampleList;

	typedef std::vector<Mp4Sample*>::const_iterator SampleListIter;
	
private:
	SampleList samples;

public:

	Mp4SampleSourceImpl();
	
	~Mp4SampleSourceImpl();

	SampleList& getSamples();
	
	void push(uint8_t* buffer, int size, double time);

	uint64_t read(uint8_t* buffer, uint64_t offset, uint64_t size);
	
	double getDuration();

	int getNumberOfSamples();
};