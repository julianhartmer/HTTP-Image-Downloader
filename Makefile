CC=gcc
CFLAGS=-std=c99 -pedantic -Wall

all: http
http: lib/tcp.o lib/network.o http.o
http.o: http.c lib/tcp.h lib/tcp.c lib/network.c

clean:
	rm -f *.o lib/*.o http

run: http
	./http http://imgs.xkcd.com/comics/compiling.png > test.png
	

run2: http
	./http http://i.imgur.com/Et2gFH3.jpg > test.jpg

run3: http
	./http http://media.giphy.com/media/amkxRknWsbWtG/giphy.gif > test.gif
