#include "umbra/ast/Nodes.h"
#include "umbra/semantic/SymbolCollector.h"
#include "umbra/ast/Visitor.h"
#include "umbra/semantic/SemanticAnalyzer.h"
#include "umbra/utils/utils.h"
#include "umbra/semantic/SymbolTable.h"
#include <algorithm>
#include <iostream>

namespace umbra {

    void SymbolCollector::visitProgramNode(ProgramNode* node) {
        theContext.enterScope();
        for(auto &F : node->functions){
            visit(F.get());
        }
        theContext.exitScope();
    }

    void SymbolCollector::visitFunctionDefinition(FunctionDefinition* node) {

        SemanticType returnType = builtinTypeToSemaType(node->returnType->builtinType);

        // Prepare function signature with parameter types
        FunctionSignature signature{
            .returnType = returnType,
            .argTypes = {}
        };

        if (node->parameters) {
            for(auto& param : node->parameters->parameters) {
                SemanticType paramType = builtinTypeToSemaType(param.first->builtinType);
                signature.argTypes.push_back(paramType);
            }
        }

        // Add function to current (global) scope
        Symbol functionSymbol{
            .type = returnType,
            .kind = SymbolKind::FUCNTION,
            .signature = signature,
            .line = 0,  // TODO: obtener línea real
            .col = 0    // TODO: obtener columna real
        };

        symTable.insert(node->name->name, functionSymbol);

        theContext.enterScope();

        if (node->parameters) {
            for(auto& param : node->parameters->parameters) {
                SemanticType paramType = builtinTypeToSemaType(param.first->builtinType);

                Symbol paramSymbol{
                    .type = paramType,
                    .kind = SymbolKind::VARIABLE,
                    .signature = {},
                    .line = 0,
                    .col = 0
                };
                symTable.insert(param.second->name, paramSymbol);
            }
        }

        for(auto& S: node->body){
            visit(S.get());
        }

        theContext.exitScope();
    }

    void SymbolCollector::visitVariableDeclaration(VariableDeclaration* node){

        Symbol varSymb {
            .type=builtinTypeToSemaType(node->type->builtinType),
            .kind=SymbolKind::VARIABLE,
            .signature={},
            .line=0,
            .col=0
        };

        if(node->initializer != nullptr){
            // Visitar la expresión inicializadora
            if(node->initializer->getKind() == NodeKind::FUNCTION_CALL){

                auto primaryExpr = dynamic_cast<PrimaryExpression*>(node->initializer.get());
                if(primaryExpr && primaryExpr->functionCall) {
                    if(!validateFunctionCall(primaryExpr->functionCall.get())) {
                        return;
                    }
                    std::string& name = primaryExpr->functionCall->functionName->name;
                    auto rvalSym = symTable.lookup(name);
                    if(rvalSym.type != builtinTypeToSemaType(node->type->builtinType)){
                        std::cout << "Function " << name << " has type " << static_cast<int>(
                            rvalSym.type
                        ) << " and variable " << node->name->name << " is type " << static_cast<int>(node->type->builtinType) << std::endl;
                    }
                }
            }

            auto rvalType = typeCk.visit(node->initializer.get());
            node->initializer->builtinExpressionType = semaTypeToBuiltinType(rvalType);
            if(rvalType == SemanticType::Error){
                return;
            }
        }
        symTable.insert(node->name->name, varSymb);
    }

    void SymbolCollector::visitFunctionCall(FunctionCall* node){
        std::cout << "Diving into function call" << std::endl;
        if(node) {
            validateFunctionCall(node);
        }
    }

    void SymbolCollector::visitPrimaryExpression(PrimaryExpression* node) {
        // Visitar expresiones primarias que podrían contener llamadas a función
        if(node && node->getKind() == NodeKind::FUNCTION_CALL) {
            if(node->functionCall) {
                validateFunctionCall(node->functionCall.get());
            }
        }
    }

    void SymbolCollector::visitExpressionStatement(ExpressionStatement* node) {
        if(node && node->exp) {
            if(node->exp->getKind() == NodeKind::FUNCTION_CALL) {
                auto primaryExpr = dynamic_cast<PrimaryExpression*>(node->exp.get());
                if(primaryExpr && primaryExpr->functionCall) {
                    validateFunctionCall(primaryExpr->functionCall.get());
                }
            }
        }
    }

    bool SymbolCollector::validateFunctionCall(FunctionCall* node) {
        if(!node) {
            std::cout << "Error: null function call node" << std::endl;
            return false;
        }

        auto symbolFCall = symTable.lookup(node->functionName->name);
        if(symbolFCall.type == SemanticType::Error){
            std::cout << "Function '" << node->functionName->name << "' does not exist" << std::endl;
            return false;
        }

        std::vector<SemanticType> argTypes = extractArgumentTypes(node->arguments);

        const auto& expectedTypes = symbolFCall.signature.argTypes;
        if(argTypes.size() != expectedTypes.size()) {
            std::cout << "Wrong number of arguments for function '" << node->functionName->name
                      << "'. Expected: " << expectedTypes.size()
                      << ", Got: " << argTypes.size() << std::endl;
            return false;
        }

        bool typesMatch = std::equal(argTypes.begin(), argTypes.end(),
                                   expectedTypes.begin());
        if(!typesMatch) {
            std::cout << "Argument types don't match function signature for '"
                      << node->functionName->name << "'" << std::endl;

            // Mostrar detalles de la discrepancia
            std::cout << "Expected types: ";
            for(const auto& type : expectedTypes) {
                std::cout << static_cast<int>(type) << " ";
            }
            std::cout << "\nActual types: ";
            for(const auto& type : argTypes) {
                std::cout << static_cast<int>(type) << " ";
            }
            std::cout << std::endl;
            return false;
        }
        return true;
    }

    std::vector<SemanticType> SymbolCollector::extractArgumentTypes(const std::vector<std::unique_ptr<Expression>>& arguments) {
        std::vector<SemanticType> argTypes;

        std::transform(arguments.begin(), arguments.end(),
                      std::back_inserter(argTypes),
                      [this](const std::unique_ptr<Expression>& arg) {
                          return typeCk.visit(arg.get());
                      });

        return argTypes;
    }


void SymbolCollector::printCollectedSymbols() const {
    auto scopesVec = symTable.getScopes();
    std::cout << "\n=== Collected Symbols ===\n";
    for(size_t level = 0; level < scopesVec.size(); ++level) {
        const auto& scope = scopesVec[level];
        std::cout << "Scope " << level << " (" << scope.size() << " entradas):\n";
        for(const auto& [name, sym] : scope) {
            std::cout << "  " << name << ": type=" << static_cast<int>(sym.type)
                      << ", kind=" << static_cast<int>(sym.kind);
            if(sym.kind == SymbolKind::FUCNTION) {
                std::cout << ", returns=" << static_cast<int>(sym.signature.returnType);
            }
            std::cout << "\n";
        }
    }
    std::cout << "==========================\n";
}


}
