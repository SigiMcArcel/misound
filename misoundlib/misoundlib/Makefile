CXX = g++
RM = rm

DESTDIR=
LIBDIR=/usr/lib
INCLUDEDIR=/usr/include


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
	

install: $(LIBRARYOUT)
        mkdir -p $(DESTDIR)/$(LIBDIR)
		mkdir -p $(DESTDIR)/$(INCLUDEDIR)
        cp cWave.h $(DESTDIR)/$(INCLUDEDIR)
		cp cAlsa.h $(DESTDIR)/$(INCLUDEDIR)
		cp cAudio.h $(DESTDIR)/$(INCLUDEDIR)
		cp $(LIBRARYOUT) $(DESTDIR)/$(LIBDIR)