#ifndef PARAMETER_NODE_H
#define PARAMETER_NODE_H

#include <string>
#include "../ASTNode.h"
#include "../../../lexer/Lexer.h"

namespace umbra {

class ParamNode : public ASTNode {
public:
    ParamNode(const TokenType type, const std::string& identifier)
        : type(type), identifier(identifier) {}
    ~ParamNode() = default;

    const std::string& getIdentifier() const { return identifier; }
    TokenType getType() const { return type; }

    void accept(ASTVisitor& visitor) override {}

private:
    TokenType type;
    std::string identifier;
};

} // namespace umbra

#endif // PARAMETER_NODE_H