TARGET=smsotpd
OBJECTS=main.c socket.c smsotp.c db.c
HEADERS=common.h
CC=gcc
CFLAGS=-Wall -O2 -g -DDEBUGGING
LIBS=-ldb

all: $(TARGET)

$(TARGET): $(OBJECTS) $(HEADERS)
	$(CC) -o $(TARGET) $(OBJECTS) $(CFLAGS) $(LIBS)
	#strip $(TARGET)

cscope:
	find . -name \*.h -o -name \*.c > cscope.files
	cscope -b

clean:
	rm -f $(TARGET) *.o cscope.*

install:
	sudo cp $(TARGET) /usr/local/bin/$(TARGET)

.PHONY: all test clean cscope
