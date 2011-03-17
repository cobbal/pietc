#include <iostream>
#include "ProgramImage.hpp"
#include "Program.hpp"
#include <string>

int main(int argc, char ** argv) {
    pietc::Program prog("test-images/euclid.png");
    prog.codegen()->dump();
    
    getchar();
}
