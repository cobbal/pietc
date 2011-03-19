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
    
    llvm::Value * const values[] = {
        constInt(ctx, llvm::LLVMDebugVersion + llvm::dwarf::DW_TAG_compile_unit), // Tag
        constInt(ctx, 0), // Unused
        constInt(ctx, languageId),
        llvm::MDString::get(ctx, sourceName),
        llvm::MDString::get(ctx, sourceDirectory),
        llvm::MDString::get(ctx, producer),
        constBit(ctx, mainCompileUnit),
        constBit(ctx, optimized),
        llvm::MDString::get(ctx, flags),
        constInt(ctx, runtimeVersion)
    };
    return  llvm::MDNode::get(ctx, values, sizeof(values) / sizeof(*values));
}


llvm::MDNode * localVariableInfo(llvm::LLVMContext & ctx,
                                 llvm::MDNode * context,
                                 const char * name,
                                 llvm::MDNode * file,
                                 int lineNumber,
                                 llvm::MDNode * type) {
    
    llvm::Value * const values[] = {
        constInt(ctx, llvm::LLVMDebugVersion + llvm::dwarf::DW_TAG_auto_variable), // Tag
        context,
        llvm::MDString::get(ctx, name),
        file,
        constInt(ctx, lineNumber),
        type
    };
    return  llvm::MDNode::get(ctx, values, sizeof(values) / sizeof(*values));
}

llvm::MDNode * fileInfo(llvm::LLVMContext & ctx,
                        const char * fileName,
                        const char * fileDirectory,
                        llvm::MDNode * compilationUnit) {
    
    llvm::Value * const values[] = {
        constInt(ctx, llvm::LLVMDebugVersion + llvm::dwarf::DW_TAG_file_type), // Tag
        llvm::MDString::get(ctx, fileName),
        llvm::MDString::get(ctx, fileDirectory),
        compilationUnit
    };
    return  llvm::MDNode::get(ctx, values, sizeof(values) / sizeof(*values));
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
    
    llvm::Value * const values[] = {
        constInt(ctx, llvm::LLVMDebugVersion + llvm::dwarf::DW_TAG_subprogram), // Tag   
        constInt(ctx, 0), // Unused
        context,
        llvm::MDString::get(ctx, name),        
        llvm::MDString::get(ctx, displayName),
        llvm::MDString::get(ctx, MIPSLinkName),
        file,
        constInt(ctx, lineNumber),
        type,
        constBit(ctx, isStatic),
        constBit(ctx, notExtern),
        constInt(ctx, virtuality),
        constInt(ctx, virtualIndex),
        vtableType,
        constBit(ctx, artificial),
        constBit(ctx, optimized),
        (llvm::Value *)function
    };
    return  llvm::MDNode::get(ctx, values, sizeof(values) / sizeof(*values));
}


}}