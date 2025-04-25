#pragma once

#include "../ast/Nodes.h"
#include "../error/ErrorManager.h"
#include "../ast/ASTVisitor.h"
#include "SymbolTable.h"
#include "StringInterner.h"
#include "ScopeManager.h"
#include "RvalExpressionType.h"

namespace umbra {

class SemanticVisitor : public BaseVisitor {
public:
    explicit SemanticVisitor(StringInterner& interner, ScopeManager& scopeManager, ErrorManager& errorManager);
    virtual ~SemanticVisitor() = default;

    StringInterner& interner;
    SymbolTable symbolTable;
    ScopeManager& scopeManager;

protected:
    ErrorManager& errorManager;
};

class ProgramChecker : public SemanticVisitor {
public:
    using SemanticVisitor::SemanticVisitor;

    BuiltinType rvalExpressionTypeToBuiltin(RvalExpressionType type);

    void visit(ProgramNode& node) override;
    void visit(FunctionDefinition& node) override;
    void visit(ParameterList& node) override;
    void visit(VariableDeclaration& node) override;
};

class ExpressionTypeChecker : public SemanticVisitor {
public:
    ExpressionTypeChecker(StringInterner& interner, ScopeManager& scopeManager, SymbolTable& symbolTable, ErrorManager& errorManager);

    RvalExpressionType resultType;

    void visit(Literal& node) override;
    void visit(NumericLiteral& node) override;
    void visit(StringLiteral& node) override;
    void visit(BooleanLiteral& node) override;
    void visit(Identifier& node) override;
    void visit(FunctionCall& node) override;
    void visit(PrimaryExpression& node) override;
    void visit(BinaryExpression& node) override;

private:
    ScopeManager& scopeManager;
    SymbolTable& symbolTable;
};

} // namespace umbra