#ifndef __PROGRAM_IMAGE_H__
#define __PROGRAM_IMAGE_H__

#include <ostream>
#include <string>
#include <boost/shared_array.hpp>
#include <boost/multi_array.hpp>
#include "colors.hpp"

namespace pietc {

class ProgramImage {
public:
    ProgramImage(const std::string & filename, int codelSize);
    ~ProgramImage();

    unsigned int get_width() const;
    unsigned int get_height() const;

    const color_t & get(unsigned int x, unsigned int y) const;

    friend std::ostream &operator<<(std::ostream &stream, const ProgramImage & prog);
private:
    unsigned int width, height;
    boost::multi_array<color_t, 2> program;
};

} // namespace pietc

#endif // __PROGRAM_IMAGE_H__
