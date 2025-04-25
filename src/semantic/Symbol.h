#pragma once 

#include "../ast/Nodes.h"
#include "StringInterner.h"

/**
 * Clase que representa un símbolo en el programa (variable, función o tipo)
 */

namespace umbra{
    
    class Symbol {
    public:
        enum class SymbolKind {
            VARIABLE,
            FUNCTION,
            TYPE
        };

        std::vector<std::unique_ptr<Type>> paramTypes;
    
        Symbol(std::string name, SymbolKind kind, std::unique_ptr<Type> type, StringInterner& interner);
        virtual ~Symbol() = default;
    
        std::unique_ptr<Type> type;
    
        const std::string& name;
        SymbolKind kind;
    };
}