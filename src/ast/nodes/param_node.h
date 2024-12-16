#ifndef PARAMETER_NODE_H
#define PARAMETER_NODE_H

#include <string>
#include "../ASTNode.h"

namespace umbra {

class ParamNode : public ASTNode {
public:
    ParamNode(const std::string& type, const std::string& identifier)
        : type(type), identifier(identifier) {}
    ~ParamNode() = default;

    const std::string& getType() const { return type; }
    const std::string& getIdentifier() const { return identifier; }

    void accept(ASTVisitor& visitor) override {
        // Lógica para aceptar un visitante
    }

private:
    std::string type;
    std::string identifier;
};

} // namespace umbra

#endif // PARAMETER_NODE_H