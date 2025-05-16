
#include "CodegenContext.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

namespace umbra { 
    namespace code_gen {
    CodegenContext::CodegenContext(const std::string& moduleName)
        : llvmContext(),
        llvmModule(moduleName, llvmContext),
        llvmBuilder(llvmContext) {
            
        }


        llvm::Function* CodegenContext::getPrintfFunction() {
            
            if(printfFunction)
                return printfFunction;

            llvm::Type* charPtrType = llvm::Type::getInt8Ty(llvmContext)->getPointerTo();
            llvm::FunctionType* printfType = llvm::FunctionType::get(
                llvm::Type::getInt32Ty(llvmContext),
                llvm::ArrayRef<llvm::Type*>(charPtrType),
                true
            );

            printfFunction = llvm::Function::Create(
                printfType,
                llvm::Function::ExternalLinkage,
                "printf",
                &llvmModule
            );

            printfFunction->setCallingConv(llvm::CallingConv::C);
            return printfFunction;

        }

    }
} //namespace umbra::code_gen