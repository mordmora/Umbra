#pragma once

#include "StringInterner.h"
#include<memory>
#include "Symbol.h"
#include <unordered_map>


namespace umbra {
    using _SymbolMap = std::unordered_map<const std::string*, std::unique_ptr<Symbol>>;


/**
 * Tabla de símbolos para almacenar y gestionar todos los símbolos del programa
*/
class SymbolTable {
    public:
        SymbolTable(StringInterner& interner);
        
        // Añade un símbolo a la tabla
        bool addSymbol(std::unique_ptr<Symbol> symbol);
        
        // Obtiene un símbolo por su nombre
        Symbol* getSymbol(const std::string& name) const;
        
        // Elimina un símbolo de la tabla
        bool removeSymbol(const std::string* key);
    
        // Imprime todos los símbolos para depuración
        void printAllSymbols() const;
    
    private:
        StringInterner& stringInterner;
        _SymbolMap symbols;
    };
    
}