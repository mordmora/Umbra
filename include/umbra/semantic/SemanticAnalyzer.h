// --------------------------------------------------------------
// - Se agregó TypeCk al constructor y se lo pasa a SymbolCollector
// - convertBuiltinToSemanticType ahora es inline en el header (sin impl duplicada)
// - Ajuste en orden de miembros para inicializar collector antes de campos usados
// --------------------------------------------------------------

/**
 * @file SemanticAnalyzer.h
 * @brief Orquestador del análisis semántico para el compilador Umbra.
 * @details
 * Ejecuta el pipeline de análisis semántico sobre el AST:
 * - Inicializa la tabla de símbolos y el contexto semántico (scopes, tipos de expresiones).
 * - Recolecta símbolos (funciones, parámetros, variables) y valida llamadas mediante SymbolCollector.
 * - Usa TypeCk para inferir/verificar tipos en expresiones (RHS, argumentos, retornos, etc.).
 * - Reporta errores a través de ErrorManager y evita lanzar excepciones.
 */

/*
    En esta clase se maneja el pipeline de analisis semantico.
    La clase SemanticAnalyzer es el orquestador que va llamando fase por fase y haciendo las
    respectivas comprobaciones.

*/
#pragma once
#include "umbra/ast/Nodes.h"
#include "umbra/semantic/SemanticContext.h"
#include "umbra/semantic/SymbolTable.h"
#include "umbra/semantic/SemanticType.h"
#include "umbra/semantic/SymbolCollector.h"
#include "umbra/error/ErrorManager.h"
#include "umbra/semantic/TypeCk.h"

namespace umbra {

    /**
     * @class SemanticAnalyzer
     * @brief Gestiona la ejecución del pipeline semántico sobre un AST Umbra.
     * @details
     * Responsabilidades principales:
     * - Crear y mantener la tabla de símbolos (SymbolTable) y el contexto semántico (SemanticContext).
     * - Invocar al recolector de símbolos (SymbolCollector) para registrar builtins y definiciones del usuario.
     * - Coordinar verificaciones de tipos usando TypeCk.
     * - Notificar errores al ErrorManager suministrado por el llamador.
     */
    class SemanticAnalyzer  {
        public:
            /**
             * @brief Construye el analizador semántico.
             * @param errManager Referencia al gestor de errores donde se reportarán los fallos.
             * @param root Puntero al nodo raíz del programa (AST). No debe ser nulo.
             * @note No toma propiedad de root ni de errManager.
             */
            SemanticAnalyzer(ErrorManager& errManager, ProgramNode* root)
                : context(symTable),
                  typeCk(context, &errManager),
                  errorManager(errManager),
                  rootASTNode(root),
                  collector(context, symTable, root, typeCk, errorManager)
            {
            }

            /**
             * @brief Ejecuta la tubería de análisis semántico completa.
             * @details
             * Pasos típicos:
             * 1) Registrar builtins y recolectar símbolos de nivel superior.
             * 2) Validar puntos de entrada y llamadas a funciones.
             * 3) Inferir/verificar tipos en expresiones mediante TypeCk.
             * Los errores encontrados se agregan al ErrorManager.
             */
            void execAnalysisPipeline();

        private:
            /// Tabla de símbolos con soporte de scopes (global/local) y firmas de funciones.
            SymbolTable symTable;        // Debe inicializarse primero
            /// Contexto semántico asociado a la tabla de símbolos (scopes y tipos de expresiones).
            SemanticContext context;     // Luego este que usa symTable
            /// Comprobador/inferecedor de tipos para expresiones (lado derecho, args, etc.).
            TypeCk typeCk;
            /// Gestor de errores compartido con el resto del compilador.
            ErrorManager& errorManager;
            /// Nodo raíz del AST del programa a analizar.
            ProgramNode* rootASTNode;
            /// Visitante que recolecta símbolos, registra builtins y valida llamadas.
            SymbolCollector collector;
            /// Último tipo inferido/reportado por ciertas operaciones (puede usarse como caché o estado temporal).
            SemanticType lastType = SemanticType::None;
    };

}
