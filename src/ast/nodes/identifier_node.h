// identifier_node.h
#ifndef IDENTIFIER_NODE_H
#define IDENTIFIER_NODE_H

#include "../ASTNode.h"
#include <string>

namespace umbra {

class IdentifierNode : public ASTNode {
public:
    explicit IdentifierNode(const std::string& name) : name(name) {}

    const std::string& getName() const { return name; }

    void accept(ASTVisitor& visitor) override;

private:
    std::string name;
};

} // namespace umbra

#endif // IDENTIFIER_NODE_H