#pragma once 

#include "../../ast/ASTVisitor.h"
#include "../context/CodegenContext.h"
#include "../../ast/Nodes.h"


namespace umbra{
    namespace code_gen{


        class CodegenVisitor : public BaseVisitor {
        public:
            CodegenVisitor();


            private: 
            CodegenContext& context;

        }


    }
}