griddle
=======

`griddle` aims to be a C implementation using [cairo](http://cairographics.org/) of a subset of the [grid graphics](http://www.stat.auckland.ac.nz/~paul/grid/grid.html) package for R. I'm copying grid so shamelessly that the documentation for that package provides a good introduction to `griddle`. As I write this, there is still a lot of work to do, but enough bits are implemented that I can give you an idea of what I'm going for. The following code is taken from [examples/sine.c](https://github.com/jotok/griddle/blob/master/examples/sine.c).

```c
grid_context_t *gr = new_grid_context(800, 600);
```

Create a new grid context whose underlying image surface is 800 pixels wide and 600 pixels high.

```c
grid_par_t par = {.color = &transparent, .fill = &lightbg1};
grid_full_rect(gr, &par);
```

Create a parameter struct with the specified `color` and `fill` fields. `color` gives the foreground color for drawing commands, and `fill` gives the background color. `transparent` and `lightbg1` are defined in [grid\_solarized.h](https://github.com/jotok/griddle/blob/master/grid_solarized.h), which defines a palette of colors lifted from the [solarized](http://ethanschoonover.com/solarized) color scheme.

After we set the parameters, we draw a "full rectangle", meaning we draw a rectangle that takes up the entire viewport.

```c
grid_push_viewport(gr,
  new_grid_viewport(unit(0, "npc"), unit_sub(unit(1, "npc"), unit(4.1, "lines")),
                    unit(1, "npc"), unit(4.1, "lines")));
```

The `grid_push_viewport` function adds a new viewport to the viewport tree. Drawing commands are understood in the context of the current viewport. The extents of the new viewport are given as units which are understood in terms of the current viewport. The x-coordinate of the lower-left corner of the new viewport is given as `unit(0, "npc")`. "npc" stands for "normalized point coordinate; every viewport has lower left corner (0, 0) and upper right corner (1, 1) in npc. Since the x-coordinate of the new viewport is 0 npc, the left side of the new viewport will align with the left side of the current viewport.

The y coordinate (of the lower left corner) of the new viewport is given by to be

```c
unit_sub(unit(1, "npc"), unit(4.1, "lines"))
```

which we might write more simply as "1 npc - 4.1 lines". That means the y-coordinate will be 4.1 lines below the top of the current viewport, where a line is the height of a line of text in the current font. Similarly, we set the width of the new viewport to be 1 npc (i.e., full width) and the hight to be 4.1 lines.

```c
par = (grid_par_t){.color = &content1, 
                   .vjust = "middle", 
                   .font_size = unit(30, "px")};
grid_text(gr, "the sine function", NULL, NULL, &par);
grid_pop_viewport_1(gr);
```

The next commands write the words "The sine function" at the top of the image and pop the current viewport off the viewport tree. In the `grid_text` function the arguments following the text allow you position the text explicitly by giving x and y coordinates. By passing `NULL` to those arguments we indicate that we want the text to be aligned automatically. The `vjust` option in the parameter struct indicates that we want to vertically align the text in the middle of the viewport. Since we didn't specify a value for `just`, the function falls back on the global default which is to center the text. The font size is set to 30 px; more font size appropriate units (e.g., points) are not yet implemented.

Popping the current viewport from the tree brings us back to the root viewport. We can also call `grid_up_viewport_1` which changes the current viewport in the grid context without modifying the tree structure.

```c
double x[100], y[100];
int i;
for (i = 0; i < 100; i++) {
    x[i] = 2 * Pi * i / 100.0;
    y[i] = sin(x[i]);
}

grid_push_viewport(gr, new_grid_plot_viewport(gr, 4.1, 1.1, 3.1, 3.1));
grid_push_viewport(gr, new_grid_data_viewport(100, x, y));
```

These commands fill two arrays with data that we'd like to plot and push two new viewports on a tree. For this example i want to plot a sine curve, so I fill an array with 100 values between 0 and 2 * Pi, and I fill a second array with the sine function evaluate at those points.

The first new viewport is constructed using the convenience function `new_grid_plot_viewport`. It handles the common need to create a data viewport with margins on all sides. The numeric arguments give the top, right, bottom, and left margins which are taken to be in "lines" (which is why we need to pass the current grid context to this constructor). `new_grid_data_viewport` constructs a viewport that we can use to display the points defined by `x` and `y`. In particular, it constructs a coordinate system in units called "native" in which all the given points will be visible.

```c
par = (grid_par_t){.color = &transparent, .fill = &bg2};
grid_full_rect(gr, &par);
```

As before, this command will fill in the background color of the current viewport (which is the data viewport we just constructed).

```c
unit_array_t x_units = UnitArray(100, x, "native");
unit_array_t y_units = UnitArray(100, y, "native");
grid_set_line_width(gr, unit(5, "px"));
par = (grid_par_t){.color = &blue};
grid_lines(gr, &x_units, &y_units, &par);
```

Next we wrap `x` and `y` in unit structs, set the global line width to 5 px, and draw a blue line connecting the dots. It's that easy! It's so easy I do it three more times in different colors.

```c
for (i = 0; i < 100; i++) {
    y[i] = sin(x[i] + Pi/4);
}

par = (grid_par_t){.color = &orange};
grid_lines(gr, &x_units, &y_units, &par);

for (i = 0; i < 100; i++) {
    y[i] = sin(x[i] + Pi / 2);
}

par = (grid_par_t){.color = &green};
grid_lines(gr, &x_units, &y_units, &par);

for (i = 0; i < 100; i++) {
    y[i] = sin(x[i] + 3 * Pi / 4);
}

par = (grid_par_t){.color = &violet};
grid_lines(gr, &x_units, &y_units, &par);
```

Finally, we can save the image as a png by calling the cairo API.

```c
cairo_surface_write_to_png(gr->surface, "sine.png");
```
