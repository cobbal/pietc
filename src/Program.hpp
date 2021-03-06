#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include <stdint.h>
#include "ColorBlock.hpp"
#include "Transition.hpp"
#include "ProgramImage.hpp"
#include <list>
#include <set>
#include <map>
#include <boost/tuple/tuple.hpp>
#include <llvm/DerivedTypes.h>
#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/IntrinsicInst.h>

namespace pietc {
    
    class Program {
    public:
        Program(const std::string & filename, int codelSize, llvm::LLVMContext & context);
        void codegen(llvm::Module * mod);
        
    private:
        // Types
        struct ColorBlockExtrema {
            ColorBlockExtrema();
            void update(std::pair<int, int> location, int dp, int cc);
            
            std::pair<int, int> directions[4][2];
        };
        
        // Common variables
        std::list<ColorBlock> colorBlocks;
        std::list<Transition> transitions;
        
        // Image variables
        ProgramImage image;
        const unsigned int width;
        const unsigned int height;
        std::map<ColorBlock *, ColorBlockExtrema> extremas;
        std::map<std::pair<int, int>, ColorBlock *> componentMap;
        
        // Code generation variables
        llvm::LLVMContext & context;
        
        llvm::AllocaInst * stackVar;
        
        llvm::Type * stackValueTy;
        llvm::Type * stackTy;
        std::map<std::pair<ColorBlock *, int>, llvm::BasicBlock *> transitionBlocks;
        
        // Generated functions
        llvm::Function * mainFn;
        
        // Declared functions
        llvm::Function * pushFn;
        llvm::Function * popFn;
        llvm::Function * peekFn;
        llvm::Function * newStackFn;
        llvm::Function * rollStackFn;
        llvm::Function * putcharFn;
        llvm::Function * getcharFn;
        llvm::Function * putintFn;
        llvm::Function * getintFn;
        llvm::Function * logStuffFn;
        llvm::Function * stackHasLengthFn;
        llvm::BasicBlock * runtimeError;
        
        // LLVM Intrinsic functions
        llvm::Function * debugDeclareFn;
        llvm::Function * debugValueFn;
        
        // Image functions
        void dfs(int x, int y, ColorBlock * colorBlock);
        void computeColorBlockTransitions(ColorBlock * colorBlock);
        bool computeTransition(ColorBlock * colorBlock, int dp, int cc, Transition & tran);
        bool whiteCodelSlide(ColorBlock ** ColorBlock, int * dp, int * cc, int x, int y);
        bool whiteCodelSlide(ColorBlock ** ColorBlock, int * dp, int * cc, 
                             std::set<boost::tuple<int, int, int> > * visited, int x, int y);
        int indexOfColorBlock(ColorBlock * colorBlock);
        
        // Static code generation (the runtime)
        void createRuntimeDeclarations(llvm::Module * module);
        
        // Code generation functions
        llvm::BasicBlock * generateReachableBlocks(ColorBlock * currentBlock, int dp, int cc, llvm::Module * module);
        void generateOp(int operation, int ColorBlockSize, llvm::IRBuilder<true> & builder);
        std::vector<llvm::BasicBlock *> generateBranchOp(int operation, llvm::IRBuilder<true> & builder);
        
        void optimize(llvm::Module * module);
        
        llvm::ConstantInt * constInt(int x);
        llvm::ConstantInt * constByte(int x);
        llvm::ConstantInt * constBit(int x);
        llvm::Constant * constString(const char * str);
    };
} // namespace pietc

#endif // __PROGRAM_H__
