#pragma once

#include <memory>
#include "../ast/Nodes.h"
#include <string>
#include <unordered_set>
#include <unordered_map>

namespace umbra {

enum class RvalExpressionType {
    INTEGER,
    FLOAT,
    STRING,
    BOOLEAN,
    CHAR,
    VAR_NAME
};

// Forward declarations
class Symbol;
class SymbolTable;

// Define el tipo de mapa utilizado para almacenar símbolos
using _SymbolMap = std::unordered_map<const std::string*, std::unique_ptr<Symbol>>;

/**
 * Clase que interna cadenas para optimizar el uso de memoria 
 * y mejorar la eficiencia de comparaciones
 */
class StringInterner {
public:
    StringInterner();
    
    // Interna una cadena y devuelve una referencia a la versión internada
    const std::string& intern(const std::string& str);
    
    // Obtiene un puntero a la cadena internada, o nullptr si no existe
    const std::string* get(const std::string& str) const;

private:
    std::unordered_set<std::string> internedStrings;
};

/**
 * Clase que representa un símbolo en el programa (variable, función o tipo)
 */
class Symbol {
public:
    enum class SymbolKind {
        VARIABLE,
        FUNCTION,
        TYPE
    };

    Symbol(std::string name, SymbolKind kind, std::unique_ptr<Type> type, StringInterner& interner);
    virtual ~Symbol() = default;

    std::unique_ptr<Type> type;

    const std::string& name;
    SymbolKind kind;
};

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

/**
 * Clase estática que verifica la compatibilidad entre tipos
 */
class TypeCompatibility {
public:
    // Verifica si el tipo de expresión es compatible con el tipo declarado
    static bool areTypesCompatible(const Type& declType, RvalExpressionType exprType);
};

} // namespace umbra