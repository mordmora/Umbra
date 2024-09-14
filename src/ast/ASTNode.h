#ifndef ASTNODE_H
#define ASTNODE_H

namespace umbra {

class ASTVisitor;

class ASTNode {
  public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor &visitor) = 0;
};

} // namespace umbra

#endif // ASTNODE_H