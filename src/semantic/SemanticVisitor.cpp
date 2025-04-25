#include "SemanticVisitor.h"
#include "../error/CompilerError.h"
#include <iostream>
#include<memory.h>
#include "TypeCompatibility.h"
#include "../utils/utils.h"

namespace umbra {

SemanticVisitor::SemanticVisitor(StringInterner& interner, ScopeManager& scopeManager, ErrorManager& errorManager)
    : interner(interner),
      symbolTable(interner),
      scopeManager(scopeManager),
      errorManager(errorManager) {}

void ProgramChecker::visit(ProgramNode& node) {
    for (auto& child : node.functions) {
        child->accept(*this);
    }
}


void ProgramChecker::visit(ParameterList& node) {

    for (auto& param : node.parameters) {
        bool status = symbolTable.addSymbol(std::make_unique<Symbol>(
            param.second->name,
            Symbol::SymbolKind::VARIABLE,
            std::move(param.first),
            interner));
        if (!status) {
            errorManager.addError(
                std::make_unique<CompilerError>(
                    ErrorType::SEMANTIC,
                    "Parameter " + param.second->name + " already declared",
                    0,
                    0)
            );
            return;
        }

        scopeManager.currentScope()[&symbolTable.getSymbol(param.second->name)->name] = nullptr;
    }
}

void ProgramChecker::visit(FunctionDefinition& node) {

    auto functionSymbol = std::make_unique<Symbol>(
        node.name->name,
        Symbol::SymbolKind::FUNCTION,
        std::move(node.returnType),
        interner);

    if(node.parameters){
        for (auto& param : node.parameters->parameters) {
               functionSymbol->paramTypes.push_back(
                std::make_unique<Type>(param.first->builtinType, param.first->arrayDimensions)
            );
        }
    }

    if(!symbolTable.addSymbol(std::move(functionSymbol))) {
        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                "Function " + node.name->name + " already declared",
                0,
                0)
        );
        return;
    }

    scopeManager.enterScope(_SymbolMap());

    if(node.parameters){
        node.parameters->accept(*this);
    }

    for (auto& child : node.body) {
        child->accept(*this);
    }

    if (node.returnValue != nullptr) {
        node.returnValue->accept(*this);
    }

    scopeManager.exitScope(symbolTable);
}

void ProgramChecker::visit(VariableDeclaration& node) {
    std::cout << "Starting variable declaration analysis " << node.name->name << std::endl;
    
    if (symbolTable.getSymbol(node.name->name)) {
        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                "Variable " + node.name->name + " already declared",
                0,
                0)
        );
        return;
    }

    if (node.initializer != nullptr) {

        ExpressionTypeChecker typeChecker(interner, scopeManager, symbolTable, errorManager);

        node.initializer->accept(typeChecker);

        auto initializerType = typeChecker.resultType;

        node.initializer->builtinExpressionType = rvalExpressionTypeToBuiltin(initializerType);

        if (!TypeCompatibility::areTypesCompatible(*node.type, initializerType)) {
            errorManager.addError(
                std::make_unique<CompilerError>(
                    ErrorType::SEMANTIC,
                    &"Type mismatch in variable declaration: expected " [ static_cast<int>(node.type->builtinType)],
                    0,
                    0)
            );
        }
    }
    
    if (symbolTable.addSymbol(std::make_unique<Symbol>(
            node.name->name,
            Symbol::SymbolKind::VARIABLE,
            std::move(node.type),
            interner)))
    {
        // Registrar el símbolo en el scope actual.
        scopeManager.currentScope()[&symbolTable.getSymbol(node.name->name)->name] = nullptr;
    }
}

ExpressionTypeChecker::ExpressionTypeChecker(StringInterner& interner, ScopeManager& scopeManager,
    SymbolTable& symbolTable, ErrorManager& errorManager)
    : SemanticVisitor(interner, scopeManager, errorManager),
      scopeManager(scopeManager),
      symbolTable(symbolTable) {}

void ExpressionTypeChecker::visit(Literal& node) {
    // Implementación según sea necesario
}

void ExpressionTypeChecker::visit(NumericLiteral& node) {
    
    if (node.builtinType == BuiltinType::Int) {
        node.builtinExpressionType = BuiltinType::Int;
        resultType = RvalExpressionType::INTEGER;
    } else if (node.builtinType == BuiltinType::Float) {
        node.builtinExpressionType = BuiltinType::Float;
        resultType = RvalExpressionType::FLOAT;
    } else {
        node.builtinExpressionType = BuiltinType::Undef;
        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                "Unknown numeric literal type",
                0,
                0)
        );
    }
}

void ExpressionTypeChecker::visit(StringLiteral& node) {
    node.builtinExpressionType = BuiltinType::String;
    resultType = RvalExpressionType::STRING;
}

void ExpressionTypeChecker::visit(BooleanLiteral& node) {
    node.builtinExpressionType = BuiltinType::Bool;
    resultType = RvalExpressionType::BOOLEAN;
}

void ExpressionTypeChecker::visit(Identifier& node) {

    auto symbol = symbolTable.getSymbol(node.name);
    if (!symbol) {
        node.builtinExpressionType = BuiltinType::Undef;
        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                "Identifier " + node.name + " not defined",
                0,
                0)
        );
        return;
    }
    node.resolvedSymbol = symbol;
    node.builtinExpressionType = symbol->type->builtinType;
    
    if (symbol->type->builtinType == BuiltinType::Int) {
        resultType = RvalExpressionType::INTEGER;
    } else if (symbol->type->builtinType == BuiltinType::Float) {
        resultType = RvalExpressionType::FLOAT;
    } else if (symbol->type->builtinType == BuiltinType::Bool) {
        resultType = RvalExpressionType::BOOLEAN;
    } else if (symbol->type->builtinType == BuiltinType::Char) {
        resultType = RvalExpressionType::CHAR;
    } else if (symbol->type->builtinType == BuiltinType::String) {
        resultType = RvalExpressionType::STRING;
    } else {
        resultType = RvalExpressionType::VAR_NAME;
    }
}

void ExpressionTypeChecker::visit(BinaryExpression& node) {
    node.left->accept(*this);
    auto leftType = resultType;
    // NO asignes node.builtinExpressionType aquí todavía

    node.right->accept(*this);
    auto rightType = resultType;

    using _eType = RvalExpressionType;

    _eType finalResultType = _eType::VAR_NAME; 

    if (node.op == "+" || node.op == "-" || node.op == "*" || node.op == "/") {
        if (leftType == _eType::INTEGER && rightType == _eType::INTEGER) {
            finalResultType = _eType::INTEGER;
        } else if (leftType == _eType::FLOAT && rightType == _eType::FLOAT) {
            finalResultType = _eType::FLOAT;
        } // TODO: Añadir más casos (int/float, string concat, etc.)
        else {
  
            errorManager.addError(
                std::make_unique<CompilerError>(
                    ErrorType::SEMANTIC,
                    "Invalid types for operator " + node.op,
                    0,
                    0)
            );
            finalResultType = _eType::VAR_NAME; // O tipo error
        }
    } else if (node.op == "==" || node.op == "!=" || node.op == "<" /* ... etc ... */) {
         // TODO: Implementar chequeo para operadores de comparación/lógicos
         // El resultado suele ser BOOLEAN si los tipos son comparables
         if (TypeCompatibility::areTypesCompatible(rvalExpressionTypeToBuiltin(leftType), rightType)) { // Necesitarás una función de compatibilidad
             finalResultType = _eType::BOOLEAN;
         } else {
                errorManager.addError(
                    std::make_unique<CompilerError>(
                        ErrorType::SEMANTIC,
                        "Invalid types for operator " + node.op,
                        0,
                        0)
                );
            finalResultType = _eType::VAR_NAME;
         }
    } else {

        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                "Unknown operator " + node.op,
                0,
                0)
        );
        finalResultType = _eType::VAR_NAME; 
    }

    resultType = finalResultType;

    node.builtinExpressionType = rvalExpressionTypeToBuiltin(finalResultType);
}

void ExpressionTypeChecker::visit(PrimaryExpression& node) {
    BuiltinType finalBuiltinType = BuiltinType::Undef; // Tipo por defecto o error

    switch (node.exprType) {
        case PrimaryExpression::IDENTIFIER:
            node.identifier->accept(*this);
            // Propagar el tipo del identificador al nodo primario
            finalBuiltinType = node.identifier->builtinExpressionType;
            break;
        case PrimaryExpression::LITERAL:
            // Asumiendo que node.literal es un puntero a la clase base Literal
            // y que los visit específicos (Numeric, String, etc.) ya fueron llamados
            // Necesitamos una forma de obtener el tipo del literal específico.
            // Podríamos necesitar visitar el literal específico aquí o
            // asegurar que el nodo base Literal también tenga builtinExpressionType.
            // Por ahora, asumamos que el accept ya lo decoró:
            node.literal->accept(*this); // Esto debería llamar al visit específico
            finalBuiltinType = node.literal->builtinExpressionType; // Asumiendo que Literal base lo tiene
            break;
        case PrimaryExpression::EXPRESSION_CALL:
            node.functionCall->accept(*this);
            // Propagar el tipo de la llamada al nodo primario
            finalBuiltinType = node.functionCall->builtinExpressionType;
            break;
        case PrimaryExpression::PARENTHESIZED:
            node.parenthesized->accept(*this);
            // Propagar el tipo de la expresión entre paréntesis al nodo primario
            finalBuiltinType = node.parenthesized->builtinExpressionType;
            break;
        // --- Añadir casos para los otros tipos de PrimaryExpression ---
        // case PrimaryExpression::ARRAY_ACCESS:
        //     node.arrayAccess->accept(*this);
        //     finalBuiltinType = node.arrayAccess->builtinExpressionType;
        //     break;
        // case PrimaryExpression::MEMBER_ACCESS:
        //     node.memberAccess->accept(*this);
        //     finalBuiltinType = node.memberAccess->builtinExpressionType;
        //     break;
        // case PrimaryExpression::CAST_EXPRESSION:
        //     node.castExpression->accept(*this);
        //     finalBuiltinType = node.castExpression->builtinExpressionType;
        //     break;
        // case PrimaryExpression::TERNARY_EXPRESSION:
        //     node.ternaryExpression->accept(*this);
        //     finalBuiltinType = node.ternaryExpression->builtinExpressionType;
        //     break;
        default:
             errorManager.addError(
                std::make_unique<CompilerError>(
                    ErrorType::SEMANTIC, // O SEMANTIC si es un tipo no esperado
                    "Unhandled primary expression type in type checker",
                    0, 0 // TODO: Línea/Columna
                )
             );
             finalBuiltinType = BuiltinType::Undef;
             resultType = RvalExpressionType::ERROR; // Actualizar también resultType interno
             break; // Salir del switch
    }

    // Decorar el nodo PrimaryExpression con el tipo determinado
    node.builtinExpressionType = finalBuiltinType;
    // El resultType interno ya debería haber sido establecido por la llamada a accept del hijo.
    // No necesitas reasignarlo aquí a menos que el default lo haya cambiado a ERROR.
}

void ExpressionTypeChecker::visit(FunctionCall& node) {
    std::vector<BuiltinType> argTypes; 
    for(auto& arg : node.arguments){
        arg->accept(*this);

        argTypes.push_back(arg->builtinExpressionType); 
    }

    auto symbol = symbolTable.getSymbol(node.functionName->name);
    if (!symbol) {
        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                "Function " + node.functionName->name + " not defined",
                0,
                0)
        );
        return;
    }

    if(symbol->kind != Symbol::SymbolKind::FUNCTION) {
        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                "Identifier " + node.functionName->name + " is not a function",
                0,
                0)
        );
        return;
    }

    node.functionName->resolvedSymbol = symbol;


    // --- VERIFICACIÓN DE ARGUMENTOS ---
    // Comparar número de argumentos
    if (argTypes.size() != symbol->paramTypes.size()) {
        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                "Function '" + node.functionName->name + "' expects " +
                std::to_string(symbol->paramTypes.size()) + " arguments, but " +
                std::to_string(argTypes.size()) + " were provided.",
                0, 0 // TODO: Línea/Columna
            )
        );

    } else {
        // Comparar tipos de argumentos
        for (size_t i = 0; i < argTypes.size(); ++i) {
            // Necesitamos comparar BuiltinType de parameterTypes[i] con argTypes[i]
            if (!TypeCompatibility::areTypesCompatible(symbol->paramTypes[i]->builtinType, argTypes[i])) {
                 errorManager.addError(
                    std::make_unique<CompilerError>(
                        ErrorType::SEMANTIC,
                        "Type mismatch for argument " + std::to_string(i+1) + " in call to function '" +
                        node.functionName->name + "'. Expected compatible with type X, got type Y.", // TODO: Mejorar mensaje con nombres de tipos
                        0, 0 // TODO: Línea/Columna del argumento i
                    )
                );
            }
            // TODO: Considerar dimensiones de array
        }
    }
    // ---------------------------------


    // Asignar tipo de retorno
    node.builtinExpressionType = symbol->type->builtinType;

    // Asignar resultType interno
    switch(symbol->type->builtinType) {
        case BuiltinType::Int:    resultType = RvalExpressionType::INTEGER; break;
        case BuiltinType::Float:  resultType = RvalExpressionType::FLOAT;   break;
        case BuiltinType::Bool:   resultType = RvalExpressionType::BOOLEAN; break;
        case BuiltinType::Char:   resultType = RvalExpressionType::CHAR;    break;
        case BuiltinType::String: resultType = RvalExpressionType::STRING;  break;
        case BuiltinType::Void:
             errorManager.addError(
                std::make_unique<CompilerError>(
                    ErrorType::SEMANTIC,
                    "Function " + node.functionName->name + " returns void",
                    0,
                    0)
            );
             resultType = RvalExpressionType::ERROR; // O VOID
             break;
        default:
             resultType = RvalExpressionType::VAR_NAME; // O ERROR
             break;
    }
}

} // namespace umbra