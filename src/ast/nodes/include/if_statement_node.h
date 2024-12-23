#ifndef IF_STATEMENT_NODE_H
#define IF_STATEMENT_NODE_H

#include <memory>
#include <vector>
#include "statement_node.h"
#include "expression_node.h"
#include "statement_list_node.h"

namespace umbra {

class IfStatementNode : public StatementNode {
public:
    IfStatementNode(
        std::unique_ptr<ExpressionNode> condition,
        std::unique_ptr<StatementListNode> ifStatements,
        std::vector<std::pair<std::unique_ptr<ExpressionNode>, std::unique_ptr<StatementListNode>>> elseIfBranches,
        std::unique_ptr<StatementListNode> elseStatements)
        : condition(std::move(condition))
        , ifStatements(std::move(ifStatements))
        , elseIfBranches(std::move(elseIfBranches))
        , elseStatements(std::move(elseStatements)) {}

    void accept(ASTVisitor& visitor) override {}

private:
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<StatementListNode> ifStatements;
    std::vector<std::pair<std::unique_ptr<ExpressionNode>, std::unique_ptr<StatementListNode>>> elseIfBranches;
    std::unique_ptr<StatementListNode> elseStatements;
};
} // namespace umbra

#endif // IF_STATEMENT_NODE_H