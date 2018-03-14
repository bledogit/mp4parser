/**
* 
*  author	Jose Mortensen
*  brief	This file implements all sources, and factories. 
*   	
*/

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <stdint.h>
#include "Mp4File.h"
#include "Mp4SampleSourceImpl.h"
#include "Mp4IStreamSource.h"

/**
	FACTORIES
*/

std::vector<Mp4Source*> __Mp4Source_createdSources;

Mp4SampleSource& Mp4Source::createMp4SampleSource() {
	Mp4SampleSource* source = new Mp4SampleSourceImpl();
	//__Mp4Source_createdSources.push_back(source);
	return *source;
}


Mp4Source& Mp4Source::createFileSource(const char* filename) {

	Mp4Source* source = new Mp4IStreamSource(*new LargeIfstream(filename));
	//__Mp4Source_createdSources.push_back(source);
	return *source;
}

void Mp4Source::destroy(Mp4Source& source) {
	delete &source;
}



/**
	Mp4IStreamSource
*/

Mp4IStreamSource::Mp4IStreamSource(large_istream& source) : is(source) {
}

Mp4IStreamSource::~Mp4IStreamSource() {
	delete &is;
}

uint64_t Mp4IStreamSource::read(uint8_t* buffer, uint64_t offset, uint64_t size) {
	is.seekg(offset, std::ios_base::beg);
	if (is.fail()) {
		is.clear();
		is.seekg(offset, std::ios_base::beg);
	}

	if (is.fail()) throw std::runtime_error("Error Seeking input");
	is.read((char*)buffer, size);
	if (is.fail()) throw std::runtime_error("Error Reading input");

	return size;
}



/**
	Mp4SampleSourceImpl
*/


Mp4SampleSourceImpl::Mp4SampleSourceImpl() {
}

Mp4SampleSourceImpl::~Mp4SampleSourceImpl() {
	SampleListIter iter = samples.begin();
	while (iter != samples.end()) {
		delete (*iter);
		iter++;
	}
}

Mp4SampleSourceImpl::SampleList& Mp4SampleSourceImpl::getSamples()  { 
	return samples;
}

/// Reads a (size) of data from an offset to a buffer
uint64_t Mp4SampleSourceImpl::read(uint8_t* buffer, uint64_t offset, uint64_t size) {

	
	if (size < (uint64_t)samples[(int)offset]->m_bufferLength) 
		throw std::runtime_error("buffer too small");

	memcpy(buffer, samples[(int)offset]->m_buffer, samples[(int)offset]->m_bufferLength);

	return samples[(int)offset]->m_bufferLength;
}

double Mp4SampleSourceImpl::getDuration() {
	if (samples.empty())
		return 0.0;
	else 
		return samples.back()->m_time;
}

void Mp4SampleSourceImpl::push(uint8_t* buffer, int size, double time) {
	Mp4Sample* sample = new Mp4Sample(buffer, size, time);
	samples.push_back(sample);
}

int Mp4SampleSourceImpl::getNumberOfSamples() {
	return samples.size();
}

/**
	SAMPLE IMPLEMENTATION
*/
Mp4Sample::Mp4Sample ( const uint8_t* buffer, int length, double time) {
		m_buffer = new uint8_t[length];
		memcpy(m_buffer, buffer, length);
		m_bufferLength = length;
		m_time = time;
		//std::cout << "Mp4Sample " << this << " built" << std::endl;
	}


Mp4Sample::~Mp4Sample () {
		//std::cout << "Mp4Sample " << this << " delete, attached=" << m_detached << std::endl;
		delete [] m_buffer;
}
