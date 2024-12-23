// primary_expression_node.h
#ifndef PRIMARY_EXPRESSION_NODE_H
#define PRIMARY_EXPRESSION_NODE_H

#include "expression_node.h"
#include "identifier_node.h"
#include <memory>

namespace umbra {

class PrimaryExpressionNode : public ExpressionNode {
public:
    explicit PrimaryExpressionNode(std::unique_ptr<IdentifierNode> identifier)
        : identifier(std::move(identifier)) {}

    const IdentifierNode* getIdentifier() const { return identifier.get(); }

    void accept(ASTVisitor& visitor) override {}

private:
    std::unique_ptr<IdentifierNode> identifier;
};

} // namespace umbra

#endif // PRIMARY_EXPRESSION_NODE_H