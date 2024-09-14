#ifndef VISITOR_H
#define VISITOR_H

#include "../ProgramNode.h"
#include "../expressionnodes/ExpressionNode.h"
#include "../statementnodes/StatementNode.h"

namespace umbra {

class st_VariableDeclNode;

class ASTVisitor {
  public:
    virtual ~ASTVisitor() = default;

    virtual void visit(ProgramNode &node) = 0;

    // statements
    virtual void visit(st_VariableDeclNode &node) = 0;

    // expressions
    virtual void visit(ex_IntegerLiteralNode &node) = 0;
    virtual void visit(ex_FloatLiteralNode &node) = 0;
    virtual void visit(ex_StringLiteralNode &node) = 0;
    virtual void visit(ex_CharLiteralNode &node) = 0;
    virtual void visit(ex_BoolLiteralNode &node) = 0;
    virtual void visit(ex_IdentifierNode &node) = 0;
};

class PrintVisitor : public ASTVisitor {
  public:
    PrintVisitor();
    ~PrintVisitor() override;

    void visit(ProgramNode &node) override;

    // statements
    void visit(st_VariableDeclNode &node) override;

    // expressions
    void visit(ex_IntegerLiteralNode &node) override;
    void visit(ex_FloatLiteralNode &node) override;
    void visit(ex_StringLiteralNode &node) override;
    void visit(ex_CharLiteralNode &node) override;
    void visit(ex_BoolLiteralNode &node) override;
    void visit(ex_IdentifierNode &node) override;

    // Implement

  private:
    int indentLevel;
    void printIndent();
    void increaseIndent();
    void decreaseIndent();
};

} // namespace umbra

#endif // VISITOR_H