#include "Semantic.h"
#include "../ast/Nodes.h"

namespace umbra{
    
    Symbol::Symbol(std::string name, Symbol::SymbolKind kind, std::unique_ptr<Type> type) : name(name), kind(kind), type(std::move(type)) {}
}