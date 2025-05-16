#include "ScopeManager.h"

namespace umbra {
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
    
    _SymbolMap scopeSymbols = std::move(scopes.back());
    scopes.pop_back();
    
    for (auto &entry : scopeSymbols) {
        symbolTable.removeSymbol(entry.first);
    }
}

}
