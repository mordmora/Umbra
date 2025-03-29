#pragma once

#include "../ast/Nodes.h"
#include "Semantic.h"
#include "../error/ErrorManager.h"
#include "../ast/ASTVisitor.h"

namespace umbra{

    class SemanticVisitor : public BaseVisitor {
        public:
        StringInterner& interner;
        SymbolTable symbolTable;
        ScopeManager& scopeManager;
        virtual ~SemanticVisitor() = default;
        explicit SemanticVisitor(StringInterner& interner, ScopeManager& scopeManager, ErrorManager& errorManager);

        protected:
        ErrorManager& errorManager;

    };

    class ProgramChecker : public SemanticVisitor {
        public:
        using SemanticVisitor::SemanticVisitor;
        void visit(ProgramNode& node) override;
        void visit(FunctionDefinition& node) override;
        //void visit(Statement& node) override;
        void visit(ParameterList& node) override;
        void visit(VariableDeclaration& node) override;


    };

    
    class ExpressionTypeChecker : public SemanticVisitor {
        public:
        ExpressionTypeChecker(StringInterner& interner, ScopeManager& scopeManager, SymbolTable& symbolTable, ErrorManager& errorManager);

        RvalExpressionType resultType;

        void visit(Literal& node) override;
        void visit(Identifier& node) override;
        void visit(FunctionCall& node) override;
        void visit(PrimaryExpression& node) override;
        void visit(BinaryExpression& node) override;
        
        private:

        SymbolTable& symbolTable;
    };

}