#include <iostream>
#include <fstream>
#include "ProgramImage.hpp"
#include "Program.hpp"
#include <string>
#include <llvm/Support/raw_os_ostream.h>
#include <fstream>

int main(int argc, char ** argv) {
    pietc::Program prog("test-images/pietquest.png");
    
    std::ofstream outputStream("/Users/acobb/Desktop/test/llvm-ir/output.ll");
    llvm::raw_os_ostream lloutput(outputStream);
    
    prog.codegen()->print(lloutput, NULL);
    
    outputStream.close();
}
