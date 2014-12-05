# File to compile the auriol-reader
all: clean auriol-reader

auriol-reader: auriol-reader.c
	gcc -Wall -ansi -o auriol-reader -I/usr/local/include -L/usr/local/lib -lwiringPi -lrt -lm -lsqlite3 auriol-reader.c db.c

clean:
	- rm -f auriol-reader
