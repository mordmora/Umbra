

#pragma once

#include <memory>
#include "umbra/ast/Nodes.h"
#include "umbra/semantic/SemanticContext.h"
#include "umbra/semantic/SymbolTable.h"
#include "umbra/ast/Visitor.h"
#include "umbra/semantic/TypeCk.h"
/*
    Primera fase de analisis semantico: Recoleccion de simbolos

    Esta clase tiene como objetivo buscar y registrar todos los simbolos presentes en un programa
    ej: Declaraciones de variables, definiciones de funciones, etc.
*/


namespace umbra {
    template<template <typename> class Ptr, typename ImplClass, typename RetTy, class... ParamTys>
    class BaseV;
}

namespace umbra {

    class SymbolCollector : public BaseV<std::unique_ptr, SymbolCollector, void> {

        public:
        SymbolCollector(SemanticContext& theContext, SymbolTable& symTable, ProgramNode* node, TypeCk typeCk) : theContext(theContext), symTable(symTable), rootASTNode(node), typeCk(typeCk) {}

        SymbolTable& getSymbolTable() { return symTable; }
        SemanticContext& getTheContext() { return theContext; }

        void visitProgramNode(ProgramNode* node);

        void visitFunctionDefinition(FunctionDefinition* node);

        void visitVariableDeclaration(VariableDeclaration* node);

        private:
        ProgramNode* rootASTNode;
        SemanticContext& theContext;
        SymbolTable& symTable;
        TypeCk typeCk;

   };

}
