#include "umbra/semantic/TypeCk.h"
#include "umbra/semantic/SemanticType.h"
#include "umbra/ast/Nodes.h"
#include "umbra/semantic/SemanticAnalyzer.h"
#include "umbra/utils/utils.h"
#include "umbra/error/ErrorManager.h"
#include "umbra/error/CompilerError.h"

namespace umbra{

    SemanticType TypeCk::visitNumericLiteral(NumericLiteral* node){
        return builtinTypeToSemaType(node->builtinType);
    }

    SemanticType TypeCk::visitBinaryExpression(BinaryExpression* node){
        auto lType = visit(node->left.get());
        auto rType = visit(node->right.get());

        if (lType == SemanticType::Error || rType == SemanticType::Error) {
            return SemanticType::Error;
        }

        if (lType != rType) {
            if(errorManager) {
                std::string msg = "Type mismatch in binary expression: left side is '" +
                                  semanticTypeToString(lType) +
                                  "', right side is '" +
                                  semanticTypeToString(rType) + "'";
                errorManager->addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
            }
            return SemanticType::Error;
        }

        return lType;
    }

    SemanticType TypeCk::visitStringLiteral(StringLiteral* /*node */) {
        return SemanticType::String;
    }

    SemanticType TypeCk::visitPrimaryExpression(PrimaryExpression* node){
        if(!node) return SemanticType::Error;

        switch(node->exprType){
            case PrimaryExpression::IDENTIFIER:
                if(node->identifier) return visit(node->identifier.get());
                break;
            case PrimaryExpression::LITERAL:
                if(node->literal) return visit(node->literal.get());
                break;
            case PrimaryExpression::EXPRESSION_CALL:
                if(node->functionCall) return visit(node->functionCall.get());
                break;
            case PrimaryExpression::PARENTHESIZED:
                if(node->parenthesized) return visit(node->parenthesized.get());
                break;
            case PrimaryExpression::ARRAY_ACCESS:
                if(node->arrayAccess) return visit(node->arrayAccess.get());
                break;
            case PrimaryExpression::MEMBER_ACCESS:
                if(node->memberAccess) return visit(node->memberAccess.get());
                break;
            case PrimaryExpression::CAST_EXPRESSION:
                if(node->castExpression) return visit(node->castExpression.get());
                break;
            case PrimaryExpression::TERNARY_EXPRESSION:
                if(node->ternaryExpression) return visit(node->ternaryExpression.get());
                break;
        }
        return SemanticType::Error;
    }

    SemanticType TypeCk::visitFunctionCall(FunctionCall* node){
        // Las llamadas a función ya fueron validadas por SymbolCollector
        // y su tipo de retorno está almacenado en node->semaT
        if(node && node->semaT != SemanticType::None) {
            return node->semaT;
        }
        // Si no tiene tipo establecido, es un error
        if(errorManager) {
            std::string msg = "Function call type not resolved (internal error)";
            errorManager->addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
        }
        return SemanticType::Error;
    }

    SemanticType TypeCk::visitIdentifier(Identifier* node){
        auto sym = ctxt.symbolTable.lookup(node->name);
        if( sym.type != SemanticType::Error ) {
            node->semaT = sym.type;
            return node->semaT;
        }

        if(errorManager) {
            std::string msg = "Undefined variable '" + node->name + "' (variable may be out of scope or not declared)";
            errorManager->addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
        }

        node->semaT = SemanticType::Error;
        return SemanticType::Error;
    }

}
