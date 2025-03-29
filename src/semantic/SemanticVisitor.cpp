#include "SemanticVisitor.h"
#include "../error/CompilerError.h"
#include <iostream>
namespace umbra{

    SemanticVisitor::SemanticVisitor(StringInterner& interner, ScopeManager& scopeManager, ErrorManager& errorManager) 
    : 
    interner(interner), 
    symbolTable(interner),
    scopeManager(scopeManager),
    errorManager(errorManager) {}

    void 
    ProgramChecker::visit(ProgramNode& node){
        std::cout << "Visiting program node" << std::endl;
        for(auto& child : node.functions){
            child->accept(*this);
        }
    }

    void 
    ProgramChecker::visit(ParameterList& node){

        std::cout << "Visiting parameter list node" << std::endl;

        for(auto& param : node.parameters){
            bool status = symbolTable.addSymbol(std::make_unique<Symbol>(param.second->name,
             Symbol::SymbolKind::VARIABLE, 
             std::move(param.first), 
             interner));
             if (!status){

                std::cout << "Getting error info..." << std::endl;

                errorManager.addError(
                    std::make_unique<CompilerError>(
                        ErrorType::SEMANTIC,
                        "Parameter " + param.second->name + " already declared",
                        0,
                        0
                    )
                );
                return;
            }
        }
    }

    void 
    ProgramChecker::visit(FunctionDefinition& node){

        symbolTable.addSymbol(std::make_unique<Symbol>(node.name->name,
             Symbol::SymbolKind::FUNCTION, 
             std::move(node.returnType), 
             interner));

        node.parameters->accept(*this);

        scopeManager.enterScope(_SymbolMap());

        for(auto& child : node.body){
            child->accept(*this);
        }
        if(node.returnValue != nullptr){
            node.returnValue->accept(*this);
        }

        scopeManager.exitScope();
    }

    void
    ProgramChecker::visit(VariableDeclaration& node){

        auto existingSymbol = symbolTable.getSymbol(node.name->name);
        if(existingSymbol){
            errorManager.addError(
                std::make_unique<CompilerError>(
                    ErrorType::SEMANTIC,
                    "Variable " + node.name->name + " already declared",
                    0,
                    0
                )
            );
            return;
        }

        if (node.initializer != nullptr){
            ExpressionTypeChecker typeChecker(interner, scopeManager, symbolTable, errorManager);
            node.initializer->accept(typeChecker);
            auto initializerType = typeChecker.resultType;

            if(!TypeCompatibility::areTypesCompatible(*node.type, initializerType)){
                errorManager.addError(
                    std::make_unique<CompilerError>(
                        ErrorType::SEMANTIC,
                        "Type mismatch in variable declaration: expected " + node.type->baseType ,
                        0,
                        0
                    )
                );
            }
        }

        symbolTable.addSymbol(std::make_unique<Symbol>(node.name->name,
             Symbol::SymbolKind::VARIABLE, 
             std::move(node.type), 
             interner));

    }


    ExpressionTypeChecker::ExpressionTypeChecker(StringInterner& interner, ScopeManager& scopeManager, 
        SymbolTable& symbolTable, ErrorManager& errorManager)
    :
    SemanticVisitor(interner, scopeManager, errorManager),
    symbolTable(symbolTable) {}

    void
    ExpressionTypeChecker::visit(Literal& node){

        switch (node.literalType){
            case Literal::Type::INTEGER:
                resultType = RvalExpressionType::INTEGER;
                break;
            case Literal::Type::FLOAT:
                resultType = RvalExpressionType::FLOAT;
                break;
            case Literal::Type::BOOLEAN:
                resultType = RvalExpressionType::BOOLEAN;
                break;
            case Literal::Type::CHAR:
                resultType = RvalExpressionType::CHAR;
                break;
            case Literal::Type::STRING:
                resultType = RvalExpressionType::STRING;
                break;
            default:
                errorManager.addError(
                    std::make_unique<CompilerError>(
                        ErrorType::SEMANTIC,
                        "Unknown literal type",
                        0,
                        0
                    )
                );
        }
    }

    void
    ExpressionTypeChecker::visit(Identifier& node){
        auto symbol = symbolTable.getSymbol(node.name);
        if(symbol == nullptr){
            errorManager.addError(
                std::make_unique<CompilerError>(
                    ErrorType::SEMANTIC,
                    "Identifier " + node.name + " not defined",
                    0,
                    0
                )
            );
        }
        
        if(symbol->type->baseType == "int"){
            resultType = RvalExpressionType::INTEGER;
        }else if(symbol->type->baseType == "float"){
            resultType = RvalExpressionType::FLOAT;
        }else if(symbol->type->baseType == "bool"){
            resultType = RvalExpressionType::BOOLEAN;
        }else if(symbol->type->baseType == "char"){
            resultType = RvalExpressionType::CHAR;
        }else if(symbol->type->baseType == "string"){
            resultType = RvalExpressionType::STRING;
        }else{
            resultType = RvalExpressionType::VAR_NAME;
        }
    }

    void
    ExpressionTypeChecker::visit(BinaryExpression& node){
        node.left->accept(*this);
        auto leftType = resultType;
        node.right->accept(*this);
        auto rightType = resultType;

        if(node.op == "+" || node.op == "*" || node.op == "-" || node.op == "/"){
            if(leftType == RvalExpressionType::FLOAT || rightType == RvalExpressionType::FLOAT){
                resultType = RvalExpressionType::FLOAT;
            }else{
                resultType = RvalExpressionType::INTEGER;
            }
        }
        else if (node.op == "==" || node.op == "!=" || node.op == "<" || node.op == ">" ||
            node.op == "<=" || node.op == ">=" || node.op == "&&" || node.op == "||"){
                resultType = RvalExpressionType::BOOLEAN; 
        }
    
    }

    void ExpressionTypeChecker::visit(PrimaryExpression& node){
        switch (node.exprType){
            case PrimaryExpression::IDENTIFIER:
                node.identifier->accept(*this);
                break;
            case PrimaryExpression::LITERAL:
                node.literal->accept(*this);
                break;
            case PrimaryExpression::EXPRESSION_CALL:
                node.functionCall->accept(*this);
                break;
            case PrimaryExpression::PARENTHESIZED:
                node.parenthesized->accept(*this);
                break;
            case PrimaryExpression::ARRAY_ACCESS:
                node.arrayAccess->accept(*this);
                break;
            case PrimaryExpression::MEMBER_ACCESS:
                node.memberAccess->accept(*this);
                break;
            case PrimaryExpression::CAST_EXPRESSION:
                node.castExpression->accept(*this);
                break;
            case PrimaryExpression::TERNARY_EXPRESSION:
                node.ternaryExpression->accept(*this);
                break;
        }
    }

    void ExpressionTypeChecker::visit(FunctionCall& node){
        auto symbol = symbolTable.getSymbol(node.functionName->name);
        if(symbol == nullptr){
            errorManager.addError(
                std::make_unique<CompilerError>(
                    ErrorType::SEMANTIC,
                    "Function " + node.functionName->name + " not defined",
                    0,
                    0
                )
            );
        }

        if(symbol->kind != Symbol::SymbolKind::FUNCTION){
            errorManager.addError(
                std::make_unique<CompilerError>(
                    ErrorType::SEMANTIC,
                    "Identifier " + node.functionName->name + " is not a function",
                    0,
                    0
                )
            );
        }

        if (symbol->type->baseType == "int") {
            resultType = RvalExpressionType::INTEGER;
        } else if (symbol->type->baseType == "float") {
            resultType = RvalExpressionType::FLOAT;
        } else if (symbol->type->baseType == "bool") {
            resultType = RvalExpressionType::BOOLEAN;
        } else if (symbol->type->baseType == "char") {
            resultType = RvalExpressionType::CHAR;
        } else if (symbol->type->baseType == "string") {
            resultType = RvalExpressionType::STRING;
        } else {
            errorManager.addError(
                std::make_unique<CompilerError>(
                    ErrorType::SEMANTIC,
                    "Unknown function return type",
                    0,
                    0
                )
            );
        }


    }
}