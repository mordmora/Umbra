#pragma once 

#include "SymbolTable.h"

namespace umbra {



/**
 * Clase que gestiona la pila de ámbitos de visibilidad durante la compilación
 */
class ScopeManager {
    public:
        ScopeManager();
        
        // Entra en un nuevo ámbito de visibilidad
        void enterScope(_SymbolMap sym);
        
        // Sale del ámbito actual y elimina sus símbolos
        void exitScope(SymbolTable& symbolTable);
        
        // Obtiene el ámbito actual
        _SymbolMap& currentScope() {
            return scopes.back();
        }
    
    private:
        std::vector<_SymbolMap> scopes;
    };
    


    
}