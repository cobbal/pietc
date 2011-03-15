#include <map>
#include <boost/foreach.hpp>
#include <dispatch/dispatch.h>
#include "colors.h"

namespace pietc {

inline float sq(float a) {
    return a * a;
}

color colorFromValue(colors::rgba value)
{
    static dispatch_once_t pred;
    static std::map<color, colors::rgba> * color_lookup = NULL;
    dispatch_once(&pred, ^ {
            color_lookup = new std::map<color, colors::rgba>;
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
    std::pair<color, float> max(colors::white, 100.0);
    std::pair<color, colors::rgba> key;
    BOOST_FOREACH(key, *color_lookup) {
        float score = 0;
        score += sq(key.second.red - value.red);
        score += sq(key.second.green - value.green);
        score += sq(key.second.blue - value.blue);
        if (score > max.second) {
            max = std::make_pair(key.first, score);
        }
    }
    return max.first;
}

} // namespace pietc
