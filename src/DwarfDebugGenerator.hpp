//
//  DwarfDebugGenerator.hpp
//  pietc
//
//  Created by Andrew Cobb on 3/18/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <llvm/Support/Dwarf.h>
#include <llvm/Value.h>
#include <llvm/LLVMContext.h>

namespace pietc { namespace dwarf {
#if 0
}} // indentation hack
#endif


llvm::MDNode * compilationUnitInfo(llvm::LLVMContext & ctx,
                                   int languageId,
                                   const char * sourceName,
                                   const char * sourceDirectory,
                                   const char * producer,
                                   bool mainCompileUnit,
                                   bool optimized,
                                   const char * flags,
                                   int runtimeVersion);

llvm::MDNode * localVariableInfo(llvm::LLVMContext & ctx,
                                 llvm::MDNode * context,
                                 const char * name,
                                 llvm::MDNode * file,
                                 int lineNumber,
                                 llvm::MDNode * type);

llvm::MDNode * fileInfo(llvm::LLVMContext & ctx,
                        const char * fileName,
                        const char * fileDirectory,
                        llvm::MDNode * compilationUnit);

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
                              llvm::Function * function);
                              
}}