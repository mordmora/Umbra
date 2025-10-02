#include "umbra/semantic/SymbolTable.h"
#include <assert.h>
#include <stdexcept>
#include "umbra/semantic/SemanticType.h"

#include<iostream>

namespace umbra {

    SymbolTable::SymbolTable() {
        scopes.emplace_back();
    }

    void SymbolTable::enterScope(){
        scopes.emplace_back();
    }

    void SymbolTable::exitScope(){
        assert(!scopes.empty());
        if (scopes.size() > 1) {
            scopes.pop_back();
        }
    }

    int SymbolTable::getCurrentScopeLevel() const {
        return scopes.size() - 1;
    }

    void SymbolTable::insert(const std::string& name, Symbol symbol){
        if (scopes.empty()) {
            throw std::runtime_error("No hay scopes disponibles para insertar simbolo: " + name);
        }


        if(scopes.back().count(name)){
            throw std::runtime_error("Redefinicion de simbolo: " + name);
        }
        scopes.back()[name] = symbol;

    }

    Symbol SymbolTable::lookup(const std::string& name) const {
        for(auto it = scopes.rbegin(); it != scopes.rend(); ++it ){
            auto found = it->find(name);
            if(found != it->end()){
                return found->second;
            }
        }
        return Symbol{SemanticType::Error, SymbolKind::VARIABLE, {}, 0, 0};
    }
}
