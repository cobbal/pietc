#include <iostream>
#include <fstream>
#include "ProgramImage.hpp"
#include "Program.hpp"
#include <string>

int main(int argc, char ** argv) {
    pietc::Program prog("test-images/tetris.png");
    prog.codegen()->dump();
    
    getchar();
}
