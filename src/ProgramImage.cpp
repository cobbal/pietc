#include "ProgramImage.hpp"
#include <png.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <dispatch/dispatch.h>
#include <boost/scoped_array.hpp>
#include <setjmp.h>
#include <map>

using std::cerr;
using std::endl;
using boost::scoped_array;
using boost::multi_array;

namespace pietc {
        
static void __attribute__((noreturn)) longjmp_wrapper(jmp_buf env, int val);
    
ProgramImage::ProgramImage(const std::string & filename, int codelSize) {    
    FILE * fp = fopen(filename.c_str(), "rb");
    if (!fp) {
        cerr << "Error: could not open \"" << filename << "\"" << endl;
        exit(1);
    }
    
    png_byte header[8];
    fread((char *)header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
        cerr << "Error: \"" << filename << "\"" << " does not appear to be a PNG file." << endl;
        exit(1);
    }
    
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        cerr << "Error: libpng failed to initialize" << endl;
        exit(1);
    }
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        cerr << "Error: libpng failed to initialize" << endl;
    }
    
    // can't use png_jumpbuf here due to conflicting types for longjmp
    if (setjmp(*png_set_longjmp_fn(png_ptr, longjmp_wrapper, sizeof(jmp_buf)))) {
        cerr << "Error: problem reading PNG file" << endl;
        exit(1);
    }
    
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    
    png_read_info(png_ptr, info_ptr);
    
    int image_width = png_get_image_width(png_ptr, info_ptr);
    int image_height = png_get_image_height(png_ptr, info_ptr);
    
    png_set_palette_to_rgb(png_ptr);
    png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
    png_read_update_info(png_ptr, info_ptr);
    
    multi_array<png_byte, 2> image_data(boost::extents[image_height][png_get_rowbytes(png_ptr, info_ptr)]);

    scoped_array<png_bytep> rows(new png_bytep[image_height]);
    for (int i = 0; i < image_height; i++) {
        rows[i] = &image_data[i][0];
    }
    
    png_read_image(png_ptr, rows.get());
    
    fclose(fp);
    
    width = image_width / codelSize;
    height = image_height / codelSize;
    
    program.resize(boost::extents[height][width]);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double components[4] = {0};
            for (int i = 0; i < codelSize; i++) {
                for (int j = 0; j < codelSize; j++) {
                    components[0] += image_data[codelSize * y + i][4 * (codelSize * x + j) + 0];
                    components[1] += image_data[codelSize * y + i][4 * (codelSize * x + j) + 1];
                    components[2] += image_data[codelSize * y + i][4 * (codelSize * x + j) + 2];
                    components[3] += image_data[codelSize * y + i][4 * (codelSize * x + j) + 3];
                }
            }
            colors::rgba codel;
            codel.red   = components[0] / (codelSize * codelSize);
            codel.green = components[1] / (codelSize * codelSize);
            codel.blue  = components[2] / (codelSize * codelSize);
            codel.alpha = components[3] / (codelSize * codelSize);
            program[y][x] = colorFromValue(codel);
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
            dbgHTML << (int)(*color_lookup)[program[y][x]].red << ", ";
            dbgHTML << (int)(*color_lookup)[program[y][x]].green << ", ";
            dbgHTML << (int)(*color_lookup)[program[y][x]].blue;
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
    return program[y][x];
}


std::ostream &operator<<(std::ostream &stream, const ProgramImage & prog) {
    std::ios_base::fmtflags originalFormat = stream.flags();
    for (int y = 0; y < prog.height; y++) {
        for (int x = 0; x < prog.width; x++) {
            stream << std::setw(3) << prog.program[y][x];
        }
        stream << std::endl;
    }
    stream.flags(originalFormat);
 
    return stream;
}

// For some reason, longjmp isn't declared as noreturn, so we wrap it in a noreturn function
static void __attribute__((noreturn)) longjmp_wrapper(jmp_buf env, int val) {
    longjmp(env, val);
    
    assert(false);
    exit(1);
}
    
} // namespace pietc
