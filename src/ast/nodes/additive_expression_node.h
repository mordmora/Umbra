// additive_expression_node.h
#ifndef ADDITIVE_EXPRESSION_NODE_H
#define ADDITIVE_EXPRESSION_NODE_H

#include "expression_node.h"
#include <memory>

namespace umbra {

class AdditiveExpressionNode : public ExpressionNode {
public:
    enum class Operator {
        ADD,
        SUBTRACT
    };

    AdditiveExpressionNode(std::unique_ptr<ExpressionNode> left,
                           Operator op,
                           std::unique_ptr<ExpressionNode> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    const ExpressionNode* getLeft() const { return left.get(); }
    Operator getOperator() const { return op; }
    const ExpressionNode* getRight() const { return right.get(); }

    void accept(ASTVisitor& visitor) override {
        // Implementación del método accept
    }

private:
    std::unique_ptr<ExpressionNode> left;
    Operator op;
    std::unique_ptr<ExpressionNode> right;
};

} // namespace umbra

#endif // ADDITIVE_EXPRESSION_NODE_H