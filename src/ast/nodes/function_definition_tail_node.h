#ifndef FUNCTION_DEFINITION_TAIL_NODE_H
#define FUNCTION_DEFINITION_TAIL_NODE_H

#include <memory>
#include <vector>
#include "../ASTNode.h"
#include "statement_list_node.h"
#include "parameters_node.h"
namespace umbra {

    class FunctionDefinitionTailNode : public ASTNode {
    public:
        FunctionDefinitionTailNode(std::string type, std::unique_ptr<StatementListNode> statements, std::unique_ptr<ParametersNode> parameters) : type(type), statements(std::move(statements)), parameters(std::move(parameters)) {}
        ~FunctionDefinitionTailNode() = default;

        const std::string &getType() const {
            return type;
        }

        const StatementListNode &getStatements() const {
            return *statements;
        }

        const ParametersNode &getParameters() const {
            return *parameters;
        }
    private:
        std::string type;
        std::unique_ptr<StatementListNode> statements;
        std::unique_ptr<ParametersNode> parameters;

    };
};
#endif
