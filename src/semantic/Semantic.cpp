#include "Semantic.h"
#include "../ast/Nodes.h"
#include <iostream>

namespace umbra{

    StringInterner::StringInterner(){}

    const std::string& 
    StringInterner::intern(const std::string& str){
        auto it = internedStrings.insert(str);
        return *it.first;
    }

    const std::string* 
    StringInterner::get(const std::string& str) const {
        auto it = internedStrings.find(str);
        return (it != internedStrings.end()) ? &(*it) : nullptr;
    }
    
    Symbol::Symbol(std::string name, Symbol::SymbolKind kind,
        std::unique_ptr<Type> type, 
        StringInterner &interner) 
        : 
        name(interner.intern(name)), 
        kind(kind), type(std::move(type)) 
        {}

    SymbolTable::SymbolTable(StringInterner &stringInterner) 
    : 
    stringInterner(stringInterner){}

    bool 
    SymbolTable::addSymbol(std::unique_ptr<Symbol> symbol){
        const std::string* key = &symbol->name;
        auto [it, inserted] = symbols.emplace(std::make_pair(key, std::move(symbol)));
        return inserted;
    }

    bool SymbolTable::removeSymbol(const std::string* key) {
        return symbols.erase(key) > 0;
    }

    Symbol* SymbolTable::getSymbol(const std::string& name) const {
        auto key = stringInterner.get(name);
        auto it = symbols.find(key);
        return (it != symbols.end()) ? it->second.get() : nullptr;
    }

    ScopeManager::ScopeManager() {}

    void ScopeManager::enterScope(_SymbolMap sym){
        scopes.push_back(std::move(sym));
    }

    void ScopeManager::exitScope(SymbolTable &symbolTable){
        std::cout << "Exiting scope" << std::endl;
        _SymbolMap scopeSymbols = std::move(scopes.back());
        scopes.pop_back();
        
        for (auto &entry : scopeSymbols) {
            symbolTable.removeSymbol(entry.first);
        }
    }

}