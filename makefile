CC:=gcc
CFLAGS:=-g -Wall -Werror 

TESTFILES := $(wildcard apps/test*.c) 
$(info TESTFILES are $(TESTFILES))

TESTS := $(TESTFILES:apps/%.c=%)
$(info TESTS are $(TESTS))

all: $(TESTS)

test%: apps/test%.c
	$(CC) $(CFLAGS) -o apps/$@ $^

run:
	./apps/test1
	./apps/test2

.PHONY: clean

clean:
	rm -rf *.o
	rm -rf disk/*.o
	rm -rf disk/vdisk
	rm -rf io/*.o
	find apps -type f -not -name '*.c' -print0 | xargs -0 rm --
