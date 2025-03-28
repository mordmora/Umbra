#include "Semantic.h"
#include "../ast/Nodes.h"

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

    std::unique_ptr<Symbol>
    SymbolTable::getSymbol(std::string name){
        auto it = symbols.find(stringInterner.get(name));
        return (it != symbols.end()) ? std::move(it->second) : nullptr;
    }

    ScopeManager::ScopeManager() {}

    void ScopeManager::enterScope(_SymbolMap sym){
        scopes.push_back(sym);
    }

    void ScopeManager::exitScope(){
        scopes.pop_back();
    }

    



}