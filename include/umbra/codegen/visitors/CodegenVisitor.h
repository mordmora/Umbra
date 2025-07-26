#pragma once

#include "umbra/codegen/context/CodegenContext.h"
#include "umbra/ast/Nodes.h"


namespace umbra{
    namespace code_gen{


        class CodegenVisitor{
        public:
            CodegenVisitor(CodegenContext &context);

            llvm::Type* mapBuiltinTypeToLLVMType(const umbra::Type& type) const;
            std::string createFormatStringSpecifier(llvm::Type* type);
            //umbra::Type* mapLLVMTypeToBuiltinType(llvm::Type* type) const;

            //auxiliar

            llvm::Value* getLastValue() const;


            private:
            llvm::Value* lastLLVMValue = nullptr;
            CodegenContext& context;

        };
    }
}
