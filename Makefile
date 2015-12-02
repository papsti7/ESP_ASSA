#------------------------------------------------------------------------------
# assa.c
#------------------------------------------------------------------------------

CFLAGS+=-Wall
SOURCE=assa.c
BINARY=assa

all: clean assa

assa:
	gcc $(CFLAGS) -o bin/$(BINARY) src/$(SOURCE)

run:
	./bin/$(BINARY)

valgrind:
	valgrind --leak-check=full ./bin/$(BINARY)

clean:
	rm -rf bin/
	rm -rf *.o
	mkdir bin