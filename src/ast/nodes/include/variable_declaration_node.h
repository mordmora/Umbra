#ifndef VARIABLE_DECLARATION_NODE_H
#define VARIABLE_DECLARATION_NODE_H

#include <string>
#include <memory>
#include "expression_node.h"
#include "../ASTNode.h"
#include "../../../lexer/Tokens.h"
#include "statement_node.h"

namespace umbra {

    class VariableDeclarationNode : public StatementNode {
    public:
        VariableDeclarationNode(const TokenType type, const std::string& identifier, std::unique_ptr<ExpressionNode> expression = nullptr)
            : type(type), identifier(identifier), expression(std::move(expression)) {}
        ~VariableDeclarationNode() = default;

        const std::string& getIdentifier() const { return identifier; }
        TokenType getType() const { return type; }
        ExpressionNode* getExpression() const { return expression.get(); }

        void accept(ASTVisitor& visitor) override {}

    private:
        TokenType type;
        std::string identifier;
        std::unique_ptr<ExpressionNode> expression;
    };

}

#endif