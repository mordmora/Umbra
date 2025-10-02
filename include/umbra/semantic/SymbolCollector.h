#pragma once

#include <memory>
#include "umbra/ast/Nodes.h"
#include "umbra/semantic/SemanticContext.h"
#include "umbra/semantic/SymbolTable.h"
#include "umbra/ast/Visitor.h"
#include "umbra/semantic/TypeCk.h"
#include "umbra/error/ErrorManager.h"
#include "umbra/error/CompilerError.h"

/**
 * @file SymbolCollector.h
 * @brief Primera fase del análisis semántico: recolección de símbolos y validación básica.
 * @details
 * Este visitante recorre el AST para:
 * - Registrar símbolos de funciones y variables en la SymbolTable (manejo de scopes via SemanticContext).
 * - Construir y adjuntar firmas de funciones (FunctionSignature) a los nodos de función.
 * - Insertar builtins (p. ej., print variádica) en el ámbito global.
 * - Validar llamadas a funciones (existencia, número/tipos de argumentos; caso especial print).
 * - Extraer/inferir tipos de argumentos usando TypeCk.
 *
 * Los errores semánticos se reportan a través de ErrorManager; no se lanzan excepciones.
 */
/*
    Primera fase de analisis semantico: Recoleccion de simbolos

    Esta clase tiene como objetivo buscar y registrar todos los simbolos presentes en un programa
    ej: Declaraciones de variables, definiciones de funciones, etc.
*/


namespace umbra {
    template<template <typename> class Ptr, typename ImplClass, typename RetTy, class... ParamTys>
    class BaseV;
}

namespace umbra {

    /**
     * @class SymbolCollector
     * @brief Visitante del AST que recolecta símbolos y valida llamadas a funciones.
     * @details
     * Responsabilidades:
     * - Registrar builtins en el scope global.
     * - Declarar funciones del usuario, construir sus firmas y registrar parámetros como variables.
     * - Gestionar entrada/salida de scopes mediante SemanticContext.
     * - Validar llamadas a función y propagar tipos/firmas al AST.
     */
    class SymbolCollector : public BaseV<std::unique_ptr, SymbolCollector, void> {

        public:
        /**
         * @brief Crea un recolector de símbolos.
         * @param theContext Contexto semántico compartido (scopes, tipos por expresión).
         * @param symTable Tabla de símbolos a utilizar.
         * @param node Nodo raíz del programa (AST) a recorrer.
         * @param typeCk Visitante para inferencia/comprobación de tipos de expresiones.
         * @param errorManager Gestor de errores para reportar problemas semánticos.
         */
        SymbolCollector(SemanticContext& theContext, SymbolTable& symTable, ProgramNode* node, TypeCk typeCk, ErrorManager& errorManager)
            : rootASTNode(node), theContext(theContext), symTable(symTable), typeCk(typeCk), errorManager(errorManager) {}

        /// @return Referencia a la tabla de símbolos administrada por el recolector.
        SymbolTable& getSymbolTable() { return symTable; }
        /// @return Referencia al contexto semántico en uso.
        SemanticContext& getTheContext() { return theContext; }

        /**
         * @brief Punto de entrada: visita el programa, registra builtins y valida el entry point.
         */
        void visitProgramNode(ProgramNode* node);

        /**
         * @brief Declara una función en el scope global y procesa su cuerpo (abre/cierra scope).
         */
        void visitFunctionDefinition(FunctionDefinition* node);

        /**
         * @brief Inserta una variable en el scope actual y valida su inicializador (si existe).
         */
        void visitVariableDeclaration(VariableDeclaration* node);

        void visitAssignmentStatement(AssignmentStatement* node);

        /**
         * @brief Visita directa de una llamada a función (si aparece aislada) para validarla.
         */
        void visitFunctionCall(FunctionCall* node);

        /**
         * @brief Si el PrimaryExpression contiene una llamada, la valida.
         */
        void visitPrimaryExpression(PrimaryExpression* node);

        /**
         * @brief Valida llamadas que aparecen como statement (expresión suelta).
         */
        void visitExpressionStatement(ExpressionStatement* node);

        /**
         * @brief Visita statements if/elseif/else, validando condiciones y cuerpos.
         */
        void visitIfStatement(IfStatement* node);

        /**
         * @brief Visita statements repeat times, validando la expresión de veces y el cuerpo.
         */
        void visitRepeatTimesStatement(RepeatTimesStatement* node);

        /**
         * @brief Visita statements repeat if (while), validando la condición y el cuerpo.
         */
        void visitRepeatIfStatement(RepeatIfStatement* node);

        /**
         * @brief Valida semánticamente una llamada a función y propaga tipos al nodo.
         * @param node Nodo de llamada.
         * @return true si la llamada es válida; false en caso contrario.
         */
        bool validateFunctionCall(FunctionCall* node);

        /**
         * @brief Valida recursivamente todas las llamadas a función dentro de una expresión.
         * @param expr Expresión a analizar (puede contener llamadas anidadas).
         */
        void validateCallsInExpression(Expression* expr);

        /**
         * @brief Convierte los argumentos de una llamada a su SemanticType usando TypeCk.
         * @param arguments Vector de expresiones de argumentos.
         * @return Tipos inferidos en el mismo orden.
         */
        std::vector<SemanticType> extractArgumentTypes(const std::vector<std::unique_ptr<Expression>>& arguments);

        /**
         * @brief Imprime por consola los símbolos recolectados, agrupados por scope (depuración).
         */
        void printCollectedSymbols() const;

        private:

        /**
         * @brief Registra símbolos builtin en el scope global (p.ej., print variádica).
         */
        void registerBuiltins();
        /**
         * @brief Valida el punto de entrada del programa (start() -> void/int sin params).
         */
        void validateEntryPoint();

        /// Nodo raíz del programa.
        ProgramNode* rootASTNode;
        /// Contexto semántico compartido (gestiona scopes y tipos por expresión).
        SemanticContext& theContext;
        /// Tabla de símbolos activa.
        SymbolTable& symTable;
        /// Comprobador/inferecedor de tipos.
        TypeCk typeCk;
        /// Gestor de errores para reportar problemas semánticos.
        ErrorManager& errorManager;

   };

}
