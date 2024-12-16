#ifndef STATEMENT_LIST_NODE_H
#define STATEMENT_LIST_NODE_H

#include <memory>
#include <vector>
#include "../ASTNode.h"
#include "statement_node.h"

namespace umbra{

class StatementListNode : public ASTNode{
    public:
        StatementListNode(std::vector<std::unique_ptr<StatementNode>> statements) : statements(std::move(statements)) {}
        ~StatementListNode() = default;

        const std::vector<std::unique_ptr<StatementNode>> &getStatements() const {
            return statements;
        }
    private:
        std::vector<std::unique_ptr<StatementNode>> statements;

};
};

#endif