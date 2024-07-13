CFLAGS = -std=c99 -ggdb -Wall -Wextra -pedantic -pedantic-errors

all: run

run: myFinger 
	 ./myFinger

myFinger: myFinger.o misc.o
	$(CC) $(LDFLAGS) -o $@ $^

myFinger.o: myFinger.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

misc.o: misc.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

clean:
	rm -f myFinger *.o

.PHONY: all run clean