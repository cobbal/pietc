#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "Codel.hpp"
#include "Transition.hpp"
#include "ProgramImage.hpp"
#include <list>
#include <map>
#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/IRBuilder.h>

namespace pietc {
    
    class Program {
    public:
        Program(const char * filename);
        llvm::Module * codegen();
        
    private:
        // Types
        struct CodelExtrema {
            CodelExtrema();
            void update(std::pair<int, int> location, int dp, int cc);
            
            std::pair<int, int> directions[4][2];
        };
        
        // Common variables
        std::list<Codel> codels;
        std::list<Transition> transitions;
        
        // Image variables
        ProgramImage image;
        const unsigned int width;
        const unsigned int height;
        std::map<Codel *, CodelExtrema> extremas;
        std::map<std::pair<int, int>, Codel *> componentMap;
        
        // Code generation variables
        llvm::LLVMContext context;
        llvm::BasicBlock * runtimeError;
        llvm::AllocaInst * dpVar;
        llvm::AllocaInst * ccVar;
        const llvm::Type * stackValueTy;
        const llvm::Type * stackTy;
        std::map<Codel *, llvm::BasicBlock *> codelBlocks;
        
        // Generated functions
        llvm::Function * mainFn;
        
        // Declared functions
        llvm::Function * pushFn;
        llvm::Function * popFn;
        llvm::Function * peekFn;
        
        // Image functions
        void explore(int x, int y, Codel * codel);
        void computeCodelTransitions(Codel * codel);
        bool computeTransition(Codel * codel, int dp, int cc, Transition & tran);
        int indexOfCodel(Codel * codel);
        
        // Static code generation (the runtime)
        void createRuntimeDeclarations(llvm::Module * module);
        /*
        void createMemoryDeclarations(llvm::Module * module);
        void createConsFn(llvm::Module * module);
        void createCarFn(llvm::Module * module);
        void createCdrFn(llvm::Module * module);
         */
        
        // Code generation functions
        void generateCodelCode(Codel * codel, llvm::IRBuilder<true> & builder);
        void generateOperationCode(int operation, int codelSize, llvm::IRBuilder<true> & builder);
        llvm::ConstantInt * constInt(int x);
    };
} // namespace pietc

#endif // __PROGRAM_H__
