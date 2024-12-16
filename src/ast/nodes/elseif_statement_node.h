#ifndef ELSEIF_STATEMENT_NODE_H
#define ELSEIF_STATEMENT_NODE_H

#include <memory>
#include "statement_node.h"
#include "expression_node.h"
#include "statement_list_node.h"

namespace umbra {

class ElseIfStatementNode : public StatementNode {
public:
    ElseIfStatementNode(
        std::unique_ptr<ExpressionNode> condition,
        std::unique_ptr<StatementListNode> statements)
        : condition(std::move(condition)),
          statements(std::move(statements)) {}

    const ExpressionNode* getCondition() const { return condition.get(); }
    const StatementListNode* getStatements() const { return statements.get(); }

    void accept(ASTVisitor& visitor) override {
        // Implementación del método accept
    }

private:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<StatementListNode> statements;
};

}; // namespace umbra

#endif // ELSEIF_STATEMENT_NODE_H