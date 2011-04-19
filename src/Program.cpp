#include "Program.hpp"
#include "DwarfDebugGenerator.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <vector>
#include <queue>
#include <llvm/Support/Dwarf.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/Scalar.h>

#define EXCESSIVE_LOGGING 0

namespace pietc {
#if 0 // fix indentation
}
#endif

using boost::tuple;
using boost::shared_ptr;
using boost::scoped_ptr;
using std::list;
using std::map;
using std::multimap;
using std::pair;
using std::string;
using std::vector;
using std::set;
using std::cerr;
using std::endl;

void Program::explore(int x, int y, ColorBlock * block) {
    if (componentMap.count(std::make_pair(x, y)) > 0) {
        return;
    }
    componentMap[std::make_pair(x, y)] = block;
    block->size++;
    
    if (x + 1 < width && image.get(x, y) == image.get(x + 1, y)) {
        explore(x + 1, y, block);
    }
    if (x - 1 >= 0 && image.get(x, y) == image.get(x - 1, y)) {
        explore(x - 1, y, block);
    }
    if (y + 1 < height && image.get(x, y) == image.get(x, y + 1)) {
        explore(x, y + 1, block);
    }
    if (y - 1 >= 0 && image.get(x, y) == image.get(x, y - 1)) {
        explore(x, y - 1, block);
    }
}

bool Program::whiteCodelSlide(ColorBlock ** block, int * dp, int * cc, int x, int y) {
    set<tuple<int, int, int> > visited;
    return whiteCodelSlide(block, dp, cc, &visited, x, y);
}

bool Program::whiteCodelSlide(ColorBlock ** block, int * dp, int * cc, 
                              set<tuple<int, int, int> > * visited, int x, int y) {
    /*
    cerr << "whiteCodelSlide(";
    cerr << *dp << ", ";
    cerr << *cc << ", ";
    cerr << "{" << visited->size() << "}, ";
    cerr << x << ", ";
    cerr << y << ") = " << image.get(x, y) << endl;
    */
    
    if (image.get(x, y) != colors::white) {
        *block = componentMap[std::make_pair(x, y)];
        //cerr << "Found block " << (*block)->id << ". Returning true." << endl;
        return true;
    }
    
    if (visited->count(boost::make_tuple(x, y, *dp))) {
        //cerr << "Been here before. returning false." << endl;
        return false;
    }
    visited->insert(boost::make_tuple(x, y, *dp));
    const static int steps[4][2] = {
        { 1, 0}, {0,  1},
        {-1, 0}, {0, -1}
    };
    
    int xstep = steps[*dp][0];
    int ystep = steps[*dp][1];
    
    if (0 > x + xstep || x + xstep >= image.get_width() ||
        0 > y + ystep || y + ystep >= image.get_height() ||
        image.get(x + xstep, y + ystep) == colors::black) {
        
        // flip cc and rotate dp
        //cerr << "hit a boundary" << endl;
        *cc = !*cc;
        *dp = (*dp + 1) % 4;
    } else {
        //cerr << "moving forward" << endl;
        x += xstep;
        y += ystep;
    }

    // tail recurse
    return whiteCodelSlide(block, dp, cc, visited, x, y);
}

bool Program::computeTransition(ColorBlock * block, int dp, int cc, Transition & tran) {
    if (block->color == colors::white || block->color == colors::black) {
        return false;
    }
    
    tran.from = block;
    
    // Try to move in gray-code-like rotations
    for (int rotation = 0; rotation < 8; rotation++) {
        int dpSpin = rotation / 2;
        int ccFlip = (dpSpin % 2) ^ (rotation % 2);
        
        int newDp = (dp + dpSpin) % 4;
        int newCc = (cc + ccFlip) % 2;
        tran.obstacleTurnsDp = dpSpin;
        tran.obstacleFlipCc = ccFlip;
        pair<int, int> newLoc = extremas[block].directions[newDp][newCc];
        
        const static int steps[4][2] = {
            { 1, 0}, {0,  1},
            {-1, 0}, {0, -1}
        };
        int xstep = steps[newDp][0];
        int ystep = steps[newDp][1];
        
        tran.opType = Transition::normal;
        
        newLoc.first += xstep;
        newLoc.second += ystep;
        
        if (newLoc.first < 0 || newLoc.first >= width || 
            newLoc.second < 0 || newLoc.second >= height) {

            tran.to = NULL;
            continue;
        }
        color_t newColor = image.get(newLoc.first, newLoc.second);
        if (newColor == colors::black) {
            tran.to = NULL;
            continue;
        }
        
        if (newColor != colors::white) {
            // We have found our transition
            tran.to = componentMap[newLoc];
            
            //std::cerr << tran.from->id << "(" << dp << ", " << cc << ")"
            //std::cerr << " ==" << tran.obstacleTurnsDp << "=" << tran.obstacleFlipCc << "==> ";
            //std::cerr << tran.to->id << std::endl;
            
            return true;
        } else if (whiteCodelSlide(&tran.to, &newDp, &newCc, newLoc.first, newLoc.second)) {
            // We have found out transition (through a white block)
            
            tran.opType = Transition::noop;
            tran.obstacleTurnsDp = (newDp - dp + 4) % 4;
            tran.obstacleFlipCc = (newCc - cc + 2) % 2;
            
            //std::cerr << tran.from->id << "(" << dp << ", " << cc << ")"
            //std::cerr << " --" << tran.obstacleTurnsDp << "-" << tran.obstacleFlipCc << "--> ";
            //std::cerr << tran.to->id << std::endl;
            
            return true;
        }
    }
    
    // This isn't reached unless program is done
    tran.opType = Transition::exit;
    tran.to = NULL;
    return true;
}

void Program::computeColorBlockTransitions(ColorBlock * block) {
    for (int dp = 0; dp < 4; dp++) {
        for (int cc = 0; cc < 2; cc++) {
            transitions.push_back(Transition());
            if (!computeTransition(block, dp, cc, transitions.back())) {
                transitions.pop_back();
            } else {
                block->transitions[dp][cc] = &transitions.back();
            }
        }
    }
}

int Program::indexOfColorBlock(ColorBlock * block) {
    int i = 0;
    BOOST_FOREACH(ColorBlock & c, colorBlocks) {
        if (&c == block) {
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
    
    //std::cout << image << std::endl;

    int idCounter = 0;
    // do a depth first search to discover connected components
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pair<int, int> pos(x, y);
            color_t color = image.get(x, y);
            if (componentMap.count(pos) == 0 && color != colors::white && color != colors::black) {
                
                std::stringstream idStream;
                idStream << "ColorBlock_" << idCounter++ << "_" << colorName(color);

                colorBlocks.push_back(ColorBlock(color, idStream.str()));
                explore(x, y, &colorBlocks.back());
            }
            //std::cout << std::setw(3) << indexOfColorBlock(componentMap[pos]);
        }
        //std::cout << std::endl;
    }
    
    typedef const pair<const pair<int, int>, ColorBlock *> componentMapPair;
    BOOST_FOREACH(componentMapPair & pixel, componentMap) {
        for (int dp = 0; dp < 4; dp++) {
            for (int cc = 0; cc < 2; cc++) {
                extremas[pixel.second].update(pixel.first, dp, cc);
            }
        }
    }
    
    BOOST_FOREACH(ColorBlock & block, colorBlocks) {
        computeColorBlockTransitions(&block);
    }
    
    std::ofstream viz("out.dot");
    
    map<string, list<string> > edges;
    
    BOOST_FOREACH(Transition & tran, transitions) {
        std::stringstream edge;
        edge << "    \"" << indexOfColorBlock(tran.from) << " - " << colorName(tran.from->color) << "\" -> ";
        if (tran.to) {
            edge << "\"" << indexOfColorBlock(tran.to) << " - " << colorName(tran.to->color) << "\"";
            string dir;
            int dp, cc;
            // generate gray-code-like rotations
            for (int ccAndDp = 0; ccAndDp < 8; ccAndDp++) {
                dp = ccAndDp / 2;
                cc = (dp % 2) ^ (ccAndDp % 2);
                if (tran.from->transitions[dp][cc] == &tran) {
                    goto outer_break; // cue velociraptors
                }
            }
        outer_break:
            dir += "RDLU"[dp];
            dir += "lr"[cc];
            dir += "{";
            dir += '0' + tran.obstacleTurnsDp;
            dir += " ";
            dir += '0' + tran.obstacleFlipCc;
            dir += "}";
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
    transitionBlocks.clear();
    
    llvm::Module * module = new llvm::Module("pietc", context);
    stackValueTy = llvm::Type::getInt32Ty(context);
    
    createRuntimeDeclarations(module);
        
    llvm::FunctionType * mainType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
    mainFn = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", module);
    
    llvm::IRBuilder<true> builder(context);
    llvm::BasicBlock * mainSetup = llvm::BasicBlock::Create(context, "", mainFn);
    runtimeError = llvm::BasicBlock::Create(context, "runtime_error", mainFn);
    builder.SetInsertPoint(mainSetup);
    
    stackVar = builder.CreateAlloca(stackTy, 0, "stk");
    builder.CreateStore(builder.CreateCall(newStackFn), stackVar);
    
    llvm::BasicBlock * start = generateReachableBlocks(&colorBlocks.front(), 0, 0, module);
    
    builder.SetInsertPoint(mainSetup);
    builder.CreateBr(start);
    
    builder.SetInsertPoint(runtimeError);
    builder.CreateRet(constInt(-1));

    
    optimize(module);
    
    return module;
}

llvm::ConstantInt * Program::constInt(int x) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), x, true);
}

llvm::ConstantInt * Program::constByte(int x) {
    return llvm::ConstantInt::get(llvm::Type::getInt8Ty(context), x, true);
}

llvm::ConstantInt * Program::constBit(int x) {
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), x, true);
}

llvm::Constant * Program::constString(const char * str) {
    return llvm::ConstantArray::get(context, llvm::StringRef::StringRef(str));
}

llvm::BasicBlock * Program::generateReachableBlocks(ColorBlock * block, int dp, int cc, llvm::Module * module) {
    if (transitionBlocks.count(std::make_pair(block, dp * 2 + cc)) > 0) {
        return transitionBlocks[std::make_pair(block, dp * 2 + cc)];
    }

    llvm::IRBuilder<true> builder(context);
    
    Transition * tran = block->transitions[dp][cc];
    //if ((tran->obstacleTurnsDp || tran->obstacleFlipCc) && tran->opType != Transition::exit) {
    //    return generateReachableBlocks(block, (dp + tran->obstacleTurnsDp) % 4, (cc + tran->obstacleFlipCc) % 2, module);
    //}
    
    int operation = 0;
    if (tran->opType == Transition::normal) {
        int hueChange = (6 + (tran->to->color / 3) - (tran->from->color / 3)) % 6;
        int saturationChange = (3 + (tran->to->color % 3) - (tran->from->color % 3)) % 3;
        operation = (hueChange * 3 + saturationChange);
    }
    
    static const char * dpNames[4] = {"right", "down", "left", "up"};
    static const char * ccNames[2] = {"left", "right"};
    
    std::stringstream idStream;
    idStream << block->id + "_" << dpNames[dp] << "_" << ccNames[cc];

    llvm::BasicBlock * transitionBlock = llvm::BasicBlock::Create(context, idStream.str(), mainFn);
    transitionBlocks[std::make_pair(block, dp * 2 + cc)] = transitionBlock;
    builder.SetInsertPoint(transitionBlock);
    
    if (tran->opType == Transition::exit) {
        builder.CreateRet(constInt(0));
        // cerr << idStream.str() << " ---> exit" << endl;
        return transitionBlock;
    }
    
#if EXCESSIVE_LOGGING
    llvm::Constant * fromArray = constString(tran->from->id.c_str());
    llvm::GlobalVariable::GlobalVariable * fromGlobal;
    fromGlobal = new llvm::GlobalVariable::GlobalVariable(*module,
                                                          fromArray->getType(),
                                                          true,
                                                          llvm::Function::PrivateLinkage,
                                                          fromArray,
                                                          "");
    llvm::Value * fromStrPtr = builder.CreateConstGEP2_32(fromGlobal, 0, 0, "fromStrPtr");
    
    llvm::Constant * toArray = constString(tran->to->id.c_str());
    llvm::GlobalVariable::GlobalVariable * toGlobal;
    toGlobal = new llvm::GlobalVariable::GlobalVariable(*module,
                                                        toArray->getType(),
                                                        true,
                                                        llvm::Function::PrivateLinkage,
                                                        toArray,
                                                        "");
    llvm::Value * toStrPtr = builder.CreateConstGEP2_32(toGlobal, 0, 0, "toStrPtr");

    
    vector<llvm::Value *> logArgs;
    logArgs.push_back(constInt(operation));
    logArgs.push_back(fromStrPtr);
    logArgs.push_back(toStrPtr);
    logArgs.push_back(constByte(dp));
    logArgs.push_back(constByte(cc));
    logArgs.push_back(builder.CreateLoad(stackVar));
    
    builder.CreateCall(logStuffFn, logArgs.begin(), logArgs.end());
#endif
        
    dp = (dp + tran->obstacleTurnsDp) % 4;
    cc = (cc + tran->obstacleFlipCc) % 2;
    
    if (operation == 10 || operation == 11) { // Pointer and switch
        std::vector<llvm::BasicBlock *> branches = generateBranchOp(operation, builder);
        
        bool isPointer = (operation == 10);
        
        for (int i = 0; i < branches.size(); i++) {
            builder.SetInsertPoint(branches[i]);

            int newDp = (dp + (isPointer ? i : 0)) % 4; 
            int newCc = (cc + (isPointer ? 0 : i)) % 2;
            // cerr << idStream.str() << " -?" << i << "-> " << tran->to->id << "_" << dpNames[newDp] << "_" << ccNames[newCc] << endl;

            builder.CreateBr(generateReachableBlocks(tran->to, newDp, newCc, module));
        }
    } else { // Non-branching
        // cerr << idStream.str() << " =" << operation << "=> " << tran->to->id << "_" << dpNames[dp] << "_" << ccNames[cc] << endl;
        generateOp(operation, tran->from->size, builder);
        builder.CreateBr(generateReachableBlocks(tran->to, dp, cc, module));
    }

    return transitionBlock;
}

vector<llvm::BasicBlock *> Program::generateBranchOp(int operation, llvm::IRBuilder<true> & builder) {
    llvm::Value * stackVal = builder.CreateLoad(stackVar, "stkVal");
    assert(operation == 10 || operation == 11);
    
    bool isPointer = (operation == 10);
    
    llvm::Value * rotFlip = builder.CreateAnd(builder.CreateCall(popFn, stackVal), constInt(isPointer ? 3 : 1), "rotFlip");
    llvm::SwitchInst * switchOrPoint = builder.CreateSwitch(rotFlip, runtimeError);
    
    vector<llvm::BasicBlock *> ret;
    
    for (int i = 0; i < (isPointer ? 4 : 2); i++) {
        llvm::BasicBlock * caseBlock = llvm::BasicBlock::Create(context, "", mainFn);
        switchOrPoint->addCase(constInt(i), caseBlock);
        ret.push_back(caseBlock);
    }
    return ret;
}

void Program::generateOp(int operation, int ColorBlockSize, llvm::IRBuilder<true> & builder) {
    llvm::Value * stackVal = builder.CreateLoad(stackVar, "stkVal");
    assert(0 < operation && operation < 18);
    
    static const int stackNeededPerOp[18] = {
        0, 0, 1, // undefined, push, pop
        2, 2, 2, // add, subtract, multiply
        2, 2, 1, // divide, mode, not
        2, 1, 1, // greater, pointer, switch
        1, 0, 0, // duplicate, roll (special case, handled by runtime), in (number)
        0, 1, 1  // in (char), out (number), out(char)
    };
    
    int stackNeeded = stackNeededPerOp[operation];
    llvm::BasicBlock * finishOpBlock = NULL;
    if (stackNeeded) {
        llvm::BasicBlock * currentBlock = builder.GetInsertBlock();
        llvm::BasicBlock * doOpBlock = llvm::BasicBlock::Create(context, "doOp", currentBlock->getParent());
        doOpBlock->moveAfter(currentBlock);
        finishOpBlock = llvm::BasicBlock::Create(context, "finishOp", currentBlock->getParent());
        finishOpBlock->moveAfter(doOpBlock);
        
        llvm::Value * stackHasRoomInt = builder.CreateCall2(stackHasLengthFn, stackVal, constInt(stackNeeded));
        llvm::Value * stackHasRoomBit = builder.CreateICmpNE(stackHasRoomInt, constInt(0));
        
        builder.CreateCondBr(stackHasRoomBit, doOpBlock, finishOpBlock);
        builder.SetInsertPoint(doOpBlock);
    }
    
    switch (operation) {
        case 1: {
            // Push
            builder.CreateCall2(pushFn, stackVal, constInt(ColorBlockSize));
            break;
        }

        case 2: { 
            // Pop
            builder.CreateCall(popFn, stackVal);
            break;
        }
            
        case 3: {
            // Add
            llvm::Value * b = builder.CreateCall(popFn, stackVal, "b");
            llvm::Value * a = builder.CreateCall(popFn, stackVal, "a");
            llvm::Value * sum = builder.CreateAdd(a, b, "sum");
            builder.CreateCall2(pushFn, stackVal, sum);
            break;
        }
            
        case 4: {
            // Subtract
            llvm::Value * b = builder.CreateCall(popFn, stackVal, "b");
            llvm::Value * a = builder.CreateCall(popFn, stackVal, "a");
            llvm::Value * difference = builder.CreateSub(a, b, "difference");
            builder.CreateCall2(pushFn, stackVal, difference);
            break;
        }
        case 5: {
            // Multiply
            llvm::Value * b = builder.CreateCall(popFn, stackVal, "b");
            llvm::Value * a = builder.CreateCall(popFn, stackVal, "a");
            llvm::Value * product = builder.CreateMul(a, b, "product");
            builder.CreateCall2(pushFn, stackVal, product);
            break;
        }
        case 6: {
            // Divide
            llvm::Value * b = builder.CreateCall(popFn, stackVal, "b");
            llvm::Value * a = builder.CreateCall(popFn, stackVal, "a");
            llvm::Value * quotient = builder.CreateUDiv(a, b, "quotient");
            builder.CreateCall2(pushFn, stackVal, quotient);
            break;
        }
        case 7: {
            // Mod
            llvm::Value * b = builder.CreateCall(popFn, stackVal, "b");
            llvm::Value * a = builder.CreateCall(popFn, stackVal, "a");
            llvm::Value * remainder = builder.CreateURem(a, b, "remainder");
            builder.CreateCall2(pushFn, stackVal, remainder);
            break;
        }
        case 8: {
            // Not
            llvm::Value * a = builder.CreateCall(popFn, stackVal, "a");
            llvm::Value * negationBit = builder.CreateICmpEQ(a, constInt(0));
            llvm::Value * negation = builder.CreateIntCast(negationBit, stackValueTy, false, "negation");
            builder.CreateCall2(pushFn, stackVal, negation);
            break;
        }
        case 9: {
            // Greater
            llvm::Value * b = builder.CreateCall(popFn, stackVal, "b");
            llvm::Value * a = builder.CreateCall(popFn, stackVal, "a");
            llvm::Value * resultBit = builder.CreateICmpSGT(a, b, "resultBit");
            llvm::Value * result = builder.CreateIntCast(resultBit, stackValueTy, false, "result");
            builder.CreateCall2(pushFn, stackVal, result);
            break;
        }
        case 12: {
            // Duplicate
            llvm::Value * a = builder.CreateCall(peekFn, stackVal, "a");
            builder.CreateCall2(pushFn, stackVal, a);
            break;
        }
        case 13: {
            // Roll
            llvm::Value * rollCount = builder.CreateCall(popFn, stackVal, "rollCount");
            llvm::Value * rollDepth = builder.CreateCall(popFn, stackVal, "rollDepth");
            builder.CreateCall3(rollStackFn, stackVal, rollCount, rollDepth);
            break;
        }
        case 14: {
            // In (number)
            llvm::Value * inNum = builder.CreateCall(getintFn, "inNum");
            builder.CreateCall2(pushFn, stackVal, inNum);
            break;
        }
        case 15: {
            // In (char)
            llvm::Value * inCh = builder.CreateCall(getcharFn, "inCh");
            builder.CreateCall2(pushFn, stackVal, inCh);
            break;
        }
        case 16: {
            // Out (number)
            llvm::Value * outNum = builder.CreateCall(popFn, stackVal, "outNum");
            builder.CreateCall(putintFn, outNum);
            break;
        }
        case 17: {
            // Out (char)
            llvm::Value * outCh = builder.CreateCall(popFn, stackVal, "outCh");
            builder.CreateCall(putcharFn, outCh);
            break;
        }
        case 0:
            // Special case (noop)
            break;
        case 10: // These are branching ops, can't deal with them in this function
        case 11:
        default:
            std::cerr << "Internal compiler error: unknown operation " << operation << std::endl;
            exit(-1);
    }
    if (finishOpBlock) {
        builder.CreateBr(finishOpBlock);
        builder.SetInsertPoint(finishOpBlock);
    }
}

void Program::optimize(llvm::Module * module) {
    //llvm::PassManager pm;
    //pm.add(llvm::createAggressiveDCEPass());
    //pm.add(llvm::createBasicAliasAnalysisPass());
    //pm.add(llvm::createInstructionCombiningPass());
    //pm.add(llvm::createReassociatePass());
    //pm.add(llvm::createGVNPass());
    //pm.add(llvm::createCFGSimplificationPass());
    assert(llvm::verifyModule(module));
    //pm.run(*module);
}

void Program::createRuntimeDeclarations(llvm::Module * module) {
    stackTy = llvm::PointerType::getUnqual(llvm::OpaqueType::get(context));
    module->addTypeName("stack", stackTy);
    
    vector<const llvm::Type *> args;
    
    // IO
    // putchar
    args.clear();
    args.push_back(stackValueTy);
    llvm::FunctionType * putcharFnTy = llvm::FunctionType::get(llvm::Type::getVoidTy(context), args, false);
    putcharFn = llvm::Function::Create(putcharFnTy, llvm::Function::ExternalLinkage, "pietc_putchar", module);
    
    // putint
    args.clear();
    args.push_back(stackValueTy);
    llvm::FunctionType * putintFnTy = llvm::FunctionType::get(llvm::Type::getVoidTy(context), args, false);
    putintFn = llvm::Function::Create(putintFnTy, llvm::Function::ExternalLinkage, "putint", module);
    
    // getchar
    args.clear();
    llvm::FunctionType * getcharFnTy = llvm::FunctionType::get(stackValueTy, false);
    getcharFn = llvm::Function::Create(getcharFnTy, llvm::Function::ExternalLinkage, "pietc_getchar", module);
    
    // getint
    args.clear();
    llvm::FunctionType * getintFnTy = llvm::FunctionType::get(stackValueTy, false);
    getintFn = llvm::Function::Create(getintFnTy, llvm::Function::ExternalLinkage, "getint", module);
    
    
    // stack operations
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
    
    // new_stack
    args.clear();
    llvm::FunctionType * newStackFnTy = llvm::FunctionType::get(stackTy, false);
    newStackFn = llvm::Function::Create(newStackFnTy, llvm::Function::ExternalLinkage, "new_stack", module);
    
    // roll_stack
    args.clear();
    args.push_back(stackTy);
    args.push_back(stackValueTy);
    args.push_back(stackValueTy);
    llvm::FunctionType * rollStackFnTy = llvm::FunctionType::get(llvm::Type::getVoidTy(context), args, false);
    rollStackFn = llvm::Function::Create(rollStackFnTy, llvm::Function::ExternalLinkage, "roll_stack", module);
    
    // stack_has_length
    args.clear();
    args.push_back(stackTy);
    args.push_back(llvm::Type::getInt32Ty(context));
    llvm::FunctionType * stackHasLengthTy = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), args, false);
    stackHasLengthFn = llvm::Function::Create(stackHasLengthTy, llvm::Function::ExternalLinkage, "stack_has_length", module);

    
    // logStuff
    args.clear();
    args.push_back(llvm::Type::getInt32Ty(context));
    args.push_back(llvm::PointerType::getInt8PtrTy(context));
    args.push_back(llvm::PointerType::getInt8PtrTy(context));
    args.push_back(llvm::PointerType::getInt8Ty(context));
    args.push_back(llvm::PointerType::getInt8Ty(context));
    args.push_back(stackTy);
    llvm::FunctionType * logStuffFnTy = llvm::FunctionType::get(llvm::Type::getVoidTy(context), args, false);
    logStuffFn = llvm::Function::Create(logStuffFnTy, llvm::Function::ExternalLinkage, "log_stuff", module);
}
 
Program::ColorBlockExtrema::ColorBlockExtrema() {
    for (int dp = 0; dp < 4; dp++) {
        for (int cc = 0; cc < 2; cc++) {
            directions[dp][cc] = std::make_pair(-1, -1);
        }
    }
}

void Program::ColorBlockExtrema::update(pair<int, int> location, int dp, int cc) {
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
