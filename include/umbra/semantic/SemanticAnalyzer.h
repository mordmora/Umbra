// --------------------------------------------------------------
// - Se agregó TypeCk al constructor y se lo pasa a SymbolCollector
// - convertBuiltinToSemanticType ahora es inline en el header (sin impl duplicada)
// - Ajuste en orden de miembros para inicializar collector antes de campos usados
// --------------------------------------------------------------

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

    class SemanticAnalyzer  {
        public:

            SemanticAnalyzer(ErrorManager& errManager, ProgramNode* root)
                : errorManager(errManager),
                  rootASTNode(root),
                  context(symTable),
                  collector(context, symTable, rootASTNode, typeCk, errorManager)
            {
            }

            void execAnalysisPipeline();


        private:
            SymbolTable symTable;        // Debe inicializarse primero
            SemanticContext context;     // Luego este que usa symTable
            SymbolCollector collector;
            TypeCk typeCk;
            SemanticType lastType = SemanticType::None;
            ErrorManager& errorManager;
            ProgramNode* rootASTNode;
    };

}
