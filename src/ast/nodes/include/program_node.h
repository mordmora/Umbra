#ifndef PROGRAM_NODE_H
#define PROGRAM_NODE_H

#include "../ASTNode.h"
#include "function_definition_node.h"
#include <memory>
#include <vector>

namespace umbra {

    class ProgramNode : public ASTNode {
    public:
        // Constructor actualizado
        ProgramNode(std::vector<std::unique_ptr<FunctionDefinitionNode>> functions) : functions(std::move(functions)) {}
        ~ProgramNode() = default;

        const std::vector<std::unique_ptr<FunctionDefinitionNode>>& getFunctions() const {
            return functions;
        }

        void addFunction(std::unique_ptr<FunctionDefinitionNode> function) {
            functions.push_back(std::move(function));
        }

        void accept(ASTVisitor& visitor) override{}
    private:
        std::vector<std::unique_ptr<FunctionDefinitionNode>> functions;
    };

};

#endif
