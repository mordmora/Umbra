// expression_node.h
#ifndef EXPRESSION_NODE_H
#define EXPRESSION_NODE_H

#include "../ASTNode.h"

namespace umbra {

class ExpressionNode : public ASTNode {
public:
    virtual ~ExpressionNode() = default;
};

} // namespace umbra

#endif // EXPRESSION_NODE_H