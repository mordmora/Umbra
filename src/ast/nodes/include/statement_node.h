#ifndef STATEMENT_NODE_H
#define STATEMENT_NODE_H

#include "../ASTNode.h"

namespace umbra{

class StatementNode : public ASTNode{
    public:
        virtual ~StatementNode() = default;
};

};

#endif