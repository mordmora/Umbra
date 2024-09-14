#ifndef UMBRA_ERROR_TYPES_H
#define UMBRA_ERROR_TYPES_H

namespace umbra {

enum class ErrorType {
    // basic error types
    LEXICAL,
    SYNTACTIC,
    SEMANTIC
};

} // namespace umbra

#endif // UMBRA_ERROR_TYPES_H