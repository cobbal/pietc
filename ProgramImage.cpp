#include "ProgramImage.h"
#include "png++/png.hpp"
#include <iomanip>

namespace pietc {


ProgramImage::ProgramImage(const char * filename) {
    const png::image<png::rgba_pixel> image(filename);
    const png::pixel_buffer<png::rgba_pixel> & buf = image.get_pixbuf();
    width = buf.get_width();
    height = buf.get_height();
    program.reset(new color[width * height]);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const png::rgba_pixel & pixel = buf[y][x];
            program[width * y + x] =
                colorFromValue((colors::rgba){pixel.red, pixel.green, pixel.blue, pixel.alpha});
        }
    }
}

ProgramImage::~ProgramImage() {
}

std::ostream &operator<<(std::ostream &stream, const ProgramImage & prog) {
    std::ios_base::fmtflags originalFormat = stream.flags();
    for (int y = 0; y < prog.height; y++) {
        for (int x = 0; x < prog.width; x++) {
            stream << std::setw(3) << prog.program[y * prog.width + x];
        }
        stream << std::endl;
    }
    stream.flags(originalFormat);
 
    return stream;
}

} // namespace pietc
