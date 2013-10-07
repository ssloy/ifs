SYSCONF_LINK = g++
CPPFLAGS     =
CPPFLAGS     =
LDFLAGS      = -lglut -lm -lGL
LIBS         =

DESTDIR = ./
TARGET  = bcifs

OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: $(DESTDIR)$(TARGET)

$(DESTDIR)$(TARGET): $(OBJECTS)
	$(SYSCONF_LINK) -Wall $(LDFLAGS) -o $(DESTDIR)$(TARGET) $(OBJECTS) $(LIBS)

$(OBJECTS): %.o: %.cpp
	$(SYSCONF_LINK) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@

clean:
	-rm -f *.rpo
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
#	-rm *.pgm
#	-rm *.tga


