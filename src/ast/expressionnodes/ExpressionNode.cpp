#include "../expressionnodes/ExpressionNode.h"
#include "../visitor/ASTVisitor.h"

namespace umbra {

ex_IntegerLiteralNode::ex_IntegerLiteralNode(int value) : value(value) {}
void ex_IntegerLiteralNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }

ex_FloatLiteralNode ::ex_FloatLiteralNode(double value) : value(value) {}
void ex_FloatLiteralNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }

ex_StringLiteralNode::ex_StringLiteralNode(std::string value) : value(value) {};
void ex_StringLiteralNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }

ex_CharLiteralNode::ex_CharLiteralNode(std::string value) : value(value) {};
void ex_CharLiteralNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }

ex_BoolLiteralNode ::ex_BoolLiteralNode(bool value) : value(value) {};
void ex_BoolLiteralNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }

ex_IdentifierNode ::ex_IdentifierNode(const std::string &name) : name(name) {}
void ex_IdentifierNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }

} // namespace umbra