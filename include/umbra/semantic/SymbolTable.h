#pragma once

#include<optional>
#include<vector>
#include"umbra/semantic/SemanticType.h"
#include<string>
#include<unordered_map>

/**
 * @file SymbolTable.h
 * @brief Estructuras y API para gestionar símbolos y scopes durante el análisis semántico.
 * @details
 * Provee:
 * - Tipos ricos (Symbol, FunctionSignature) para variables y funciones.
 * - Tabla de símbolos con scopes anidados (pila de mapas nombre->símbolo).
 * - Inserción y búsqueda con reglas de visibilidad.
 */

/*
    En este archivo se encuentran las clases relacionalas a la tabla de simbolos.
    Se usa una estructura con informacion semantica rica para realizar un analisis mas agresivo
    y reducir la complejidad a la hora de generar codigo.
*/

namespace umbra{
    /**
     * @enum SymbolKind
     * @brief Clase de símbolo: variable o función.
     */
    enum class SymbolKind {
        VARIABLE,  ///< Símbolo de variable (local o global)
        FUCNTION   ///< Símbolo de función (nota: typo preservado para compatibilidad)
    };

    /**
     * @struct FunctionSignature
     * @brief Firma de función con retorno y tipos de parámetros.
     * @param isVarArg Indica si la función acepta número variable de argumentos.
     * @param returnType Tipo de retorno semántico.
     * @param argTypes Tipos de los parámetros formales en orden.
     */
    struct FunctionSignature {
        bool isVarArg = false;
        SemanticType returnType;
        std::vector<SemanticType> argTypes;
    };

    /**
     * @struct Symbol
     * @brief Representa una entrada en la tabla de símbolos.
     * @param type Tipo semántico principal (para variables) o redundante al retorno en funciones.
     * @param kind Clase del símbolo (variable/función).
     * @param signature Firma de función si aplica (vacía en variables).
     * @param line Línea de declaración (si se dispone).
     * @param col Columna de declaración (si se dispone).
     */
    struct Symbol{
        SemanticType type;
        SymbolKind kind;
        FunctionSignature signature;
        int line;
        int col;
    };

    /**
     * @class SymbolTable
     * @brief Tabla de símbolos con scopes anidados (estilo pila).
     * @details
     * - Cada scope es un unordered_map nombre->símbolo.
     * - Al entrar a un scope se apila un mapa vacío; al salir se desapila.
     * - lookup busca de adentro hacia afuera (scope actual hacia global).
     */
    class SymbolTable{

    public:

        /// Construye una tabla con al menos un scope global.
        SymbolTable();

        /// Entra a un nuevo scope (apila un mapa).
        void enterScope();
        /// Sale del scope actual (desapila el mapa más interno).
        void exitScope();
        /// Inserta o redefine un símbolo en el scope actual.
        void insert(const std::string& name, Symbol);
        /// Busca un símbolo por nombre desde el scope actual hacia los exteriores.
        Symbol lookup(const std::string& name) const;
        /// Devuelve el nivel de scope actual (0 = global).
        int getCurrentScopeLevel() const;

        /// Acceso de sólo lectura a todos los scopes, para depuración o reporte.
        std::vector<std::unordered_map<std::string, Symbol>> getScopes() {return scopes;}

    private:
        std::vector<std::unordered_map<std::string, Symbol>> scopes;
    };

}
