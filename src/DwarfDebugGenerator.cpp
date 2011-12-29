//
//  DwarfDebugGenerator.cpp
//  pietc
//
//  Created by Andrew Cobb on 3/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "DwarfDebugGenerator.hpp"
#include <llvm/Constants.h>
#include <llvm/Type.h>
#include <llvm/Metadata.h>
#include <vector>

namespace pietc { namespace dwarf {
#if 0
}} // indentation hack
#endif

static llvm::Value * constInt(llvm::LLVMContext & ctx, int x) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), x, true);
}
static llvm::Value * constBit(llvm::LLVMContext & ctx, int x) {
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(ctx), x, true);
}

llvm::MDNode * compilationUnitInfo(llvm::LLVMContext & ctx,
                               int languageId,
                               const char * sourceName,
                               const char * sourceDirectory,
                               const char * producer,
                               bool mainCompileUnit,
                               bool optimized,
                               const char * flags,
                               int runtimeVersion) {
    
    std::vector<llvm::Value *> values;
    values.push_back(constInt(ctx, llvm::LLVMDebugVersion + llvm::dwarf::DW_TAG_compile_unit)); // Tag
    values.push_back(constInt(ctx, 0)); // Unused
    values.push_back(constInt(ctx, languageId));
    values.push_back(llvm::MDString::get(ctx, sourceName));
    values.push_back(llvm::MDString::get(ctx, sourceDirectory));
    values.push_back(llvm::MDString::get(ctx, producer));
    values.push_back(constBit(ctx, mainCompileUnit));
    values.push_back(constBit(ctx, optimized));
    values.push_back(llvm::MDString::get(ctx, flags));
    values.push_back(constInt(ctx, runtimeVersion));

    return  llvm::MDNode::get(ctx, values);
}


llvm::MDNode * localVariableInfo(llvm::LLVMContext & ctx,
                                 llvm::MDNode * context,
                                 const char * name,
                                 llvm::MDNode * file,
                                 int lineNumber,
                                 llvm::MDNode * type) {
    
    std::vector<llvm::Value *> values;
    values.push_back(constInt(ctx, llvm::LLVMDebugVersion + llvm::dwarf::DW_TAG_auto_variable)); // Tag
    values.push_back(context);
    values.push_back(llvm::MDString::get(ctx, name));
    values.push_back(file);
    values.push_back(constInt(ctx, lineNumber));
    values.push_back(type);

    return  llvm::MDNode::get(ctx, values);
}

llvm::MDNode * fileInfo(llvm::LLVMContext & ctx,
                        const char * fileName,
                        const char * fileDirectory,
                        llvm::MDNode * compilationUnit) {
    
    std::vector<llvm::Value *> values;
    values.push_back(constInt(ctx, llvm::LLVMDebugVersion + llvm::dwarf::DW_TAG_file_type)); // Tag
    values.push_back(llvm::MDString::get(ctx, fileName));
    values.push_back(llvm::MDString::get(ctx, fileDirectory));
    values.push_back(compilationUnit);

    return  llvm::MDNode::get(ctx, values);
}

llvm::MDNode * subprogramInfo(llvm::LLVMContext & ctx,
                              llvm::MDNode * context,
                              const char * name,
                              const char * displayName,
                              const char * MIPSLinkName,
                              llvm::MDNode * file,
                              int lineNumber,
                              llvm::MDNode * type,
                              bool isStatic,
                              bool notExtern,
                              int virtuality,
                              int virtualIndex,
                              llvm::MDNode * vtableType,
                              bool artificial,
                              bool optimized,
                              llvm::Function * const function) {
    
    std::vector<llvm::Value *> values;
    values.push_back(constInt(ctx, llvm::LLVMDebugVersion + llvm::dwarf::DW_TAG_subprogram)); // Tag   
    values.push_back(constInt(ctx, 0)); // Unused
    values.push_back(context);
    values.push_back(llvm::MDString::get(ctx, name));
    values.push_back(llvm::MDString::get(ctx, displayName));
    values.push_back(llvm::MDString::get(ctx, MIPSLinkName));
    values.push_back(file);
    values.push_back(constInt(ctx, lineNumber));
    values.push_back(type);
    values.push_back(constBit(ctx, isStatic));
    values.push_back(constBit(ctx, notExtern));
    values.push_back(constInt(ctx, virtuality));
    values.push_back(constInt(ctx, virtualIndex));
    values.push_back(vtableType);
    values.push_back(constBit(ctx, artificial));
    values.push_back(constBit(ctx, optimized));
    values.push_back((llvm::Value *)function);

    return  llvm::MDNode::get(ctx, values);
}


}}
