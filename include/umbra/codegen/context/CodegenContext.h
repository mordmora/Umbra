#pragma once


#include <llvm/IR/LLVMContext.h>
#include<llvm/IR/Module.h>
#include<llvm/IR/IRBuilder.h>
#include<unordered_map>
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
            std::unordered_map<std::string, llvm::Value*> namedValues;
            std::unordered_map<std::string, llvm::Value*> globalStrings;

            llvm::Function* getPrintfFunction();

            CodegenContext(const std::string& moduleName);

            private:
            llvm::Function* printfFunction = nullptr;

        };
}} // namespace umbra::code_gen


