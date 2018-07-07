#
# makefile for fl
#

CC = gcc -Wall -Wextra  -g

fl: fl.o  process.o wordstore13.o

fl.o: fl.c fl.h

process.o: process.c fl.h ws13.h

wordstore13.o: wordstore13.c ws13.h

clean:
	rm -f *.o fl
