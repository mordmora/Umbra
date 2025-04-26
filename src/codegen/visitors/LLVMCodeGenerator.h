#pragma once

#include "../../ast/ASTVisitor.h"
#include "../context/CodegenContext.h"
#include <llvm/IR/Type.h>
#include <memory>


namespace umbra{

    namespace code_gen{

        class LLVMCodeGenerator : public umbra::BaseVisitor{

            private:
                std::unique_ptr<CodegenContext> context;

                llvm::Type* getLLVMType(umbra::Type& umbraType);
            public:  
                LLVMCodeGenerator(std::unique_ptr<CodegenContext> context);

                void generate(umbra::ProgramNode& program);

                void dumpIR() const;

                bool writeIRToFile(const std::string& filename) const;

                void visit(umbra::ProgramNode& program) override;

                void visit(umbra::FunctionDefinition& function) override;


        }; 
    }


}