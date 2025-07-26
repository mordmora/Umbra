#pragma once
#include "umbra/semantic/SymbolTable.h"
#include "umbra/ast/Nodes.h"
#include "umbra/semantic/SemanticType.h"

/*
    En esta clase se maneja el contexto semantico. Los simbolos actuales, los alcances y
    los tipos deducidos de las expresiones.
*/

namespace umbra {

    struct SemanticContext{
        SymbolTable symbolTable;
        std::unordered_map<Expression*, SemanticType> exprTypes;

        void enterScope();
        void exitScope();
        //void reportError();
    };


};
