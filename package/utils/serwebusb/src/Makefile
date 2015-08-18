#
# File: Linux serwebproxy makefile
#
# (C)2011-2012 Lari Temmes, manually modified not tested.
# (C)1999 Stefano Busti
#

VERSION = `cat VERSION`

SRCS = \
  main.c sio.c sock.c thread.c vlist.c cfglib.c config.c string.c \
  pipe.c error.c websocket.c http.c

OBJS = \
  main.o sio.o sock.o thread.o vlist.o cfglib.o config.o string.o \
  pipe.o error.o websocket.o http.o

CC = gcc

ifdef DEBUG
CFLAGS = -Wall -g -D__UNIX__ -DDEBUG
else
CFLAGS = -Wall -O2 -fomit-frame-pointer -D__UNIX__
endif

ifdef USE_EF
LIBS= -lpthread -lefence -lssl -lcrypto
else
LIBS= -lpthread -lssl -lcrypto
endif

# Build the program

serwebusb: $(SRCS) $(OBJS)
	$(CC) $(CFLAGS)  -o serwebusb $(OBJS) $(LDFLAGS) $(LIBS)

install: serwebusb
	cp -f serwebusb /usr/local/bin

clean:
	rm -f *.o *~

realclean:
	rm -f *.o *~ serwebusb *.gz *.zip

dep:
	makedepend -Y -- $(CFLAGS) -- $(SRCS) 2&>/dev/null

# DO NOT DELETE

main.o: sio.h sock.h pipe.h thread.h vlist.h cfglib.h config.h error.h
sio.o: sio.h
sock.o: sock.h
thread.o: thread.h
vlist.o: vlist.h
cfglib.o: cfglib.h
config.o: config.h cfglib.h string.h
string.o: string.h
pipe.o: pipe.h sio.h sock.h thread.h
error.o: error.h
http.o: http.h
websocket.o: websocket.h
