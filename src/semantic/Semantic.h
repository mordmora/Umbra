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
    SymbolKind kind;
    std::unique_ptr<Type> type;
};

class ScopeManager {
public:
    ScopeManager();
    void enterScope();
    void exitScope();
};

class SymbolTable {
    public:
        SymbolTable();
        void addSymbol(std::unique_ptr<Symbol> symbol);
        std::unique_ptr<Symbol> getSymbol(std::string name);
    private:
        std::vector<std::unique_ptr<Symbol>> symbols;
    };

}