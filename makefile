# File to compile the auriol-reader

# TODO
# Create an install target to place
# - auriol-reader in /usr/local/bin
# - start-up scripts in /etc/init.d and /etc/rcX.d
# - Webb pages in /var/www/auriol-reader and config file in /etc/apache2/sites-*

all: clean auriol-reader

auriol-reader: auriol-reader.c
	gcc -Wall -ansi -o auriol-reader -I/usr/local/include -L/usr/local/lib -lwiringPi -lrt -lm -lsqlite3 auriol-reader.c db.c

clean:
	- rm -f auriol-reader
