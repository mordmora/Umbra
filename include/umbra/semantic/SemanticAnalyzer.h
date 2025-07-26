// --------------------------------------------------------------
// - Se agregó TypeCk al constructor y se lo pasa a SymbolCollector
// - convertBuiltinToSemanticType ahora es inline en el header (sin impl duplicada)
// - Ajuste en orden de miembros para inicializar collector antes de campos usados
// --------------------------------------------------------------
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
                  collector(context, symTable, rootASTNode, typeCk)
            {
            }

            void execAnalysisPipeline();

        private:
            SymbolCollector collector;
            TypeCk typeCk;
            SymbolTable symTable;
            SemanticType lastType = SemanticType::NONE;
            SemanticContext context;
            ErrorManager& errorManager;
            ProgramNode* rootASTNode;
    };

    inline SemanticType convertBuiltinToSemanticType(BuiltinType builtinType) {
        switch (builtinType) {
            case BuiltinType::Int:
                return SemanticType::INTEGER;
            case BuiltinType::Float:
                return SemanticType::FLOAT;
            case BuiltinType::String:
                return SemanticType::STRING;
            case BuiltinType::Bool:
                return SemanticType::BOOLEAN;
            case BuiltinType::Char:
                return SemanticType::CHAR;
            case BuiltinType::Void:
                return SemanticType::NONE;
            default:
                return SemanticType::ERROR;
        }
    }

}
