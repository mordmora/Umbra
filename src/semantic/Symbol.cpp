
#include "Symbol.h"

namespace umbra {
    Symbol::Symbol(std::string name, Symbol::SymbolKind kind,
        std::unique_ptr<Type> type, 
        StringInterner &interner) 
        : 
        type(std::move(type)), 
        name(interner.intern(name)), 
        kind(kind) 
    {
        paramTypes = std::vector<std::unique_ptr<Type>>();
    }
    
}