#ifndef ELSE_STATEMENT_NODE_H
#define ELSE_STATEMENT_NODE_H

#include <memory>
#include "statement_node.h"
#include "statement_list_node.h"

namespace umbra {

class ElseStatementNode : public StatementNode {
public:
    explicit ElseStatementNode(std::unique_ptr<StatementListNode> statements)
        : statements(std::move(statements)) {}

    const StatementListNode* getStatements() const { return statements.get(); }

    void accept(ASTVisitor& visitor) override {
        // Implementación del método accept
    }

private:
    std::unique_ptr<StatementListNode> statements;
};

}; // namespace umbra

#endif // ELSE_STATEMENT_NODE_H