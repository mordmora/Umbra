#ifndef IF_STATEMENT_NODE_H
#define IF_STATEMENT_NODE_H

#include <memory>
#include <vector>
#include "statement_node.h"
#include "expression_node.h"
#include "statement_list_node.h"

namespace umbra {

class ElseIfStatementNode; // Declaración adelantada

class IfStatementNode : public StatementNode {
public:
    IfStatementNode(
        std::unique_ptr<ExpressionNode> condition,
        std::unique_ptr<StatementListNode> thenStatements,
        std::vector<std::unique_ptr<ElseIfStatementNode>> elseIfClauses,
        std::unique_ptr<StatementListNode> elseStatements = nullptr)
        : condition(std::move(condition)),
          thenStatements(std::move(thenStatements)),
          elseIfClauses(std::move(elseIfClauses)),
          elseStatements(std::move(elseStatements)) {}

    const ExpressionNode* getCondition() const { return condition.get(); }
    const StatementListNode* getThenStatements() const { return thenStatements.get(); }
    const std::vector<std::unique_ptr<ElseIfStatementNode>>& getElseIfClauses() const { return elseIfClauses; }
    const StatementListNode* getElseStatements() const { return elseStatements.get(); }

    void accept(ASTVisitor& visitor) override;

private:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<StatementListNode> thenStatements;
    std::vector<std::unique_ptr<ElseIfStatementNode>> elseIfClauses;
    std::unique_ptr<StatementListNode> elseStatements;
};

} // namespace umbra

#endif // IF_STATEMENT_NODE_H