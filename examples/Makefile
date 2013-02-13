EXAMPLES = basic_viewports color_test sine
OBJECTS = ../grid_units.o ../griddle.o
CFLAGS = -g -Wall -I.. \
		 -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include \
		 -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng15
LDLIBS = -lcairo -lm
CC=c99

all: $(EXAMPLES)

$(EXAMPLES): $(OBJECTS)

clean:
	rm -rf $(EXAMPLES) *.png
