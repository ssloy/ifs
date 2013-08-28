SYSCONF_LINK = g++
CPPFLAGS     = -O3 
CPPFLAGS     = 
LDFLAGS      = 
LIBS         = 

DESTDIR = ./
TARGET  = postscript

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
