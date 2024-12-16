#include <string>
#ifndef FUCTION_DEFINITION_NODE_H
#define FUCTION_DEFINITION_NODE_H

#include "../ASTNode.h"
#include "function_definition_tail_node.h"

namespace umbra {

    class FunctionDefinitionNode : public ASTNode {

    public:
        FunctionDefinitionNode(std::string identifier, std::unique_ptr<FunctionDefinitionTailNode> tail) : identifier(identifier), tail(std::move(tail)) {}

    private:
        std::string identifier;
        std::unique_ptr<FunctionDefinitionTailNode> tail;

    };

};

#endif
