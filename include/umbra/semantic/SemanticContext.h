#pragma once
#include "umbra/semantic/SymbolTable.h"
#include "umbra/ast/Nodes.h"
#include "umbra/semantic/SemanticType.h"

namespace umbra {

    struct SemanticContext{
        SymbolTable symbolTable;
        std::unordered_map<Expression*, SemanticType> exprTypes;

        void enterScope();
        void exitScope();
        //void reportError();
    };


};
