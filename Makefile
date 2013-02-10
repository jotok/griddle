P = example
OBJECTS = griddle.o
CFLAGS = -g -Wall \
		 -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include \
		 -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng15
LDLIBS = -lcairo
CC=c99

$(P): $(OBJECTS)

griddle_tests: $(OBJECTS) CuTest.o

test: griddle_tests
	./griddle_tests
