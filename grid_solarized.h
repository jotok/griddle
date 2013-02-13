#ifndef GridColors_h
#define GridColors_h

#include "griddle.h"

// A 16 color palette designed for readability.
//
// http://ethanschoonover.com/solarized

rgba_t bg1      = { .red = 0.000000, .green = 0.168627, .blue = 0.211765, .alpha = 1 };
rgba_t bg2      = { .red = 0.027451, .green = 0.211765, .blue = 0.258824, .alpha = 1 };
rgba_t content1 = { .red = 0.345098, .green = 0.431373, .blue = 0.458824, .alpha = 1 };
rgba_t content2 = { .red = 0.396078, .green = 0.482353, .blue = 0.513725, .alpha = 1 };
rgba_t content3 = { .red = 0.513725, .green = 0.580392, .blue = 0.588235, .alpha = 1 };
rgba_t content4 = { .red = 0.576471, .green = 0.631373, .blue = 0.631373, .alpha = 1 };
rgba_t lightbg1 = { .red = 0.933333, .green = 0.909804, .blue = 0.835294, .alpha = 1 };
rgba_t lightbg2 = { .red = 0.992157, .green = 0.964706, .blue = 0.890196, .alpha = 1 };
rgba_t yellow   = { .red = 0.709804, .green = 0.537255, .blue = 0.000000, .alpha = 1 };
rgba_t orange   = { .red = 0.796078, .green = 0.294118, .blue = 0.086275, .alpha = 1 };
rgba_t red      = { .red = 0.862745, .green = 0.196078, .blue = 0.184314, .alpha = 1 };
rgba_t magenta  = { .red = 0.827451, .green = 0.211765, .blue = 0.509804, .alpha = 1 };
rgba_t violet   = { .red = 0.423529, .green = 0.443137, .blue = 0.768627, .alpha = 1 };
rgba_t blue     = { .red = 0.149020, .green = 0.545098, .blue = 0.823529, .alpha = 1 };
rgba_t cyan     = { .red = 0.164706, .green = 0.631373, .blue = 0.596078, .alpha = 1 };
rgba_t green    = { .red = 0.521569, .green = 0.600000, .blue = 0.000000, .alpha = 1 };

rgba_t transparent = { .alpha = 0 };

#endif
