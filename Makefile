all: run

run: main
	 ./main

main: main.o
	$(CC) $(LDFLAGS) -o $@ $<

main.o: main.c
	$(CC) -g $(CFLAGS) -c -o $@ $<

clean:
	rm -f main main.o

.PHONY: all run clean