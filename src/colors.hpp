#ifndef __COLORS_HPP__
#define __COLORS_HPP__

namespace pietc {
namespace colors {

struct rgba {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
};

enum color_t {
    lightRed, red, darkRed,
    lightYellow, yellow, darkYellow,
    lightGreen, green, darkGreen,
    lightCyan, cyan, darkCyan,
    lightBlue, blue, darkBlue,
    lightMagenta, magenta, darkMagenta,
    white = -1,
    black = -2
};

}

using colors::color_t;

color_t colorFromValue(colors::rgba value);
const char * colorName(color_t color);

}

#endif // __COLORS_HPP__
