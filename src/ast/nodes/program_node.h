#ifndef PROGRAM_NODE_H
#define PROGRAM_NODE_H

#include "../ASTNode.h"
#include "function_definition_node.h"
#include <memory>
#include <vector>

namespace umbra {

    class ProgramNode : public ASTNode {
    public:
        ProgramNode(std::unique_ptr<FunctionDefinitionNode> main, std::vector<std::shared_ptr<FunctionDefinitionNode>> functions) : main(std::move(main)), functions(std::move(functions)) {}
        ~ProgramNode() = default;

        const FunctionDefinitionNode& getMain() const {
            return *main;
        }

        const std::vector<std::shared_ptr<FunctionDefinitionNode>>& getFunctions() const {
            return functions;
        }

        void accept(ASTVisitor& visitor) override;
    private:
        std::unique_ptr<FunctionDefinitionNode> main;
        std::vector<std::shared_ptr<FunctionDefinitionNode>> functions;
    };

};

#endif
