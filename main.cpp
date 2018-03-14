#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
#include <algorithm>
#include "Mp4File.h"

class  MyObserver : public Mp4FileObserver {
public:


	/**
	Processes information messages
	@param level Log Level (used for filtering message kinds)
	@param message  Message
	*/
	virtual void processMessage(Mp4File::LogLevel level, const char* message) {
		std::cout << message << std::endl;
	}; 

};

int main (int argc, char** argv) {

	MyObserver obs;

	Mp4File& parser = Mp4File::create();
	parser.setObserver(&obs);
				

	if (argc < 1)	{
		std::cout << "mp4parser file.mp4" << std::endl;
		return 1;
	}		

	if (!Mp4File::isMp4File(argv[1]) ) {
		std::cout << "Not an Mp4 File" << std::endl;
	//	return 1;
	}
	
	std::cout << "file: " << argv[1] << std::endl;

	Mp4Source& mp4file = Mp4Source::createFileSource(argv[1]);
	parser.parse(mp4file);

	std::cout << parser << std::endl;

	Mp4File::destroy(parser);
	Mp4Source::destroy(mp4file);

	return 0;

}