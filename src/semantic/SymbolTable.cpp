#include "SymbolTable.h"

namespace umbra {
    
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
                  << ", Type: " << static_cast<int>(pair.second->type->builtinType)
                  << std::endl;
    }
}


}