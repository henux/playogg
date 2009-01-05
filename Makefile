# Makefile

PREFIX=/usr/local
BINDIR=$(PREFIX)/bin

LIBS=`sdl-config --libs` `pkg-config --libs vorbisfile`
CFLAGS=`sdl-config --cflags` `pkg-config --cflags vorbisfile`

all: playogg

playogg: playogg.c
	gcc -o playogg playogg.c $(LIBS) $(CFLAGS)

.PHONY: all clean install uninstall

clean:
	rm -f playogg

install:
	install -D -m755 playogg $(BINDIR)/playogg

uninstall:
	rm -f $(BINDIR)/playogg
