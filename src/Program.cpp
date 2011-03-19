#include "Program.hpp"
#include "DwarfDebugGenerator.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <vector>
#include <llvm/Support/Dwarf.h>

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
    if (x - 1 >= 0 && image.get(x, y) == image.get(x - 1, y)) {
        explore(x - 1, y, codel);
    }
    if (y + 1 < height && image.get(x, y) == image.get(x, y + 1)) {
        explore(x, y + 1, codel);
    }
    if (y - 1 >= 0 && image.get(x, y) == image.get(x, y - 1)) {
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
            int newDp = (dp + dpSpin) % 4;
            int newCc = (cc + ccflip) % 2;
            tran.obstacleTurnsDp = dpSpin;
            tran.obstacleFlipCc = ccflip;
            pair<int, int> newLoc = extremas[codel].directions[newDp][newCc];
            
            const static int steps[4][2] = {
                { 1, 0}, {0,  1},
                {-1, 0}, {0, -1}
            };
            int xstep = steps[newDp][0];
            int ystep = steps[newDp][1];
            
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
    
    //std::cout << image << std::endl;

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
            //std::cout << std::setw(4) << indexOfCodel(componentMap[pos]);
        }
        //std::cout << std::endl;
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
    codelBlocks.clear();
    
    llvm::Module * module = new llvm::Module("pietc", context);
    stackValueTy = llvm::Type::getInt32Ty(context);
    
    //debugDeclareFn = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::dbg_declare);
    //debugValueFn = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::dbg_value);

    createRuntimeDeclarations(module);
        
    /*llvm::MDNode * dbgCompilationUnit = dwarf::compilationUnitInfo(context,
                                                                   llvm::dwarf::DW_LANG_lo_user + 'P' + 'i' + 'e' + 't',
                                                                   "someimage.png", 
                                                                   "/", 
                                                                   "pietc 0.1", 
                                                                   true, 
                                                                   false, 
                                                                   "",
                                                                   0);
    llvm::NamedMDNode * moduleMD = module->getOrInsertNamedMetadata("debugInfo");
    moduleMD->addOperand(dbgCompilationUnit);
*/
     
    llvm::FunctionType * mainType = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
    mainFn = llvm::Function::Create(mainType, llvm::Function::ExternalLinkage, "main", module);
    
    llvm::IRBuilder<true> builder(context);
    llvm::BasicBlock * mainSetup = llvm::BasicBlock::Create(context, "", mainFn);
    builder.SetInsertPoint(mainSetup);
    
    dpVar = builder.CreateAlloca(llvm::Type::getInt8Ty(context), 0, "dp");
    builder.CreateStore(constByte(0), dpVar);
    
    
    //llvm::MDNode * dpMetaNode = llvm::MDNode::get(&dpVar, 1);
    //builder.CreateCall(debugValueFn, dpMetaNode, );
    
    ccVar = builder.CreateAlloca(llvm::Type::getInt8Ty(context), 0, "cc");
    builder.CreateStore(constByte(0), ccVar);
    
    stackVar = builder.CreateAlloca(stackTy, 0, "stk");
    builder.CreateStore(builder.CreateCall(newStackFn), stackVar);
    
    // create all the blocks first, then fill them out
    BOOST_FOREACH(Codel & codel, codels) {
        codelBlocks[&codel] = llvm::BasicBlock::Create(context, codel.id, mainFn);
    }
    runtimeError = llvm::BasicBlock::Create(context, "runtime_error", mainFn);
    builder.SetInsertPoint(runtimeError);
    
    typedef const pair<Codel *, llvm::BasicBlock *> CodelBlock;
    BOOST_FOREACH(CodelBlock & cb, codelBlocks) {
        builder.SetInsertPoint(cb.second);
        generateCodelCode(cb.first, builder, module);
    }
    builder.SetInsertPoint(mainSetup);
    builder.CreateBr(codelBlocks[&codels.front()]);
    builder.SetInsertPoint(runtimeError);
    builder.CreateRet(constInt(1));
    
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

void Program::generateCodelCode(Codel * codel, llvm::IRBuilder<true> & builder, llvm::Module * module) {
    if (codel->transitions[0][0]->opType == Transition::exit) {
        // if one transition is an exit transition, they all are.
        builder.CreateRet(constInt(0));
        return;
    }
    
    llvm::Value * dpValue = builder.CreateLoad(dpVar, "dpValue");
    llvm::Value * ccValue = builder.CreateLoad(ccVar, "ccValue");
    
    llvm::Value * combinedValue;
    combinedValue = builder.CreateMul(dpValue, constByte(2));
    combinedValue = builder.CreateAdd(combinedValue, ccValue);
    
    
    llvm::SwitchInst * codelSwitch = builder.CreateSwitch(combinedValue, runtimeError);
    //builder.CreateCall(llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::dbg_value));
    llvm::BasicBlock * lastBlock = codelBlocks[codel];
    static const char * dpNames[4] = {"right", "down", "left", "up"};
    static const char * ccNames[2] = {"left", "right"};
    for (int dp = 0; dp < 4; dp++) {
        for (int cc = 0; cc < 2; cc++) {
            std::stringstream idStream;
            idStream << codel->id + "_" << dpNames[dp] << "_" << ccNames[cc];
            llvm::BasicBlock * caseBlock = llvm::BasicBlock::Create(context, idStream.str().c_str(), mainFn);
            caseBlock->moveAfter(lastBlock);
            lastBlock = caseBlock;
            
            codelSwitch->addCase(constByte(dp * 2 + cc),
                                 caseBlock);
            Transition * tran = codel->transitions[dp][cc];
            builder.SetInsertPoint(caseBlock);
                        
            llvm::Value * dpRot = constByte(tran->obstacleTurnsDp);
            llvm::Value * newDp = builder.CreateAdd(dpValue, dpRot);
            newDp = builder.CreateURem(newDp, constByte(4));
            builder.CreateStore(newDp, dpVar);
            
            llvm::Value * ccRot = constByte(tran->obstacleFlipCc);
            llvm::Value * newCc = builder.CreateAdd(ccValue, ccRot);
            newCc = builder.CreateURem(newCc, constByte(2));
            builder.CreateStore(newCc, ccVar);
            
            if (tran->opType == Transition::normal) {
                int hueChange = (6 + (tran->to->color / 3) - (tran->from->color / 3)) % 6;
                int saturationChange = (3 + (tran->to->color % 3) - (tran->from->color % 3)) % 3;
                int operation = (hueChange * 3 + saturationChange);
                
#if 0
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

                builder.CreateCall4(logStuffFn, constInt(operation), fromStrPtr, toStrPtr, builder.CreateLoad(stackVar));
#endif
                
                generateOperationCode(operation, tran->from->size, builder);
            }
            builder.CreateBr(codelBlocks[tran->to]);
        }
    }
}

void Program::generateOperationCode(int operation, int codelSize, llvm::IRBuilder<true> & builder) {
    llvm::Value * stackVal = builder.CreateLoad(stackVar, "stkVal");
    switch (operation) {
        case 1: {
            // Push
            builder.CreateCall2(pushFn, stackVal, constInt(codelSize));
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
        case 10: {
            // Pointer
            llvm::Value * rotation = builder.CreateCall(popFn, stackVal, "rotation");
            
            llvm::Value * dpBeforePointerByte = builder.CreateLoad(dpVar);
            llvm::Value * dpBeforePointer = builder.CreateIntCast(dpBeforePointerByte, stackValueTy, true, "dpBeforePointer");
            llvm::Value * dpAfterPointerRaw = builder.CreateAdd(dpBeforePointer, rotation);
            llvm::Value * dpAfterPointerUncast = builder.CreateAnd(dpAfterPointerRaw, 3); // correct mod 4
            llvm::Value * newDp = builder.CreateIntCast(dpAfterPointerUncast, llvm::Type::getInt8Ty(context), true, "newDp");
            
            builder.CreateStore(newDp, dpVar);
            break;
        }
        case 11: {
            // Switch
            llvm::Value * rotation = builder.CreateCall(popFn, stackVal, "rotation");
            
            llvm::Value * ccBeforeSwitchByte = builder.CreateLoad(ccVar, "ccBeforeSwitch");
            llvm::Value * ccBeforeSwitch = builder.CreateIntCast(ccBeforeSwitchByte, stackValueTy, true, "ccBeforeSwitch");
            llvm::Value * ccAfterSwitchRaw = builder.CreateAdd(ccBeforeSwitch, rotation);
            llvm::Value * ccAfterSwitchUncast = builder.CreateAnd(ccAfterSwitchRaw, 1); // correct mod 2
            llvm::Value * newCc = builder.CreateIntCast(ccAfterSwitchUncast, llvm::Type::getInt8Ty(context), true, "newCc");
            
            builder.CreateStore(newCc, ccVar);
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
        default:
            std::cerr << "Internal compiler error: unknown operation " << operation << std::endl;
            exit(-1);
    }
}

void Program::createRuntimeDeclarations(llvm::Module * module) {
    stackTy = llvm::PointerType::getUnqual(llvm::OpaqueType::get(context));
    module->addTypeName("stack", stackTy);
    
    vector<const llvm::Type *> args;
    
    // IO
    // putchar
    args.clear();
    args.push_back(llvm::Type::getInt32Ty(context));
    llvm::FunctionType * putcharFnTy = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), args, false);
    putcharFn = llvm::Function::Create(putcharFnTy, llvm::Function::ExternalLinkage, "putchar", module);
    
    // putint
    args.clear();
    args.push_back(stackValueTy);
    llvm::FunctionType * putintFnTy = llvm::FunctionType::get(llvm::Type::getVoidTy(context), args, false);
    putintFn = llvm::Function::Create(putintFnTy, llvm::Function::ExternalLinkage, "putint", module);
    
    // getchar
    args.clear();
    llvm::FunctionType * getcharFnTy = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), false);
    getcharFn = llvm::Function::Create(getcharFnTy, llvm::Function::ExternalLinkage, "getchar", module);
    
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
    
    // logStuff
    args.clear();
    args.push_back(llvm::Type::getInt32Ty(context));
    args.push_back(llvm::PointerType::getInt8PtrTy(context));
    args.push_back(llvm::PointerType::getInt8PtrTy(context));
    args.push_back(stackTy);
    llvm::FunctionType * logStuffFnTy = llvm::FunctionType::get(llvm::Type::getVoidTy(context), args, false);
    logStuffFn = llvm::Function::Create(logStuffFnTy, llvm::Function::ExternalLinkage, "logStuff", module);
}
 
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
