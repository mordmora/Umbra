#ifndef EXPRESSIONNODE_H
#define EXPRESSIONNODE_H

#include "../ASTNode.h"
#include <string>

namespace umbra {

class ASTVisitor;

class ExpressionNode : public ASTNode {
  public:
    void accept(ASTVisitor &visitor) = 0;
    virtual ~ExpressionNode() noexcept = default;
};

class ex_IntegerLiteralNode : public ExpressionNode {
  public:
    int value;
    ex_IntegerLiteralNode(int value);
    void accept(ASTVisitor &visitor);
    virtual ~ex_IntegerLiteralNode() noexcept = default;
};

class ex_FloatLiteralNode : public ExpressionNode {
  public:
    double value;
    ex_FloatLiteralNode(double value);
    void accept(ASTVisitor &visitor);
    virtual ~ex_FloatLiteralNode() noexcept = default;
};

class ex_StringLiteralNode : public ExpressionNode {
  public:
    std::string value;
    ex_StringLiteralNode(std::string value);
    void accept(ASTVisitor &visitor);
    virtual ~ex_StringLiteralNode() noexcept = default;
};

class ex_CharLiteralNode : public ExpressionNode {
  public:
    std::string value;
    ex_CharLiteralNode(std::string value);
    void accept(ASTVisitor &visitor);
    virtual ~ex_CharLiteralNode() noexcept = default;
};

class ex_BoolLiteralNode : public ExpressionNode {
  public:
    bool value;
    ex_BoolLiteralNode(bool value);
    void accept(ASTVisitor &visitor);
    virtual ~ex_BoolLiteralNode() noexcept = default;
};

class ex_IdentifierNode : public ExpressionNode {
  public:
    std::string name;
    ex_IdentifierNode(const std::string &name);
    void accept(ASTVisitor &visitor);
};

} // namespace umbra

#endif // EXPRESSIONNODE_H