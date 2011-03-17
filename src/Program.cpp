#include "Program.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <vector>

namespace pietc {
#if 0 // fix indentation
}
#endif

using boost::shared_ptr;
using std::list;
using std::map;
using std::pair;
using std::string;
using std::vector;

void Program::explore(int x, int y, Codel * codel) {
    if (componentMap.count(std::make_pair(x, y)) > 0) {
        return;
    }
    componentMap[std::make_pair(x, y)] = codel;
    codel->size++;
    
    if (x + 1 < width && image.get(x, y) == image.get(x + 1, y)) {
        explore(x + 1, y, codel);
    }
    if (x - 1 > 0 && image.get(x, y) == image.get(x - 1, y)) {
        explore(x - 1, y, codel);
    }
    if (y + 1 < height && image.get(x, y) == image.get(x, y + 1)) {
        explore(x, y + 1, codel);
    }
    if (y - 1 > 0 && image.get(x, y) == image.get(x, y - 1)) {
        explore(x, y - 1, codel);
    }
}

bool Program::computeTransition(Codel * codel, int dp, int cc, Transition & tran) {
    if (codel->color == colors::white || codel->color == colors::black) {
        return false;
    }
    
    tran.from = codel;
    
    // Try to move
    for (int dpSpin = 0; dpSpin < 4; ++dpSpin) {
        for (int ccflip = 0; ccflip < 2; ++ccflip) {
            tran.obstacleTurnsDp = (dp + dpSpin) % 4;
            tran.obstacleFlipCc = (cc + ccflip) % 2;
            pair<int, int> newLoc = extremas[codel].directions[tran.obstacleTurnsDp][tran.obstacleFlipCc];
            
            const static int steps[4][2] = {
                { 1, 0}, {0,  1},
                {-1, 0}, {0, -1}
            };
            int xstep = steps[tran.obstacleTurnsDp][0];
            int ystep = steps[tran.obstacleTurnsDp][1];
            
            tran.opType = Transition::normal;
            
            while (true) {
                newLoc.first += xstep;
                newLoc.second += ystep;
                
                if (newLoc.first < 0 || newLoc.first >= width || 
                    newLoc.second < 0 || newLoc.second >= height) {
                    
                    break;
                }
                color_t newColor = image.get(newLoc.first, newLoc.second);
                if (newColor == colors::black) {
                    tran.to = NULL;
                    break;
                }
                
                if (newColor != colors::white) {
                    // We have found our transition
                    tran.to = componentMap[newLoc];
                    return true;
                }
                tran.opType = Transition::noop;
            }
        } 
    }
    
    // This isn't reached unless program is done
    tran.opType = Transition::exit;
    tran.to = NULL;
    return true;
}

void Program::computeCodelTransitions(Codel * codel) {
    for (int dp = 0; dp < 4; dp++) {
        for (int cc = 0; cc < 2; cc++) {
            transitions.push_back(Transition());
            if (!computeTransition(codel, dp, cc, transitions.back())) {
                transitions.pop_back();
            } else {
                codel->transitions[dp][cc] = &transitions.back();
            }
        }
    }
}

int Program::indexOfCodel(Codel * codel) {
    int i = 0;
    BOOST_FOREACH(Codel & c, codels) {
        if (&c == codel) {
            return i;
        }
        i++;
    }
    return -1;
}

Program::Program(const char * filename) :
image(filename), 
width(image.get_width()),
height(image.get_height()) {
    
    std::cout << image << std::endl;

    int idCounter = 0;
    // do a depth first search to discover connected components
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pair<int, int> pos(x, y);
            color_t color = image.get(x, y);
            if (componentMap.count(pos) == 0 && color != colors::white && color != colors::black) {
                
                std::stringstream idStream;
                idStream << "codel_" << idCounter++ << "_" << colorName(color);

                codels.push_back(Codel(color, idStream.str()));
                explore(x, y, &codels.back());
            }
            std::cout << std::setw(3) << indexOfCodel(componentMap[pos]);
        }
        std::cout << std::endl;
    }
    
    typedef const pair<const pair<int, int>, Codel *> componentMapPair;
    BOOST_FOREACH(componentMapPair & pixel, componentMap) {
        for (int dp = 0; dp < 4; dp++) {
            for (int cc = 0; cc < 2; cc++) {
                extremas[pixel.second].update(pixel.first, dp, cc);
            }
        }
    }
    
    BOOST_FOREACH(Codel & codel, codels) {
        computeCodelTransitions(&codel);
    }
    
    std::ofstream viz("out.dot");
    
    map<string, list<string> > edges;
    
    BOOST_FOREACH(Transition & tran, transitions) {
        std::stringstream edge;
        edge << "    \"" << indexOfCodel(tran.from) << " - " << colorName(tran.from->color) << "\" -> ";
        if (tran.to) {
            edge << "\"" << indexOfCodel(tran.to) << " - " << colorName(tran.to->color) << "\"";
            string dir;
            int dp, cc;
            for (dp = 0; dp < 4; dp++) {
                for (cc = 0; cc < 2; cc++) {
                    if (tran.from->transitions[dp][cc] == &tran) {
                        goto outer_break; // cue velociraptors
                    }
                }
            }
        outer_break:
            dir += "RDLU"[dp];
            dir += "lr"[cc];
            edges[edge.str()].push_back(dir);
        } else {
            edge << "EXIT";
            edges[edge.str()];
        }
    }
    
    viz << "digraph \"" << filename << "\" {" << std::endl;
    typedef const pair<string, list<string> > edgePair;
    BOOST_FOREACH(edgePair & e, edges) {
        viz << e.first << " [ label = \"";
        bool first = true;
        BOOST_FOREACH(const string & label, e.second) {
            viz << (first ? "" : ", ") << label;
            first = false;
        }
        viz << "\" ];" << std::endl;
    }
    viz << "}" << std::endl;
    viz.close();
}

llvm::Module * Program::codegen() {
    codelBlocks.clear();
    
    llvm::Module * module = new llvm::Module("pietc", context);
    stackValueTy = llvm::Type::getInt32Ty(context);
    
    createRuntimeDeclarations(module);
    
    llvm::FunctionType * mainType = llvm::FunctionType::get(llvm::Type::getVoidTy(context), false);
    mainFn = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", module);

    llvm::IRBuilder<true> builder(context);
    llvm::BasicBlock * mainSetup = llvm::BasicBlock::Create(context, "entry", mainFn);
    builder.SetInsertPoint(mainSetup);
    
    dpVar = builder.CreateAlloca(llvm::Type::getInt8Ty(context), 0, "dp");
    ccVar = builder.CreateAlloca(llvm::Type::getInt8Ty(context), 0, "cc");
    builder.CreateStore(constInt(0), dpVar);
    builder.CreateStore(constInt(0), ccVar);
    
    // create all the blocks first, then fill them out
    BOOST_FOREACH(Codel & codel, codels) {
        codelBlocks[&codel] = llvm::BasicBlock::Create(context, codel.id, mainFn);
    }
    runtimeError = llvm::BasicBlock::Create(context, "runtime_error", mainFn);
    
    typedef const pair<Codel *, llvm::BasicBlock *> CodelBlock;
    BOOST_FOREACH(CodelBlock & cb, codelBlocks) {
        builder.SetInsertPoint(cb.second);
        generateCodelCode(cb.first, builder);
    }
    
    return module;
}

llvm::ConstantInt * Program::constInt(int x) {
    return llvm::ConstantInt::get(llvm::Type::getInt8Ty(context), x, true);
}

void Program::generateCodelCode(Codel * codel, llvm::IRBuilder<true> & builder) {
    if (codel->transitions[0][0]->opType == Transition::exit) {
        // if one transition is an exit transition, they all are.
        builder.CreateRetVoid();
        return;
    }
    
    llvm::Value * dpValue = builder.CreateLoad(dpVar);
    llvm::Value * ccValue = builder.CreateLoad(ccVar);
    
    llvm::Value * combinedValue;
    combinedValue = builder.CreateMul(dpValue, constInt(2));
    combinedValue = builder.CreateAdd(combinedValue, ccValue);
    
    
    llvm::SwitchInst * codelSwitch = builder.CreateSwitch(combinedValue, runtimeError);
    llvm::BasicBlock * lastBlock = codelBlocks[codel];
    for (int dp = 0; dp < 4; dp++) {
        for (int cc = 0; cc < 2; cc++) {
            llvm::BasicBlock * caseBlock = llvm::BasicBlock::Create(context, "", mainFn);
            caseBlock->moveAfter(lastBlock);
            lastBlock = caseBlock;
            
            codelSwitch->addCase(constInt(dp * 2 + cc),
                                 caseBlock);
            Transition * tran = codel->transitions[dp][cc];
            builder.SetInsertPoint(caseBlock);
            
            llvm::Value * dpRot = constInt(tran->obstacleTurnsDp);
            llvm::Value * newDp = builder.CreateAdd(dpValue, dpRot);
            newDp = builder.CreateURem(newDp, constInt(4));
            builder.CreateStore(newDp, dpVar);
            
            llvm::Value * ccRot = constInt(tran->obstacleFlipCc);
            llvm::Value * newCc = builder.CreateAdd(ccValue, ccRot);
            newCc = builder.CreateURem(newCc, constInt(2));
            builder.CreateStore(newCc, ccVar);
            
            if (tran->opType == Transition::normal) {
                int operation = (tran->from->color - tran->to->color + colors::chromaticCount) % colors::chromaticCount;
                generateOperationCode(operation, tran->from->size, builder);
            }
        }
    }
}

void Program::generateOperationCode(int operation, int codelSize, llvm::IRBuilder<true> & builder) {
    switch (operation) {
        case 1: // Push
            
        case 0:
        default:
            //std::cerr << "Internal compiler error: unknown operation " << operation << std::endl;
            //exit(-1);
            ;
    }
}

void Program::createRuntimeDeclarations(llvm::Module * module) {
    // Create a list type for the stack
    // Borrowed from http://llvm.org/docs/ProgrammersManual.html#TypeResolve
    stackTy = llvm::PointerType::getUnqual(llvm::OpaqueType::get(context));
    module->addTypeName("stack", stackTy);

    // Generate functions
    /*
    createMemoryDeclarations(module);
    createConsFn(module);
    createCarFn(module);
    createCdrFn(module);
     */
    
    vector<const llvm::Type *> args;
    
    // push
    args.clear();
    args.push_back(stackTy);
    args.push_back(stackValueTy);
    llvm::FunctionType * pushFnTy = llvm::FunctionType::get(llvm::Type::getVoidTy(context), args, false);
    pushFn = llvm::Function::Create(pushFnTy, llvm::Function::ExternalLinkage, "push", module);
    
    // pop
    args.clear();
    args.push_back(stackTy);
    llvm::FunctionType * popFnTy = llvm::FunctionType::get(stackValueTy, args, false);
    popFn = llvm::Function::Create(popFnTy, llvm::Function::ExternalLinkage, "pop", module);
    
    // peek
    args.clear();
    args.push_back(stackTy);
    llvm::FunctionType * peekFnTy = llvm::FunctionType::get(stackValueTy, args, false);
    peekFn = llvm::Function::Create(peekFnTy, llvm::Function::ExternalLinkage, "peek", module);
}

/*
void Program::createMemoryDeclarations(llvm::Module * module) {
    vector<const llvm::Type *> args;
    args.push_back(llvm::Type::getInt32Ty(context));
    llvm::FunctionType * mallocFnTy = llvm::FunctionType::get(llvm::Type::getInt8PtrTy(context), args, false);
    mallocFn = llvm::Function::Create(mallocFnTy, llvm::Function::ExternalLinkage, "malloc", module);
    
    args.clear();
    args.push_back(llvm::Type::getInt8PtrTy(context));
    llvm::FunctionType * freeFnTy = llvm::FunctionType::get(llvm::Type::getVoidTy(context), args, false);
    freeFn = llvm::Function::Create(freeFnTy, llvm::Function::ExternalLinkage, "free", module);
}

void Program::createConsFn(llvm::Module * module) {
    vector<const llvm::Type *> args;
    args.push_back(stackValueTy);
    args.push_back(llvm::PointerType::getUnqual(listTy));
    llvm::FunctionType * consFnTy = llvm::FunctionType::get(llvm::PointerType::getUnqual(listTy), args, false);
    llvm::Function * consFn = llvm::Function::Create(consFnTy, llvm::Function::InternalLinkage, "cons", module);
   
    llvm::IRBuilder<true> builder(context);
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "", consFn));
    
    llvm::Function::arg_iterator argIt = consFn->arg_begin();
    llvm::Value * carArg = argIt++; // int
    llvm::Value * cdrArg = argIt++; // list*
        
    llvm::Value * listSize = builder.CreateConstGEP1_32(llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(listTy)), 1);
    listSize = builder.CreatePtrToInt(listSize, llvm::Type::getInt32Ty(context));
    llvm::Value * newNode = builder.CreateCall(mallocFn, listSize);
    newNode = builder.CreatePointerCast(newNode, llvm::PointerType::getUnqual(listTy), "newNode");
    
    llvm::Value * carLoc = builder.CreateConstGEP2_32(newNode, 0, 0, "carLoc");
    builder.CreateStore(carArg, carLoc);
    
    llvm::Value * cdrLoc = builder.CreateConstGEP2_32(newNode, 0, 1, "cdrLoc");
    builder.CreateStore(cdrArg, cdrLoc);
    
    builder.CreateRet(newNode);
}

void Program::createCarFn(llvm::Module * module) {
    vector<const llvm::Type *> args;
    args.push_back(llvm::PointerType::getUnqual(listTy));
    llvm::FunctionType * carFnTy = llvm::FunctionType::get(stackValueTy, args, false);
    carFn = llvm::Function::Create(carFnTy, llvm::Function::InternalLinkage, "car", module);
    
    llvm::IRBuilder<true> builder(context);
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "", carFn));
    
    llvm::Function::arg_iterator argIt = carFn->arg_begin();
    llvm::Value * listArg = argIt++; // list*
    
    llvm::Value * carLoc = builder.CreateConstGEP2_32(listArg, 0, 0, "carLoc");
    llvm::Value * car = builder.CreateLoad(carLoc, "car");
    builder.CreateRet(car);
}

void Program::createCdrFn(llvm::Module * module) {
    vector<const llvm::Type *> args;
    args.push_back(llvm::PointerType::getUnqual(listTy));
    llvm::FunctionType * cdrFnTy = llvm::FunctionType::get(llvm::PointerType::getUnqual(listTy), args, false);
    cdrFn = llvm::Function::Create(cdrFnTy, llvm::Function::InternalLinkage, "cdr", module);
    
    llvm::IRBuilder<true> builder(context);
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "", cdrFn));
    
    llvm::Function::arg_iterator argIt = cdrFn->arg_begin();
    llvm::Value * listArg = argIt++; // list*
    
    llvm::Value * cdrLoc = builder.CreateConstGEP2_32(listArg, 0, 1, "cdrLoc");
    llvm::Value * cdr = builder.CreateLoad(cdrLoc, "cdr");
    builder.CreateRet(cdr);
}
*/
 
Program::CodelExtrema::CodelExtrema() {
    for (int dp = 0; dp < 4; dp++) {
        for (int cc = 0; cc < 2; cc++) {
            directions[dp][cc] = std::make_pair(-1, -1);
        }
    }
}

void Program::CodelExtrema::update(pair<int, int> location, int dp, int cc) {
    pair<int, int> & current = directions[dp][cc];
    int xcmp = location.first - current.first;
    int ycmp = location.second - current.second;
    
    bool better = (current.first == -1);
    switch (dp) {
        case 0:
            better |= (xcmp > 0);
            better |= (xcmp == 0 && (cc == 0 ? (ycmp < 0) : (ycmp > 0)));
            break;
        case 1:
            better |= (ycmp > 0);
            better |= (ycmp == 0 && (cc == 0 ? (xcmp > 0) : (xcmp < 0)));
            break;
        case 2:
            better |= (xcmp < 0);
            better |= (xcmp == 0 && (cc == 0 ? (ycmp > 0) : (ycmp < 0)));
            break;
        case 3:
            better |= (ycmp < 0);
            better |= (ycmp == 0 && (cc == 0 ? (xcmp < 0) : (xcmp > 0)));
            break;
        default:
            assert(false);
    }
    if (better) {
        directions[dp][cc] = location;
    }
}

} // namespace pietc
