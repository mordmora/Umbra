#pragma once

#include "../ast/Nodes.h"
#include "Semantic.h"

namespace umbra{

    class SemanticVisitor {
        public:
        StringInterner& interner;
        SymbolTable symbolTable;
        ScopeManager& scopeManager;
        virtual ~SemanticVisitor() = default;
        virtual void visit(ProgramNode& node) = 0;
        virtual void visit(FunctionDefinition& node) = 0;
        virtual void visit(ParameterList& node) = 0;

        //Statement section

        virtual void visit(Statement& node) = 0;
        virtual void visit(VariableDeclaration& node) = 0;
        explicit SemanticVisitor(StringInterner& interner, ScopeManager& scopeManager);
    };

    class ProgramChecker : public SemanticVisitor {
        public:
        using SemanticVisitor::SemanticVisitor;
        void visit(ProgramNode& node) override;
        void visit(FunctionDefinition& node) override;
        void visit(Statement& node) override;
        void visit(ParameterList& node) override;
        void visit(VariableDeclaration& node) override;
    };


}