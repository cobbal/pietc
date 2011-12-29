#include <iostream>
#include <fstream>
#include "ProgramImage.hpp"
#include "Program.hpp"
#include <string>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/PassManager.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Host.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/TargetFolder.h>
#include <llvm/Support/Signals.h>
#include <llvm/ADT/Triple.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FormattedStream.h>
#include <fstream>
#include <getopt.h>
#include <boost/algorithm/string/predicate.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::string;

static llvm::cl::opt<string>
InputFilename(llvm::cl::Positional,
              llvm::cl::desc("<input image files>"));

static llvm::cl::opt<string>
OutputFilename("o",
               llvm::cl::desc("Output filename"),
               llvm::cl::value_desc("filename"));

static llvm::cl::opt<char>
OptLevel("O",
         llvm::cl::desc("Optimization level. [-O0, -O1, -O2, or -O3] "
                        "(default = '-O2')"),
         llvm::cl::Prefix,
         llvm::cl::ZeroOrMore,
         llvm::cl::init(' '));

static llvm::cl::opt<unsigned>
CodelSize("C",
          llvm::cl::desc("size of codels in image"),
          llvm::cl::init(1));

static inline string
GetFileNameRoot(const std::string & inName) {
    const string suffix = ".png";
    if (boost::algorithm::ends_with(inName, suffix)) {
        return inName.substr(0, inName.length() - suffix.length());
    } else {
        return inName;
    }
}

static llvm::tool_output_file *
GetOutputStream() {
 
    if (OutputFilename.empty()) {
        OutputFilename = GetFileNameRoot(InputFilename) + ".o";
    }
    
    string error;
    unsigned openFlags = llvm::raw_fd_ostream::F_Binary;
    llvm::tool_output_file * FDOut = new llvm::tool_output_file(OutputFilename.c_str(), error, openFlags);
    
    if (!error.empty()) {
        llvm::errs() << error << '\n';
        delete FDOut;
        return NULL;
    }
    
    return FDOut;
}

int main(int argc, char ** argv) {
    llvm::sys::PrintStackTraceOnErrorSignal();
    llvm::PrettyStackTraceProgram X(argc, argv);
    
    llvm::LLVMContext & context = llvm::getGlobalContext();
    llvm::llvm_shutdown_obj Y; // Call llvm_shutdown() on exit.
    
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();

    // Initialize passes
    llvm::PassRegistry & Registry = *llvm::PassRegistry::getPassRegistry();
    llvm::initializeCore(Registry);
    llvm::initializeScalarOpts(Registry);
    llvm::initializeIPO(Registry);
    llvm::initializeAnalysis(Registry);
    llvm::initializeIPA(Registry);
    llvm::initializeTransformUtils(Registry);
    llvm::initializeInstCombine(Registry);
    llvm::initializeTarget(Registry);

    llvm::cl::ParseCommandLineOptions(argc, argv, "piet compiler\n");
    
    cerr << "building program" << flush;
    pietc::Program prog(InputFilename, CodelSize, context);
    cerr << " done" << endl;
    
    llvm::OwningPtr<llvm::tool_output_file> out(GetOutputStream());
    if (!out) {
        return 1;
    }

    llvm::Module mod("pietc", context);
    cerr << "code gening... " << flush;
    prog.codegen(&mod);
    cerr << "done" << endl;
    
    // Build up all of the passes that we want to do to the module.
    llvm::PassManager PM;
    PM.add(new llvm::TargetData(&mod));

    string err;
    
    string triple = llvm::sys::getDefaultTargetTriple();
    const llvm::Target & target = *llvm::TargetRegistry::lookupTarget(triple, err);
    if (err != "") {
        cerr << err << endl;
        return 1;
    }

    llvm::CodeGenOpt::Level oLvl;
    switch (OptLevel) {
        default:
            cerr << argv[0] << ": invalid optimization level.\n";
            return 1;
        case ' ': break;
        case '0': oLvl = llvm::CodeGenOpt::None; break;
        case '1': oLvl = llvm::CodeGenOpt::Less; break;
        case '2': oLvl = llvm::CodeGenOpt::Default; break;
        case '3': oLvl = llvm::CodeGenOpt::Aggressive; break;
    }
    
    llvm::Reloc::Model RM = llvm::Reloc::Default;
    llvm::CodeModel::Model CM = llvm::CodeModel::Default;
    
    llvm::TargetOptions Options;
    std::auto_ptr<llvm::TargetMachine> machine(target.createTargetMachine(triple, "", "", Options, RM, CM, oLvl));
    
    {
        llvm::formatted_raw_ostream FOS(out->os());

        // Ask the target to add backend passes as necessary.
        if (machine->addPassesToEmitFile(PM, FOS, llvm::TargetMachine::CGFT_ObjectFile, false)) {
            cerr << argv[0] << ": target does not support generation of this"
                << " file type!\n";
            return 1;
        }

        // Before executing passes, print the final values of the LLVM options.
        llvm::cl::PrintOptionValues();

        cerr << "compiling... " << flush;
        PM.run(mod);
        cerr << "done" << endl;
    }

    // Declare success.
    out->keep();
    
#if 0
    std::ofstream outputStream("output.ll");
    llvm::raw_os_ostream lloutput(outputStream);
    mod.print(lloutput, NULL);
    outputStream.close();
#endif
    
    return 0;
}
