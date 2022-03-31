CXX = g++
RM = rm

CXX_FLAGS = -fpic -Wall
LD_FLAGS = -shared -lsndfile -lpthread -lasound

SOURCES=cAlsa.cpp cAudio.cpp cWave.cpp
OBJECTS=$(SOURCES:.cpp=.o)

LIBRARYOUT=libmisoundlib.so


.cpp.o:
	$(CXX) $(CXX_FLAGS) -c $<
$(LIBRARYOUT):$(OBJECTS)
	$(CXX) $(LD_FLAGS) $(OBJECTS) -o $(LIBRARYOUT)
clean:
	$(RM) $(LIBRARYOUT) *.o
	

