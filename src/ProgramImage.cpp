#include "ProgramImage.hpp"
#include "png++/png.hpp"
#include <iomanip>
#include <fstream>
#include <dispatch/dispatch.h>
#include <map>

using std::endl;

namespace pietc {

ProgramImage::ProgramImage(const char * filename) {
    const png::image<png::rgba_pixel> image(filename);
    const png::pixel_buffer<png::rgba_pixel> & buf = image.get_pixbuf();
    width = buf.get_width();
    height = buf.get_height();
    program.reset(new color_t[width * height]);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            const png::rgba_pixel & pixel = buf[y][x];
            program[width * y + x] =
            colorFromValue((colors::rgba){pixel.red, pixel.green, pixel.blue, pixel.alpha});
        }
    }
    
#if 1
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
    
    std::ofstream dbgHTML("/Users/acobb/Desktop/outimg.html");
    dbgHTML << "<html>" << endl;
    dbgHTML << " <head>" << endl;
    dbgHTML << "  <title>" << filename << "</title>" << endl;
    dbgHTML << "  <style type=\"text/css\"> td { width: 4px; height: 4px; } </style>" << endl;
    dbgHTML << " </head>" << endl;
    dbgHTML << " <body>" << endl;
    dbgHTML << "  <table border=0 cellspacing=0 cellpadding=0>" << endl;
    for (int y = 0; y < height; y++) {
        dbgHTML << "   <tr>" << endl << "    ";
        for (int x = 0; x < width; x++) {
            
            dbgHTML << "<td style=\"background-color: rgb(";
            dbgHTML << (int)(*color_lookup)[program[width * y + x]].red << ", ";
            dbgHTML << (int)(*color_lookup)[program[width * y + x]].green << ", ";
            dbgHTML << (int)(*color_lookup)[program[width * y + x]].blue;
            dbgHTML << ");\"></td>";
        }
        dbgHTML << endl << "  </tr>" << endl;
    }
    dbgHTML << " </body>" << endl;
    dbgHTML << "</html>" << endl;
#endif
}

ProgramImage::~ProgramImage() {
}

unsigned int ProgramImage::get_width() const {
    return width;
}

unsigned int ProgramImage::get_height() const {
    return height;
}

const color_t & ProgramImage::get(unsigned int x, unsigned int y) const {
    return program[width * y + x];
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
