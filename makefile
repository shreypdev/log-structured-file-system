CC:=gcc
CFLAGS:=-g -Wall -Werror 

TESTFILES := $(wildcard apps/test*.c) 
$(info TESTFILES are $(TESTFILES))

TESTS := $(TESTFILES:apps/%.c=%)
$(info TESTS are $(TESTS))

all: $(TESTS) interactive_shell disk.o file.o

test%: apps/test%.c disk/disk.o io/file.o
	$(CC) $(CFLAGS) -o apps/$@ $^

interactive_shell: apps/interactive_shell.c disk/disk.o io/file.o
	$(CC) $(CFLAGS) -o apps/$@ $^

disk.o: disk/disk.c disk/disk.h
	$(CC) $(CFLAGS) -c -o disk/$@ $<

file.o: io/file.c io/file.h
	$(CC) $(CFLAGS) -c -o io/$@ $<

run:
	./apps/interactive_shell ./disk/vdisk 10

.PHONY: clean

clean:
	rm -rf *.o
	rm -rf disk/*.o
	rm -rf disk/vdisk
	rm -rf io/*.o
	find apps -type f -not -name '*.c' -print0 | xargs -0 rm --
