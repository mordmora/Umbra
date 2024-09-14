#ifndef PROGRAM_NODE_H
#define PROGRAM_NODE_H

#include "ASTNode.h"
#include <memory>
#include <vector>

namespace umbra {

class ProgramNode : public ASTNode {
  public:
    ProgramNode(std::vector<std::unique_ptr<ASTNode>> ast) : ast(std::move(ast)) {}

    const std::vector<std::unique_ptr<ASTNode>> &getStatements() const { return ast; }

    void accept(ASTVisitor &visitor);
    ~ProgramNode() noexcept override = default;

  private:
    std::vector<std::unique_ptr<ASTNode>> ast;
};

} // namespace umbra

#endif // PROGRAM_NODE_H
