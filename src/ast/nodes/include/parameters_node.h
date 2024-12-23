#ifndef PARAMETERS_NODE_H
#define PARAMETERS_NODE_H

#include <memory>
#include <vector>
#include "../ASTNode.h"
#include "param_node.h"

namespace umbra {

class ParametersNode : public ASTNode {
public:
    ParametersNode(std::vector<std::unique_ptr<ParamNode>> parameters)
        : parameters(std::move(parameters)) {}
    ~ParametersNode() = default;

    const std::vector<std::unique_ptr<ParamNode>>& getParameters() const {
        return parameters;
    }

    void addParameter(std::unique_ptr<ParamNode> parameter) {
        parameters.push_back(std::move(parameter));
    }

    void accept(ASTVisitor& visitor) override {}

private:
    std::vector<std::unique_ptr<ParamNode>> parameters;
};

} // namespace umbra

#endif // PARAMETERS_NODE_H
