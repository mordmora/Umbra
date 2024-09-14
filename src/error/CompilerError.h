#ifndef UMBRA_COMPILER_ERROR_H
#define UMBRA_COMPILER_ERROR_H

#include "ErrorTypes.h"
#include <string>

namespace umbra {

class CompilerError {
  public:
    CompilerError(ErrorType type, const std::string &message, int line, int column)
        : type(type), message(message), line(line), column(column) {}

    virtual ~CompilerError() = default;

    virtual std::string toString() const;
    ErrorType getType() const { return type; }
    int getLine() const { return line; }
    int getColumn() const { return column; }

  protected:
    std::string getErrorTypeString() const;

    ErrorType type;
    std::string message;
    int line;
    int column;
};

class LexicalError : public CompilerError {
  public:
    LexicalError(const std::string &message, int line, int column, char problematic_char)
        : CompilerError(ErrorType::LEXICAL, message, line, column),
          problematic_char(problematic_char) {}

    std::string toString() const override;

  private:
    char problematic_char;
};

} // namespace umbra

#endif // UMBRA_COMPILER_ERROR_H