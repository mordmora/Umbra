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
        void accept(ASTVisitor &visitor) override{
            
        };

        const std::vector<std::unique_ptr<StatementNode>> &getStatements() const {
            return statements;
        }

        void addStatement(std::unique_ptr<StatementNode> statement) {
            statements.push_back(std::move(statement));
        }
    private:
        std::vector<std::unique_ptr<StatementNode>> statements;

};
};

#endif