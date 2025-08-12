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
        // Usar el scope global existente. Registrar builtins antes.
        registerBuiltins();
        for(auto &F : node->functions){
            visit(F.get());
        }
        // Validar punto de entrada al final
        validateEntryPoint();
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

        node->Signature = signature;

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

            if(node->initializer->getKind() == NodeKind::FUNCTION_CALL){

                auto primaryExpr = dynamic_cast<PrimaryExpression*>(node->initializer.get());
                if(primaryExpr && primaryExpr->functionCall) {
                    if(!validateFunctionCall(primaryExpr->functionCall.get())) {
                        return;
                    }
                    std::string& name = primaryExpr->functionCall->functionName->name;
                    auto rvalSym = symTable.lookup(name);
                    if(rvalSym.type != builtinTypeToSemaType(node->type->builtinType)){
                        std::string msg = "Type mismatch: function '" + name + "' returns " + std::to_string(static_cast<int>(rvalSym.type)) +
                                          ", but variable '" + node->name->name + "' is type " + std::to_string(static_cast<int>(node->type->builtinType));
                        errorManager.addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
                        return;
                    }
                }
            }

            auto rvalType = typeCk.visit(node->initializer.get());
            node->initializer->builtinExpressionType = semaTypeToBuiltinType(rvalType);
            if(rvalType == SemanticType::Error){
                errorManager.addError(std::make_unique<SemanticError>("Invalid initializer expression", 0, 0, SemanticError::Action::ERROR));
                return;
            }
        }
        symTable.insert(node->name->name, varSymb);
    }

    void SymbolCollector::visitFunctionCall(FunctionCall* node){
        if(node) {
            validateFunctionCall(node);
        }
    }

    void SymbolCollector::visitPrimaryExpression(PrimaryExpression* node) {
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
            errorManager.addError(std::make_unique<SemanticError>("Null function call node", 0, 0, SemanticError::Action::ERROR));
            return false;
        }

        auto symbolFCall = symTable.lookup(node->functionName->name);
        if(symbolFCall.type == SemanticType::Error){
            std::string msg = "Undefined function '" + node->functionName->name + "'";
            errorManager.addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
            return false;
        }

        if(node->functionName->name == "print"){
            //Parsear string de la funcion print, por ejemplo print("n: {$1}, n2: {$2}", v1, v2);
            std::string_view arg1 = node->functionName->name;

        }

        std::vector<SemanticType> argTypes = extractArgumentTypes(node->arguments);

        const auto& expectedTypes = symbolFCall.signature.argTypes;
        if(argTypes.size() != expectedTypes.size()) {
            std::string msg = "Wrong number of arguments for function '" + node->functionName->name + "'. Expected: " +
                              std::to_string(expectedTypes.size()) + ", Got: " + std::to_string(argTypes.size());
            errorManager.addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
            return false;
        }

        bool typesMatch = std::equal(argTypes.begin(), argTypes.end(), expectedTypes.begin());
        if(!typesMatch) {
            std::string msg = "Argument types don't match function signature for '" + node->functionName->name + "'";
            errorManager.addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
            return false;
        }
        node->argTypes = argTypes;
        node->semaT = symbolFCall.signature.returnType;
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

void SymbolCollector::registerBuiltins() {
    // builtin: print(string) -> void (mínimo para Hola Mundo)
    Symbol printSym{
        .type = SemanticType::Void,
        .kind = SymbolKind::FUCNTION,
        .signature = FunctionSignature{true, SemanticType::Void, {SemanticType::String}},
        .line = 0,
        .col = 0
    };
    symTable.insert("print", printSym);
}

void SymbolCollector::validateEntryPoint() {
    auto sym = symTable.lookup("start");
    if (sym.kind != SymbolKind::FUCNTION || sym.signature.argTypes.size() != 0) {
        errorManager.addError(std::make_unique<SemanticError>(
            "Entry point 'start' must be a function with no parameters", 0, 0, SemanticError::Action::ERROR));
        return;
    }
    // Permitir Void o Int
    if (!(sym.signature.returnType == SemanticType::Void || sym.signature.returnType == SemanticType::Int)) {
        errorManager.addError(std::make_unique<SemanticError>(
            "Entry point 'start' must return void or int", 0, 0, SemanticError::Action::ERROR));
    }
}


}
