/**
 * @file SymbolCollector.cpp
 * @brief Visitor que recolecta símbolos (funciones, parámetros, variables) y valida llamadas.
 * @details
 * - Inserta builtins (p. ej. print variádica) en el scope global.
 * - Construye firmas de funciones (retorno + params) y las guarda en el AST.
 * - Abre/cierra scopes por función usando SemanticContext.
 * - Valida llamadas (existencia, número/tipos de args) y el entry point start.
 *
 * Notas:
 * - Usa TypeCk para inferir tipos de expresiones (inicializadores/args).
 * - Reporta errores en ErrorManager (no lanza excepciones).
 */

#include "umbra/ast/Nodes.h"
#include "umbra/semantic/SymbolCollector.h"
#include "umbra/ast/Visitor.h"
#include "umbra/semantic/SemanticAnalyzer.h"
#include "umbra/utils/utils.h"
#include "umbra/semantic/SymbolTable.h"
#include <algorithm>
#include <iostream>

namespace umbra {

/// @brief Orquesta la recolección de símbolos a nivel de programa.
/// @details Registra builtins, visita cada función y valida el entry point.
/// @param node Nodo raíz del programa (no nulo).
void SymbolCollector::visitProgramNode(ProgramNode* node) {
    registerBuiltins();
    for(auto &F : node->functions){
        visit(F.get());
    }
    validateEntryPoint();
}

/**
 * @brief Declara la función en el scope global y procesa su cuerpo.
 * @details
 * 1) Construye la FunctionSignature (tipos de parámetros + retorno) y la cuelga en el nodo.
 * 2) Inserta el símbolo de la función en el scope global.
 * 3) Entra al scope de la función, inserta parámetros como variables y visita el cuerpo.
 * 4) Sale del scope de la función.
 * @param node Definición de función a registrar/visitar.
 */
void SymbolCollector::visitFunctionDefinition(FunctionDefinition* node) {

    SemanticType returnType = builtinTypeToSemaType(node->returnType->builtinType);

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

/**
 * @brief Inserta variables locales y valida su inicializador (si existe).
 * @details
 * - Si el inicializador es una llamada, primero valida la firma y el tipo de retorno.
 * - Consulta a TypeCk para inferir el tipo de la expresión; si falla, reporta error.
 * - Inserta el símbolo de la variable en el scope actual.
 * @param node Declaración de variable a procesar.
 */
void SymbolCollector::visitVariableDeclaration(VariableDeclaration* node){

    Symbol varSymb {
        .type=builtinTypeToSemaType(node->type->builtinType),
        .kind=SymbolKind::VARIABLE,
        .signature={},
        .line=0,
        .col=0
    };

    if(node->initializer != nullptr){
        validateCallsInExpression(node->initializer.get());

        // TypeCk ahora reporta errores específicos a través de errorManager,
        // por lo que no necesitamos agregar un mensaje genérico aquí.
        (void)typeCk.visit(node->initializer.get());
        // Si el resultado es Error, TypeCk ya reportó el problema específico
    }
    symTable.insert(node->name->name, varSymb);
}

void SymbolCollector::visitAssignmentStatement(AssignmentStatement* node){
    auto Sym = theContext.symbolTable.lookup(node->target->name);
    if(Sym.type == SemanticType::Error){
        std::string msg = "Cannot assign to undefined variable '" + node->target->name + "' (variable not declared in current scope)";
        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                msg,
                Sym.line,
                Sym.col
            )
        );
        return; // No continuar validando si la variable no existe
    }
    validateCallsInExpression(node->value.get());

    SemanticType semaT = typeCk.visit(node->value.get());

    // TypeCk ya reportó errores específicos si hubo problemas en la expresión
    if(semaT == SemanticType::Error){
        return; // El error específico ya fue reportado por TypeCk
    }

    if(semaT != Sym.type){
        std::string msg = "Type mismatch in assignment: variable '" + node->target->name +
                          "' has type '" + semanticTypeToString(Sym.type) +
                          "' but assigned value has type '" + semanticTypeToString(semaT) + "'";
        errorManager.addError(
            std::make_unique<CompilerError>(
                ErrorType::SEMANTIC,
                msg,
                Sym.line,
                Sym.col
            )
        );
    }
}

/// @brief Visita directa a una llamada a función (si aparece aislada).
/// @param node Nodo de llamada (puede ser nulo).
void SymbolCollector::visitFunctionCall(FunctionCall* node){
    if(node) {
        validateFunctionCall(node);
    }
}

/// @brief Si el PrimaryExpression contiene una llamada, la valida.
/// @param node Nodo primario a revisar.
void SymbolCollector::visitPrimaryExpression(PrimaryExpression* node) {
    if(node && node->exprType == PrimaryExpression::Type::EXPRESSION_CALL) {
        if(node->functionCall) {
            validateFunctionCall(node->functionCall.get());
        }
    }
}

/// @brief Valida llamadas que aparecen como statement (expresión suelta).
/// @param node Statement con expresión; si es llamada, se valida.
void SymbolCollector::visitExpressionStatement(ExpressionStatement* node) {
    PrimaryExpression* pExpr = dynamic_cast<PrimaryExpression*>(node->exp.get());
    if(pExpr != nullptr){
        visitPrimaryExpression(pExpr);
    }
}

void SymbolCollector::visitIfStatement(IfStatement* node) {
    if(!node) return;

    // Validar condiciones y cuerpos de todas las ramas (if, elseif)
    for(auto& branch : node->branches) {
        if(branch.condition) {
            validateCallsInExpression(branch.condition.get());
        }
        for(auto& stmt : branch.body) {
            visit(stmt.get());
        }
    }

    // Validar rama else si existe
    for(auto& stmt : node->elseBranch) {
        visit(stmt.get());
    }
}

void SymbolCollector::visitRepeatTimesStatement(RepeatTimesStatement* node) {
    if(!node) return;

    // Validar la expresión de veces
    if(node->times) {
        validateCallsInExpression(node->times.get());
    }

    // Visitar el cuerpo del bucle
    for(auto& stmt : node->body) {
        visit(stmt.get());
    }
}

void SymbolCollector::visitRepeatIfStatement(RepeatIfStatement* node) {
    if(!node) return;

    // Validar la condición
    if(node->condition) {
        validateCallsInExpression(node->condition.get());
    }

    // Visitar el cuerpo del bucle
    for(auto& stmt : node->body) {
        visit(stmt.get());
    }
}

/**
 * @brief Valida semánticamente una llamada a función.
 * @details
 * - Busca el símbolo de la función (si no existe: error).
 * - Caso especial "print": la firma está marcada como variádica en registerBuiltins()
 *   (pendiente de parsear placeholders aquí; por ahora no se forza conteo para el extra).
 * - Para funciones no variádicas: compara número y tipos de argumentos 1:1.
 * - Propaga en el nodo: tipos de args (argTypes) y tipo de retorno (semaT).
 * @param node Nodo de llamada.
 * @return true si la llamada es válida; false si se reportó error.
 */
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

    std::vector<SemanticType> argTypes = extractArgumentTypes(node->arguments);
    const auto& expectedTypes = symbolFCall.signature.argTypes;

    if(symbolFCall.signature.isVarArg) {
        if(argTypes.size() < expectedTypes.size()) {
            std::string msg = "Wrong number of arguments for function '" + node->functionName->name + "'. Expected at least: " +
                              std::to_string(expectedTypes.size()) + ", Got: " + std::to_string(argTypes.size());
            errorManager.addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
            return false;
        }
    } else {
        if(argTypes.size() != expectedTypes.size()) {
            std::string msg = "Wrong number of arguments for function '" + node->functionName->name + "'. Expected: " +
                              std::to_string(expectedTypes.size()) + ", Got: " + std::to_string(argTypes.size());
            errorManager.addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
            return false;
        }
    }

    for(size_t i = 0; i < expectedTypes.size(); ++i) {
        if(argTypes[i] != expectedTypes[i]) {
            std::string msg = "Type mismatch in argument " + std::to_string(i + 1) + " of function '" + node->functionName->name +
                              "': expected type '" + semanticTypeToString(expectedTypes[i]) +
                              "' but got type '" + semanticTypeToString(argTypes[i]) + "'";
            errorManager.addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
            return false;
        }
    }

    node->argTypes = std::move(argTypes);
    node->semaT = symbolFCall.signature.returnType;
    return true;
}

/**
 * @brief Convierte cada argumento a su SemanticType usando el TypeCk.
 * @param arguments Vector de expresiones de argumentos.
 * @return Vector de tipos inferidos en el mismo orden.
 */
void SymbolCollector::validateCallsInExpression(Expression* expr) {
    static int recursionDepth = 0;
    if(!expr) return;

    // Protección contra recursión infinita
    if(++recursionDepth > 1000) {
        std::string msg = "Internal error: infinite recursion in validateCallsInExpression";
        errorManager.addError(std::make_unique<SemanticError>(msg, 0, 0, SemanticError::Action::ERROR));
        --recursionDepth;
        return;
    }

    if(expr->getKind() == NodeKind::PRIMARY_EXPRESSION) {
        auto* primaryExpr = dynamic_cast<PrimaryExpression*>(expr);
        if(primaryExpr && primaryExpr->exprType == PrimaryExpression::Type::EXPRESSION_CALL && primaryExpr->functionCall) {
            validateFunctionCall(primaryExpr->functionCall.get());
            for(auto& arg : primaryExpr->functionCall->arguments) {
                validateCallsInExpression(arg.get());
            }
        }
    } else if(expr->getKind() == NodeKind::BINARY_EXPRESSION) {
        auto* binExpr = dynamic_cast<BinaryExpression*>(expr);
        if(binExpr) {
            validateCallsInExpression(binExpr->left.get());
            validateCallsInExpression(binExpr->right.get());
        }
    } else if(expr->getKind() == NodeKind::UNARY_EXPRESSION) {
        auto* unaryExpr = dynamic_cast<UnaryExpression*>(expr);
        if(unaryExpr) {
            validateCallsInExpression(unaryExpr->operand.get());
        }
    }

    --recursionDepth;
}

std::vector<SemanticType> SymbolCollector::extractArgumentTypes(const std::vector<std::unique_ptr<Expression>>& arguments) {
    std::vector<SemanticType> argTypes;

    std::transform(arguments.begin(), arguments.end(),
                  std::back_inserter(argTypes),
                  [this](const std::unique_ptr<Expression>& arg) {
                      validateCallsInExpression(arg.get());
                      return typeCk.visit(arg.get());
                  });

    return argTypes;
}

/**
 * @brief Imprime todos los símbolos recolectados, agrupados por scope.
 * @details Útil para depuración: muestra nombre, tipo y kind; para funciones, el tipo de retorno.
 */
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

/**
 * @brief Registra símbolos builtin en el scope global.
 * @details
 * - print(String, ...) -> Void (variádica): la firma marca vararg=true y exige primer arg String.
 */
void SymbolCollector::registerBuiltins() {
    Symbol printSym{
        .type = SemanticType::Void,
        .kind = SymbolKind::FUCNTION,
        .signature = FunctionSignature{true, SemanticType::Void, {SemanticType::String}},
        .line = 0,
        .col = 0
    };
    symTable.insert("print", printSym);
}

/**
 * @brief Valida el punto de entrada del programa.
 * @details Debe existir start() con 0 parámetros y retorno void o int.
 */
void SymbolCollector::validateEntryPoint() {
    auto sym = symTable.lookup("start");
    if (sym.kind != SymbolKind::FUCNTION || sym.signature.argTypes.size() != 0) {
        errorManager.addError(std::make_unique<SemanticError>(
            "Entry point 'start' must be a function with no parameters", 0, 0, SemanticError::Action::ERROR));
    }
    if (!(sym.signature.returnType == SemanticType::Void || sym.signature.returnType == SemanticType::Int)) {
        errorManager.addError(std::make_unique<SemanticError>(
            "Entry point 'start' must return void or int", 0, 0, SemanticError::Action::ERROR));
    }
}


} // namespace umbra
