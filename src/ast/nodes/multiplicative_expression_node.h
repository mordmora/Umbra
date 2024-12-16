// multiplicative_expression_node.h
#ifndef MULTIPLICATIVE_EXPRESSION_NODE_H
#define MULTIPLICATIVE_EXPRESSION_NODE_H

#include "expression_node.h"
#include <memory>

namespace umbra {

class MultiplicativeExpressionNode : public ExpressionNode {
public:
    enum class Operator {
        MULTIPLY,
        DIVIDE,
        MODULO
    };

    MultiplicativeExpressionNode(std::unique_ptr<ExpressionNode> left,
                                 Operator op,
                                 std::unique_ptr<ExpressionNode> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    const ExpressionNode* getLeft() const { return left.get(); }
    Operator getOperator() const { return op; }
    const ExpressionNode* getRight() const { return right.get(); }

    void accept(ASTVisitor& visitor) override;

private:
    std::unique_ptr<ExpressionNode> left;
    Operator op;
    std::unique_ptr<ExpressionNode> right;
};

} // namespace umbra

#endif // MULTIPLICATIVE_EXPRESSION_NODE_H