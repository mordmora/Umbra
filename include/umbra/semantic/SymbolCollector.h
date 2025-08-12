#pragma once

#include <memory>
#include "umbra/ast/Nodes.h"
#include "umbra/semantic/SemanticContext.h"
#include "umbra/semantic/SymbolTable.h"
#include "umbra/ast/Visitor.h"
#include "umbra/semantic/TypeCk.h"
#include "umbra/error/ErrorManager.h"
#include "umbra/error/CompilerError.h"
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
        SymbolCollector(SemanticContext& theContext, SymbolTable& symTable, ProgramNode* node, TypeCk typeCk, ErrorManager& errorManager)
            : rootASTNode(node), theContext(theContext), symTable(symTable), typeCk(typeCk), errorManager(errorManager) {}

        SymbolTable& getSymbolTable() { return symTable; }
        SemanticContext& getTheContext() { return theContext; }

        void visitProgramNode(ProgramNode* node);

        void visitFunctionDefinition(FunctionDefinition* node);

        void visitVariableDeclaration(VariableDeclaration* node);

        void visitFunctionCall(FunctionCall* node);

        void visitPrimaryExpression(PrimaryExpression* node);

        void visitExpressionStatement(ExpressionStatement* node);


        bool validateFunctionCall(FunctionCall* node);
        std::vector<SemanticType> extractArgumentTypes(const std::vector<std::unique_ptr<Expression>>& arguments);


        void printCollectedSymbols() const;

        private:

        void registerBuiltins();
        void validateEntryPoint();

        ProgramNode* rootASTNode;
        SemanticContext& theContext;
        SymbolTable& symTable;
        TypeCk typeCk;
        ErrorManager& errorManager;

   };

}
