#include <ostream>
#include <boost/shared_array.hpp>
#include "colors.h"

namespace pietc {

class ProgramImage {
public:
    ProgramImage(const char * filename);
    ~ProgramImage();

    friend std::ostream &operator<<(std::ostream &stream, const ProgramImage & prog);

private:
    unsigned int width, height;
    boost::shared_array<color> program;
};

}

