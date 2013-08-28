SYSCONF_LINK = g++
CPPFLAGS     = -O3 -I/usr/local/coin3d/include -I/usr/include/qt4 -I/usr/include/qt4/Qt
LDFLAGS      = -L/usr/share/qt4/lib -L/usr/lib
LIBS         =  -lCoin -lSoQt

DESTDIR = ./
TARGET  = affine

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
