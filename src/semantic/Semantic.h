#pragma once

#include <memory>
#include "../ast/Nodes.h"
#include <string>
#include <unordered_set>
#include <unordered_map>
namespace umbra{

class Symbol;

typedef std::unordered_map<const std::string*, std::unique_ptr<Symbol>> _SymbolMap;

class StringInterner {
    public:
        StringInterner();
        const std::string& intern(const std::string& str);
        const std::string* get(const std::string& str) const;

    private:
        std::unordered_set<std::string> internedStrings;
};

class Symbol {
public:
    enum class SymbolKind {
        VARIABLE,
        FUNCTION,
        TYPE
    };

    Symbol(std::string name, SymbolKind kind, std::unique_ptr<Type> type, StringInterner &interner);
    virtual ~Symbol() = default;

    const std::string& name;
    SymbolKind kind;
    std::unique_ptr<Type> type;
};

class ScopeManager {
public:
    ScopeManager();
    void enterScope(_SymbolMap sym);
    void exitScope();

private:
    std::vector<_SymbolMap> scopes;
};

class SymbolTable {
    public:
        SymbolTable(StringInterner &interner);
        bool addSymbol(std::unique_ptr<Symbol> symbol);
        std::unique_ptr<Symbol> getSymbol(std::string name);
    private:
    StringInterner &stringInterner;
    _SymbolMap symbols;
    };

}