// unary_expression_node.h
#ifndef UNARY_EXPRESSION_NODE_H
#define UNARY_EXPRESSION_NODE_H

#include "expression_node.h"
#include <memory>

namespace umbra {

class UnaryExpressionNode : public ExpressionNode {
public:
    enum class Operator {
        PTR,
        REF,
        ACCESS,
        NONE
    };

    UnaryExpressionNode(Operator op, std::unique_ptr<ExpressionNode> operand)
        : op(op), operand(std::move(operand)) {}

    Operator getOperator() const { return op; }
    const ExpressionNode* getOperand() const { return operand.get(); }

    void accept(ASTVisitor& visitor) override {}

private:
    Operator op;
    std::unique_ptr<ExpressionNode> operand;
};

} // namespace umbra

#endif // UNARY_EXPRESSION_NODE_H