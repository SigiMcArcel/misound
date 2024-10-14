DESTDIR=
prefix=/usr
includedir=$(prefix)/include
libdir=$(prefix)/lib

INCLUDES = -I. -I/.
CXXFLAGS = -fpic -Wall -Wl,--no-undefined
LD_FLAGS = -shared -lsndfile -lpthread -lasound

# Automatisch alle .cpp-Dateien im aktuellen Verzeichnis als SOURCES verwenden
SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)

# Bibliotheksname basierend auf dem Verzeichnisnamen erstellen
PROJECT_NAME := $(notdir $(CURDIR))
LIBRARYOUT = lib$(PROJECT_NAME).so

# Regel zum Kompilieren der .cpp-Dateien zu .o-Dateien
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $<

# Regel zum Erstellen der Bibliothek
$(LIBRARYOUT): $(OBJECTS)
	$(CXX) $(LD_FLAGS) $(OBJECTS) -o $(LIBRARYOUT)

# Regel zum Bereinigen der Build-Artefakte
clean:
	$(RM) $(LIBRARYOUT) *.o

# Regel zur Installation der Bibliothek und Header-Dateien
install: $(LIBRARYOUT)
	install -d $(DESTDIR)/$(libdir)
	install -D -m 0644 $(LIBRARYOUT) $(DESTDIR)/$(libdir)/$(LIBRARYOUT)
	install -d $(DESTDIR)/$(includedir)/mi/$(PROJECT_NAME)
	for header in *.h; do \
		install -D -m 0644 $$header $(DESTDIR)/$(includedir)/mi/$(PROJECT_NAME)/$$header; \
	done