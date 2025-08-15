#pragma once
#include "umbra/semantic/SymbolTable.h"
#include "umbra/ast/Nodes.h"
#include "umbra/semantic/SemanticType.h"

/**
 * @file SemanticContext.h
 * @brief Contexto semántico en tiempo de análisis.
 * @details
 * Mantiene referencias al estado semántico compartido durante el análisis:
 * - Tabla de símbolos (con scopes anidados).
 * - Mapa de tipos inferidos por expresión (Expression* -> SemanticType).
 * Provee utilidades para manejar scopes de manera coherente con SymbolTable.
 */

/*
    En esta clase se maneja el contexto semantico. Los simbolos actuales, los alcances y
    los tipos deducidos de las expresiones.
*/

namespace umbra {

    /**
     * @struct SemanticContext
     * @brief Pequeño contenedor de estado semántico para el pipeline.
     */
    struct SemanticContext{
        /// Referencia a la tabla de símbolos activa (global y locales).
        SymbolTable& symbolTable;
        /// Tipos inferidos por expresión durante el análisis.
        std::unordered_map<Expression*, SemanticType> exprTypes;

        /**
         * @brief Construye un contexto semántico asociado a una SymbolTable.
         * @param symTable Tabla de símbolos a utilizar (no se toma propiedad).
         */
        SemanticContext(SymbolTable& symTable) : symbolTable(symTable) {}

        /**
         * @brief Entra a un nuevo scope (bloque/función), delegando en SymbolTable.
         */
        void enterScope();
        /**
         * @brief Sale del scope actual, delegando en SymbolTable.
         */
        void exitScope();
        /**
         * @brief Obtiene la profundidad de scope actual.
         * @return Nivel actual de anidamiento (0 = global).
         */
        int getCurrentScopeLevel() const;
        //void reportError();
    };


};
