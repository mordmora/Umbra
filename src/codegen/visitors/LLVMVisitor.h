#pragma once 

#include <llvm/IR/Value.h>
#include "../../ast/Nodes.h"

namespace umbra{
    namespace code_gen{

        class LLVMBaseVisitor {
            public:
                virtual void visit(umbra::ProgramNode& program) = 0;
                virtual void visit(umbra::FunctionDefinition& function) = 0;
                // Add other visit methods for different AST nodes as needed
        }
    }
}