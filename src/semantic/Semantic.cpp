#include "Semantic.h"
#include "../ast/Nodes.h"
#include <iostream>

namespace umbra {

StringInterner::StringInterner() {}

const std::string& 
StringInterner::intern(const std::string& str) {
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
    type(std::move(type)), 
    name(interner.intern(name)), 
    kind(kind) 
{}

SymbolTable::SymbolTable(StringInterner &stringInterner) 
: 
stringInterner(stringInterner) {}

bool 
SymbolTable::addSymbol(std::unique_ptr<Symbol> symbol) {
    const std::string* key = &symbol->name;
    auto [it, inserted] = symbols.emplace(key, std::move(symbol));
    return inserted;
}

bool 
SymbolTable::removeSymbol(const std::string* key) {
    return symbols.erase(key) > 0;
}

Symbol* 
SymbolTable::getSymbol(const std::string& name) const {
    auto key = stringInterner.get(name);
    if (!key) return nullptr;
    
    auto it = symbols.find(key);
    return (it != symbols.end()) ? it->second.get() : nullptr;
}

void 
SymbolTable::printAllSymbols() const {
    std::cout << "All symbols in the symbol table:" << std::endl;
    for (const auto& pair : symbols) {
        std::cout << "Symbol: " << *pair.first 
                  << ", Type: " << pair.second->type->baseType 
                  << std::endl;
    }
}

ScopeManager::ScopeManager() {}

void 
ScopeManager::enterScope(_SymbolMap sym) {
    scopes.push_back(std::move(sym));
}

void 
ScopeManager::exitScope(SymbolTable &symbolTable) {
    if (scopes.empty()) {
        std::cerr << "Error: Attempting to exit from empty scope stack" << std::endl;
        return;
    }
    
    std::cout << "Exiting scope" << std::endl;
    _SymbolMap scopeSymbols = std::move(scopes.back());
    scopes.pop_back();
    
    for (auto &entry : scopeSymbols) {
        symbolTable.removeSymbol(entry.first);
    }
}

bool 
TypeCompatibility::areTypesCompatible(const Type& declType, RvalExpressionType exprType) {
    // Verificación directa de tipos
    if (declType.baseType == "int" && exprType == RvalExpressionType::INTEGER) return true;
    if (declType.baseType == "float" && exprType == RvalExpressionType::FLOAT) return true;
    if (declType.baseType == "string" && exprType == RvalExpressionType::STRING) return true;
    if (declType.baseType == "bool" && exprType == RvalExpressionType::BOOLEAN) return true;
    if (declType.baseType == "char" && exprType == RvalExpressionType::CHAR) return true;
        
    // Conversiones implícitas permitidas
    if (declType.baseType == "float" && exprType == RvalExpressionType::INTEGER) return true;
    
    return false;
}

} // namespace umbra