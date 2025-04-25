#include "SemanticVisitor.h"
#include "../error/CompilerError.h"
#include <iostream>
#include "TypeCompatibility.h"

namespace umbra {

SemanticVisitor::SemanticVisitor(StringInterner& interner, ScopeManager& scopeManager, ErrorManager& errorManager)
    : interner(interner),
      symbolTable(interner),
      scopeManager(scopeManager),
      errorManager(errorManager) {}

void ProgramChecker::visit(ProgramNode& node) {
    std::cout << "Visiting program node" << std::endl;
    for (auto& child : node.functions) {
        child->accept(*this);
    }
}

BuiltinType ProgramChecker::rvalExpressionTypeToBuiltin(RvalExpressionType type) {
    switch (type) {
        case RvalExpressionType::INTEGER:
            return BuiltinType::Int;
        case RvalExpressionType::FLOAT:
            return BuiltinType::Float;
        case RvalExpressionType::STRING:
            return BuiltinType::String;
        case RvalExpressionType::BOOLEAN:
            return BuiltinType::Bool;
        case RvalExpressionType::CHAR:
            return BuiltinType::Char;
        default:
            return BuiltinType::Undef; // Default case
    }
}

void ProgramChecker::visit(ParameterList& node) {
    std::cout << "Visiting parameter list node" << std::endl;
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

        // Registrar el parámetro en el scope local para su eliminación al salir del mismo
        scopeManager.currentScope()[&symbolTable.getSymbol(param.second->name)->name] = nullptr;
    }
}

void ProgramChecker::visit(FunctionDefinition& node) {
    symbolTable.addSymbol(std::make_unique<Symbol>(
        node.name->name,
        Symbol::SymbolKind::FUNCTION,
        std::move(node.returnType),
        interner));

    // Crear un nuevo scope para parámetros y variables locales
    scopeManager.enterScope(_SymbolMap());

    node.parameters->accept(*this);

    for (auto& child : node.body) {
        std::cout << "Checking body of function " << node.name->name << std::endl;
        child->accept(*this);
    }

    if (node.returnValue != nullptr) {
        node.returnValue->accept(*this);
    }

    // Al salir del scope, se eliminan los símbolos locales
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
    std::cout << "Starting type checking for numeric literal" << std::endl;
    if (node.builtinType == BuiltinType::Int) {
        resultType = RvalExpressionType::INTEGER;
    } else if (node.builtinType == BuiltinType::Float) {
        resultType = RvalExpressionType::FLOAT;
    } else {
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
    std::cout << "Starting type checking for string literal" << std::endl;
    resultType = RvalExpressionType::STRING;
}

void ExpressionTypeChecker::visit(BooleanLiteral& node) {
    std::cout << "Starting type checking for boolean literal" << std::endl;
    resultType = RvalExpressionType::BOOLEAN;
}

void ExpressionTypeChecker::visit(Identifier& node) {
    std::cout << "Starting type checking for identifier for " << node.name << std::endl;
    auto symbol = symbolTable.getSymbol(node.name);
    std::cout << "Symbol is " << (symbol == nullptr ? "null" : "not null") << std::endl;
    symbolTable.printAllSymbols();

    if (!symbol) {
        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                "Identifier " + node.name + " not defined",
                0,
                0)
        );
        return;
    }
    
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
    std::cout << "Starting type checking for binary expression" << std::endl;
    node.left->accept(*this);
    auto leftType = resultType;
    std::cout << "Left type is " << static_cast<int>(leftType) << std::endl;
    
    node.right->accept(*this);
    auto rightType = resultType;
    std::cout << "Right type is " << static_cast<int>(rightType) << std::endl;
    
    std::cout << "Operator is " << node.op << std::endl;
    using _eType = RvalExpressionType;
    if (node.op == "+" || node.op == "-" || node.op == "*" || node.op == "/") {
        if (leftType == _eType::INTEGER && rightType == _eType::INTEGER) {
            resultType = _eType::INTEGER;
        } else if (leftType == _eType::FLOAT && rightType == _eType::FLOAT) {
            resultType = _eType::FLOAT;
        }
    } else {
        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                "Invalid types for binary operator " + node.op,
                0,
                0)
        );
    }
}

void ExpressionTypeChecker::visit(PrimaryExpression& node) {
    std::cout << "Visiting primary expression node" << std::endl;
    switch (node.exprType) {
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

void ExpressionTypeChecker::visit(FunctionCall& node) {
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

    if (symbol->kind != Symbol::SymbolKind::FUNCTION) {
        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                "Identifier " + node.functionName->name + " is not a function",
                0,
                0)
        );
        return;
    }

    if (symbol->type->builtinType == BuiltinType::Void) {
        resultType = RvalExpressionType::INTEGER;
    } else if (symbol->type->builtinType == BuiltinType::Int) {
        resultType = RvalExpressionType::FLOAT;
    } else if (symbol->type->builtinType == BuiltinType::Float) {
        resultType = RvalExpressionType::BOOLEAN;
    } else if (symbol->type->builtinType == BuiltinType::Bool) {
        resultType = RvalExpressionType::CHAR;
    } else if (symbol->type->builtinType == BuiltinType::Char) {
        resultType = RvalExpressionType::STRING;
    } else {
        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                "Unknown function return type",
                0,
                0)
        );
    }
}

} // namespace umbra