#include <map>
#include <iostream>
#include <boost/foreach.hpp>
#include <dispatch/dispatch.h>
#include "colors.hpp"

namespace pietc {

inline float sq(float a) {
    return a * a;
}

color_t colorFromValue(colors::rgba value)
{
    static dispatch_once_t pred;
    static std::map<color_t, colors::rgba> * color_lookup = NULL;
    dispatch_once(&pred, ^ {
            color_lookup = new std::map<color_t, colors::rgba>;
            (*color_lookup)[colors::lightRed]     = (colors::rgba){0xFF, 0xC0, 0xC0, 0xFF};
            (*color_lookup)[colors::red]          = (colors::rgba){0xFF, 0x00, 0x00, 0xFF};
            (*color_lookup)[colors::darkRed]      = (colors::rgba){0xC0, 0x00, 0x00, 0xFF};
                                             
            (*color_lookup)[colors::lightYellow]  = (colors::rgba){0xFF, 0xFF, 0xC0, 0xFF};
            (*color_lookup)[colors::yellow]       = (colors::rgba){0xFF, 0xFF, 0x00, 0xFF};
            (*color_lookup)[colors::darkYellow]   = (colors::rgba){0xC0, 0xC0, 0x00, 0xFF};

            (*color_lookup)[colors::lightGreen]   = (colors::rgba){0xC0, 0xFF, 0xC0, 0xFF};
            (*color_lookup)[colors::green]        = (colors::rgba){0x00, 0xFF, 0x00, 0xFF};
            (*color_lookup)[colors::darkGreen]    = (colors::rgba){0x00, 0xC0, 0x00, 0xFF};

            (*color_lookup)[colors::lightCyan]    = (colors::rgba){0xC0, 0xFF, 0xFF, 0xFF};
            (*color_lookup)[colors::cyan]         = (colors::rgba){0x00, 0xFF, 0xFF, 0xFF};
            (*color_lookup)[colors::darkCyan]     = (colors::rgba){0x00, 0xC0, 0xC0, 0xFF};

            (*color_lookup)[colors::lightBlue]    = (colors::rgba){0xC0, 0xC0, 0xFF, 0xFF};
            (*color_lookup)[colors::blue]         = (colors::rgba){0x00, 0x00, 0xFF, 0xFF};
            (*color_lookup)[colors::darkBlue]     = (colors::rgba){0x00, 0x00, 0xC0, 0xFF};

            (*color_lookup)[colors::lightMagenta] = (colors::rgba){0xFF, 0xC0, 0xFF, 0xFF};
            (*color_lookup)[colors::magenta]      = (colors::rgba){0xFF, 0x00, 0xFF, 0xFF};
            (*color_lookup)[colors::darkMagenta]  = (colors::rgba){0xC0, 0x00, 0xC0, 0xFF};

            (*color_lookup)[colors::white]        = (colors::rgba){0xFF, 0xFF, 0xFF, 0xFF};
            (*color_lookup)[colors::black]        = (colors::rgba){0x00, 0x00, 0x00, 0xFF};
        });
    std::pair<color_t, float> max(colors::white, 100.0);
    std::pair<color_t, colors::rgba> key;
    BOOST_FOREACH(key, *color_lookup) {
        float badnessScore = 0;
        badnessScore += sq(key.second.red - value.red);
        badnessScore += sq(key.second.green - value.green);
        badnessScore += sq(key.second.blue - value.blue);
        if (badnessScore < max.second) {
            max = std::make_pair(key.first, badnessScore);
        }
    }
    return max.first;
}

const char * colorName(color_t color) {
    static const char * names[] = {
        "black", "white",
        "lightRed", "red", "darkRed",
        "lightYellow", "yellow", "darkYellow",
        "lightGreen", "green", "darkGreen",
        "lightCyan", "cyan", "darkCyan",
        "lightBlue", "blue", "darkBlue",
        "lightMagenta", "magenta", "darkMagenta"
    };
    return names[color + 2];
}

} // namespace pietc
