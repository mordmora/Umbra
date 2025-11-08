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
        static int recursionDepth = 0;
        if(!node) return SemanticType::Error;

        // Protección contra recursión infinita
        if(++recursionDepth > 1000) {
            if(errorManager) {
                std::string msg = "Internal error: infinite recursion detected in type checking";
                errorManager->addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
            }
            --recursionDepth;
            return SemanticType::Error;
        }

        SemanticType result = SemanticType::Error;
        switch(node->exprType){
            case PrimaryExpression::IDENTIFIER:
                if(node->identifier) result = visit(node->identifier.get());
                break;
            case PrimaryExpression::LITERAL:
                if(node->literal) result = visit(node->literal.get());
                break;
            case PrimaryExpression::EXPRESSION_CALL:
                if(node->functionCall) result = visit(node->functionCall.get());
                break;
            case PrimaryExpression::PARENTHESIZED:
                if(node->parenthesized) result = visit(node->parenthesized.get());
                break;
            case PrimaryExpression::ARRAY_ACCESS:
                if(node->arrayAccess) result = visit(node->arrayAccess.get());
                break;
            case PrimaryExpression::MEMBER_ACCESS:
                if(node->memberAccess) result = visit(node->memberAccess.get());
                break;
            case PrimaryExpression::CAST_EXPRESSION:
                if(node->castExpression) result = visit(node->castExpression.get());
                break;
            case PrimaryExpression::TERNARY_EXPRESSION:
                if(node->ternaryExpression) result = visit(node->ternaryExpression.get());
                break;
        }

        --recursionDepth;
        return result;
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

    SemanticType TypeCk::visitArrayAccessExpression(ArrayAccessExpression* node){
        SemanticType arrayType = visit(node->array.get());
        
        if(arrayType == SemanticType::Error){
            return SemanticType::Error;
        }
        
        SemanticType indexType = visit(node->index.get());
        if(indexType != SemanticType::Int){
            if(errorManager){
                std::string msg = "Array index must be of type Int, got " + semanticTypeToString(indexType);
                errorManager->addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
            }
            return SemanticType::Error;
        }
        
        return arrayType;
    }

    SemanticType TypeCk::visitIncrementExpression(IncrementExpression* node){
        SemanticType operandType = visit(node->operand.get());
        
        if(operandType == SemanticType::Error){
            return SemanticType::Error;
        }
        
        if(operandType != SemanticType::Int && operandType != SemanticType::Float){
            if(errorManager){
                std::string msg = "Increment operator requires numeric type (Int or Float), got " + semanticTypeToString(operandType);
                errorManager->addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
            }
            return SemanticType::Error;
        }
        
        return operandType;
    }

    SemanticType TypeCk::visitDecrementExpression(DecrementExpression* node){
        SemanticType operandType = visit(node->operand.get());
        
        if(operandType == SemanticType::Error){
            return SemanticType::Error;
        }
        
        if(operandType != SemanticType::Int && operandType != SemanticType::Float){
            if(errorManager){
                std::string msg = "Decrement operator requires numeric type (Int or Float), got " + semanticTypeToString(operandType);
                errorManager->addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
            }
            return SemanticType::Error;
        }
        
        return operandType;
    }

}
