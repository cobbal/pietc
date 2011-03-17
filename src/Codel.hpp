#ifndef __CODEL__HPP__
#define __CODEL__HPP__

#include <stddef.h>
#include "colors.hpp"
#include <string>

namespace pietc {

class Transition;

class Codel {
public:
    Codel(color_t color, std::string id) : color(color), id(id) {
        size = 0;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 2; j++) {
                transitions[i][j] = NULL;
            }
        }
    }

    int size;
    std::string id;
    Transition * transitions[4][2]; // 2D array of pointers
    color_t color;
};

} // namespace pietc

#endif // __CODEL__HPP__
