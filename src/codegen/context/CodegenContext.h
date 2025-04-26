#pragma once


#include<llvm/IR/LLVMContext.h>
#include<llvm/IR/Module.h>
#include<llvm/IR/IRBuilder.h>
#include<map>
#include<string>

namespace llvm { class Value; }
namespace umbra { class Symbol; }

namespace umbra {
    namespace code_gen {
        class CodegenContext {
            public:
            llvm::LLVMContext llvmContext;
            llvm::Module llvmModule;
            llvm::IRBuilder<> llvmBuilder;
            std::map<const umbra::Symbol*, llvm::Value*> namedValues;
            std::map<std::string, llvm::Value*> globalStrings;

            CodegenContext(const std::string& moduleName);

        };
}} // namespace umbra::code_gen


