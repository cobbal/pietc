namespace pietc {
namespace colors {

struct rgba {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
};

enum color {
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

using colors::color;

color colorFromValue(colors::rgba value);

}
