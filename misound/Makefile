

DESTDIR=
LIBDIR=/usr/lib
INCLUDEDIR=/usr/include

INCLUDES = -I. -I/.
CXX_FLAGS = -fpic -Wall
LD_FLAGS = -shared -lsndfile -lpthread -lasound

SOURCES=Alsa.cpp Audio.cpp Wave.cpp
OBJECTS=$(SOURCES:.cpp=.o)

LIBRARYOUT=libmisound.so


.cpp.o:
	$(CXX) $(CXX_FLAGS) $(INCLUDES) -c $<
$(LIBRARYOUT):$(OBJECTS)
	$(CXX) $(LD_FLAGS) $(OBJECTS) -o $(LIBRARYOUT)
clean:
	$(RM) $(LIBRARYOUT) *.o
	

install: $(LIBRARYOUT)
	mkdir -p $(DESTDIR)/$(LIBDIR)
	mkdir -p $(DESTDIR)/$(INCLUDEDIR)
	cp Wave.h $(DESTDIR)/$(INCLUDEDIR)
	cp Alsa.h $(DESTDIR)/$(INCLUDEDIR)
	cp Audio.h $(DESTDIR)/$(INCLUDEDIR)
	cp $(LIBRARYOUT) $(DESTDIR)/$(LIBDIR)