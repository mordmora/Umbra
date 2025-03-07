#pragma once

#include <memory>
#include "../ast/Nodes.h"
#include <string>
namespace umbra{

class Symbol {
public:
    enum class SymbolKind {
        VARIABLE,
        FUNCTION,
        TYPE
    };

    Symbol(std::string name, SymbolKind kind, std::unique_ptr<Type> type);
    virtual ~Symbol() = default;

    std::string name;
    SymbolKind type;
};
 


}