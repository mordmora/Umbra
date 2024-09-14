#include "StatementNode.h"
#include "../visitor/ASTVisitor.h"
#include <memory>
#include <string>

namespace umbra {

st_VariableDeclNode::st_VariableDeclNode(TokenType type, const std::string &name,
                                         std::unique_ptr<ExpressionNode> initializer = nullptr,
                                         std::unique_ptr<ExpressionNode> arraySize = nullptr)
    : type(type), name(name), initializer(std::move(initializer)), arraySize(std::move(arraySize)) {
}

void st_VariableDeclNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }

} // namespace umbra