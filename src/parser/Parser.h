#ifndef PARSER_H
#define PARSER_H

#include "../ast/ASTNode.h"
#include "../ast/statementnodes/StatementNode.h"
#include "../lexer/Lexer.h"
#include <memory>
#include <vector>

namespace umbra {

class Parser {
  public:
    // Parser(Lexer &lexer);
    Parser(const std::vector<Lexer::Token> &tokens);
    Parser(const std::vector<Lexer::Token> &tokens, ErrorManager &externalErrorManager);

    std::unique_ptr<ASTNode> parse();

  private:
    // Lexer &lexer;
    std::vector<Lexer::Token> tokens;
    std::vector<Lexer::Token>::const_iterator current;
    ErrorManager *errorManager;
    Lexer::Token previousToken;

    bool match(TokenType type);
    bool check(TokenType type) const;
    Lexer::Token advance();
    Lexer::Token previous() const;
    Lexer::Token peek() const;
    bool isAtEnd() const;
    Lexer::Token consume(TokenType type, const std::string &message);

    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ExpressionNode> parseNumber(const Lexer::Token &token);
    std::unique_ptr<st_VariableDeclNode> parseVariableDeclaration();
    std::unique_ptr<FunctionDeclNode> parseFunctionDefinition();
    std::unique_ptr<ExpressionNode> parseExpression();
    TokenType parseTypeSpecifier();
    bool isTypeSpecifier(TokenType type) const;
    bool isTypeCompatible(TokenType declaredType, ExpressionNode *expr);
    std::string getExpressionTypeName(ExpressionNode *expr);
    std::string getTypeName(TokenType expr);
};

} // namespace umbra

#endif // PARSER_H