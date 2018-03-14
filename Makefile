CC=clang++
CFLAGS=-c -Wall 
LDFLAGS=
SOURCES=LargeFileSources.o Mp4File.o Mp4Sources.o Mp4Box.o Mp4FileImpl.o main.o
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=mp4parser

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
