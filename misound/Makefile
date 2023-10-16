DESTDIR=
refix=/usr
includedir=$(prefix)/include
libdir=$(prefix)/lib

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
	install -d $(DESTDIR)/${libdir}
	install -D -m 0644 libmisound.so $(DESTDIR)/${libdir}/$(LIBRARYOUT)
	install -d $(DESTDIR)/${includedir}
	install -d $(DESTDIR)/${includedir}/mi
	install -d $(DESTDIR)/${includedir}/mi/misound
	install -D -m 0644 Wave.h $(DESTDIR)/${includedir}/mi/misound/Wave.h
	install -D -m 0644 Alsa.h $(DESTDIR)/${includedir}/mi/misound/Alsa.h
	install -D -m 0644 Audio.h $(DESTDIR)/${includedir}/mi/misound/Audio.h
