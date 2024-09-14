#include "CompilerError.h"
#include <sstream>

namespace umbra {

std::string CompilerError::toString() const {
    std::ostringstream oss;
    oss << "\033[31m" // Iniciar color rojo
        << getErrorTypeString() << " error at line " << line << ", column " << column << ": "
        << "\033[0m" // Restablecer color
        << message;
    return oss.str();
}

std::string CompilerError::getErrorTypeString() const {
    switch (type) {
    case ErrorType::LEXICAL:
        return "Lexical";
    case ErrorType::SYNTACTIC:
        return "Syntactic";
    case ErrorType::SEMANTIC:
        return "Semantic";
    default:
        return "Unknown";
    }
}

std::string LexicalError::toString() const {
    std::ostringstream oss;
    oss << CompilerError::toString() << " ('" << problematic_char << "')";
    return oss.str();
}

} // namespace umbra