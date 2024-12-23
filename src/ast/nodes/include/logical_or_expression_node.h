// logical_or_expression_node.h
#ifndef LOGICAL_OR_EXPRESSION_NODE_H
#define LOGICAL_OR_EXPRESSION_NODE_H

#include "expression_node.h"
#include <vector>
#include <memory>

namespace umbra {

    class LogicalOrExpressionNode : public ExpressionNode {
    public:
        LogicalOrExpressionNode(std::unique_ptr<ExpressionNode> left,
            std::unique_ptr<ExpressionNode> right)
            : left(std::move(left)), right(std::move(right)) {
        }

        const ExpressionNode* getLeft() const { return left.get(); }
        const ExpressionNode* getRight() const { return right.get(); }

        void accept(ASTVisitor& visitor) override {}

    private:
        std::unique_ptr<ExpressionNode> left;
        std::unique_ptr<ExpressionNode> right;
    };

} // namespace umbra

#endif // LOGICAL_OR_EXPRESSION_NODE_H
