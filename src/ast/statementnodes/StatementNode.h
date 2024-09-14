#ifndef STATEMENTNODE_H
#define STATEMENTNODE_H

#include "../../lexer/Tokens.h"
#include "../ASTNode.h"
#include "../expressionnodes/ExpressionNode.h"
#include "../visitor/ASTVisitor.h"
#include <iostream>
#include <memory>
#include <vector>

namespace umbra {

class ExpressionNode;

class StatementNode : public ASTNode {
  public:
    void accept(ASTVisitor &visitor) = 0;
    virtual ~StatementNode() noexcept = default;
};

class st_VariableDeclNode : public StatementNode {
  public:
    st_VariableDeclNode(TokenType type, const std::string &name,
                        std::unique_ptr<ExpressionNode> initializer,
                        std::unique_ptr<ExpressionNode> arraySize);

    TokenType getType() const { return type; }
    const std::string &getName() const { return name; }
    ExpressionNode *getInitializer() const { return initializer.get(); }
    ExpressionNode *getArraySize() const { return arraySize.get(); }

    void accept(ASTVisitor &visitor);

    ~st_VariableDeclNode() noexcept override = default;

  private:
    TokenType type;
    std::string name;
    std::unique_ptr<ExpressionNode> initializer;
    std::unique_ptr<ExpressionNode> arraySize;
};

class ParameterNode : public ASTNode {
  public:
    ParameterNode(TokenType type, const std::string &name) : type(type), name(name) {}

    TokenType getType() const { return type; }
    const std::string &getName() const { return name; }
    void accept(ASTVisitor &visitor) = 0;

  private:
    TokenType type;
    std::string name;
};

class ReturnStatementNode : public StatementNode {
  public:
    ReturnStatementNode(std::unique_ptr<ExpressionNode> returnValue)
        : returnValue(std::move(returnValue)) {}

    const ExpressionNode *getReturnValue() const { return returnValue.get(); }
    void accept(ASTVisitor &visitor) = 0;

    ~ReturnStatementNode() noexcept override = default;

  private:
    std::unique_ptr<ExpressionNode> returnValue;
};

class FunctionDeclNode : public StatementNode {
  public:
    FunctionDeclNode(std::string name, std::vector<std::unique_ptr<ParameterNode>> params,
                     TokenType returnType, std::vector<std::unique_ptr<ASTNode>> body,
                     std::unique_ptr<ReturnStatementNode> returnStmt)
        : name(std::move(name)), params(std::move(params)), returnType(returnType),
          body(std::move(body)), returnStmt(std::move(returnStmt)) {}

    void accept(ASTVisitor &visitor) = 0;
    ~FunctionDeclNode() noexcept override = default;

  private:
    std::string name;
    std::vector<std::unique_ptr<ParameterNode>> params;
    TokenType returnType;
    std::vector<std::unique_ptr<ASTNode>> body;
    std::unique_ptr<ReturnStatementNode> returnStmt;
};

} // namespace umbra

#endif // STATEMENTNODE_H