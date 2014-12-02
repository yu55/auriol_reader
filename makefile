# File to compile the auriol-pluviometer-reader
all: auriol-pluviometer-reader
auriol-pluviometer-reader: auriol-reader.c
	gcc -Wall -ansi -o build/auriol-pluviometer-reader -I/usr/local/include -L/usr/local/lib -lwiringPi -lrt -lsqlite3 auriol-reader.c db.c

