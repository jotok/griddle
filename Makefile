P = example
OBJECTS = griddle.o
CFLAGS = -g -Wall \
		 -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include \
		 -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng15
LDLIBS = -lcairo -lm
CC=c99

$(P): $(OBJECTS)

griddle_tests: $(OBJECTS) CuTest.o

color_test: $(OBJECTS)

test: griddle_tests
	./griddle_tests

doc: griddle.h griddle.c
	doxygen doxygen_config

clean:
	rm -rf $(OBJECTS) CuTest.o griddle_tests example hello.png
