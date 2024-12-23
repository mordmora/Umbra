#ifndef LEXER_H
#define LEXER_H

#include "../error/ErrorManager.h"
#include "Tokens.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace umbra {

class Lexer {
  public:
    struct Token {
        TokenType type;
        std::string lexeme;
        int line;
        int column;

        Token(TokenType t, std::string l, int ln, int col)
            : type(t), lexeme(std::move(l)), line(ln), column(col) {}
    };
    TokenManager tokenManager;

    Lexer(const std::string &source);
    Lexer(const std::string &source, ErrorManager &externalErrorManager);
    std::vector<Token> tokenize();
    const ErrorManager &getErrorManager() const { return *errorManager; }
    void reset();
    Token peekToken();
    Token getNextToken();
    std::string getSource();
  private:
    std::string source;
    std::unique_ptr<ErrorManager> internalErrorManager;
    ErrorManager *errorManager;
    std::vector<Token> tokens;
    char lastChar = ' ';
    enum class State { Start, Integer, Decimal, NotationNumber, Acceptance, Rejection };
    // std::unordered_map<std::string, TokenType> keywords;
    int current = 0;
    int line = 1;
    int start = 0;
    int column = 1;
    State state=State::Start;

    char advance();
    bool isAtEnd() const;
    char peek() const;
    char peekNext() const;
    bool match(char expected);
    void addToken(TokenType type);
    void addToken(TokenType type, const std::string &lexeme);
    void string();
    void number();
    void charliteral();
    void identifier();
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;
    bool isDigit(char c) const;
    bool isSing(char c) const;
    bool isWhitespace(char c) const;
    //void scanToken();
    //void handleIdentifierOrKeyword();
    bool isBlankSpace(char c) const;
    bool isBinary(char c) ;
    void errorMessage(char c); // error unexpected character c
};

} // namespace umbra

#endif // LEXER_H