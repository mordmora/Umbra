#include "ProgramNode.h"
#include "visitor/ASTVisitor.h"

namespace umbra {

void ProgramNode::accept(ASTVisitor &visitor) { visitor.visit(*this); }

} // namespace umbra