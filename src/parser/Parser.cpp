#include "Parser.h"
#include "../error/ErrorManager.h"
#include <algorithm>
#include <vector>
#include <cctype>

namespace umbra {

/**
 * @brief Constructor de Parser.
 * @details Inicializa una nueva instancia de Parser a partir de un vector de tokens. 
 * Se establece el iterador 'current' al inicio del vector y se guarda el primer token 
 * en 'previousToken'. Este constructor es útil cuando no se dispone de un ErrorManager externo.
 * 
 * @param tokens Vector de tokens proveniente del análisis léxico.
 */
Parser::Parser(const std::vector<Lexer::Token>& tokens) 
    : tokens(tokens), current(tokens.begin()), errorManager(nullptr),
      previousToken(*current) {
}

/**
 * @brief Constructor de Parser con ErrorManager externo.
 * @details Inicializa una nueva instancia de Parser usando un vector de tokens y una 
 * referencia a un ErrorManager externo para registrar errores sintácticos durante el análisis.
 * Se posiciona el iterador 'current' al comienzo del vector y se guarda el primer token
 * en 'previousToken'.
 *
 * @param tokens Vector de tokens generados por el analizador léxico.
 * @param externalErrorManager Referencia al manejador de errores externo.
 */
Parser::Parser(const std::vector<Lexer::Token>& tokens, ErrorManager& externalErrorManager) 
    : tokens(tokens), current(tokens.begin()),
      errorManager(&externalErrorManager), 
      previousToken(*current) {
}

/**
 * @brief Verifica y avanza si el token actual coincide con el tipo esperado.
 * @details Esta función comprueba si el token actual tiene el tipo indicado en @p type.
 * Si es así, se avanza el iterador y se retorna true. De lo contrario, se retorna false.
 *
 * @param type Tipo de token a comparar.
 * @return true si el token actual coincide con @p type y se avanza; false en caso contrario.
 */
bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

/**
 * @brief Obtiene la representación en cadena del tipo de un token.
 * @details Dado un token @p tk, esta función utiliza un switch para retornar el lexema 
 * asociado si el token corresponde a un tipo (por ejemplo, entero, flotante, booleano, etc.).
 * Si el token no es de tipo, se retorna una cadena vacía.
 *
 * @param tk Token del cual obtener la representación de su tipo.
 * @return Cadena que representa el tipo del token o una cadena vacía si no corresponde a un tipo.
 */
BuiltinType tokenTypeToBuiltin(Lexer::Token tk){
    switch(tk.type){
        case TokenType::TOK_INT:
            return BuiltinType::Int;
        case TokenType::TOK_FLOAT:
            return BuiltinType::Float;
        case TokenType::TOK_BOOL:
            return BuiltinType::Bool;
        case TokenType::TOK_CHAR:
            return BuiltinType::Char;
        case TokenType::TOK_STRING:
            return BuiltinType::String;
        case TokenType::TOK_VOID:
            return BuiltinType::Void;
        default:
            return BuiltinType::Undef;
    }
}

/**
 * @brief Retorna el token ubicado a cierta distancia desde el token actual.
 * @details Permite visualizar el token que se encuentra @p distance posiciones adelante 
 * en el vector de tokens. Si no existe un token en esa posición, se retorna el último token del vector.
 *
 * @param distance La cantidad de tokens a adelantar desde la posición actual.
 * @return El token que se encuentra a @p distance posiciones adelante o el último token si se excede el límite.
 */
Lexer::Token Parser::lookAhead(int distance) {
    if (current + distance < tokens.end()) {
        return *(current + distance);
    }
    return tokens.back();
}

/**
 * @brief Comprueba si el token actual es del tipo indicado.
 * @details Verifica que el token que actualmente se apunta no sea el final de la entrada y
 * compara su tipo con el valor de @p type. En modo debug, detalla mediante un string stream
 * el proceso de verificación.
 *
 * @param type Tipo de token que se espera en la posición actual.
 * @return true si el token actual coincide con @p type; false en caso contrario o si se ha alcanzado el final.
 */
bool Parser::check(TokenType type) const {
    if (isAtEnd()) {
        return false;
    }

    return peek().type == type;
}

/**
 * @brief Avanza al siguiente token y retorna el token actual.
 * @details Si el parser no se encuentra al final de la entrada, esta función obtiene el token actual,
 * imprime información de depuración (si UMBRA_DEBUG está activado), actualiza el token previo con el
 * token actual y luego incrementa el iterador para avanzar al siguiente token en el vector.
 * Si el parser está al final de la entrada, retorna el último token consumido.
 *
 * @return El token actual antes de avanzar.
 */
Lexer::Token Parser::advance() {
    if (!isAtEnd()) {
        previousToken = *current;
        return *current++;
    }

    return previous();
}

/**
 * @brief Obtiene el token previamente consumido.
 * @details Esta función retorna el último token que fue consumido por el parser. 
 * Es útil para poder referenciar el contexto anterior sin modificar la posición actual en el flujo de tokens.
 *
 * @return El token que fue consumido anteriormente.
 */
Lexer::Token Parser::previous() const { 
    return previousToken; 
}

bool isNumber(const std::string &str) {
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

/**
 * @brief Omite los tokens de nueva línea.
 * @details Mientras que el token actual sea un salto de línea, se llama a la función match para avanzar.
 * Esta función ayuda a ignorar líneas vacías o separadores en la entrada, permitiendo continuar el análisis sin
 * procesar redundancias en la estructura del código.
 */
void Parser::skipNewLines(){
    while(match(TokenType::TOK_NEWLINE));
}

/**
 * @brief Retorna el token actual sin avanzar el iterador.
 * @details Esta función proporciona acceso al token que se encuentra en la posición actual del flujo sin 
 * alterar el estado del parser. Es fundamental para operaciones de lookahead o para validaciones donde no 
 * se desee consumir el token.
 *
 * @return El token actual apuntado por el iterador.
 */
Lexer::Token Parser::peek() const { 
    return *current; 
}

/**
 * @brief Verifica si el parser ha alcanzado el fin de la entrada.
 * @details Compara el tipo del token actual con el token especial de fin de archivo (EOF). Esto garantiza que 
 * el parser no intente avanzar más allá del flujo de tokens y ayuda a proteger contra accesos fuera de rango.
 *
 * @return true si el token actual es EOF; false en caso contrario.
 */
bool Parser::isAtEnd() const { 
    return peek().type == TokenType::TOK_EOF; 
}

/**
 * @brief Consume el token actual si coincide con el tipo esperado, en caso contrario genera un error.
 * @details Compara el tipo del token actual con el tipo esperado (@p type). Si coinciden, se avanza el parser y se 
 * retorna el token consumido. Si no coinciden, se utiliza el ErrorManager para registrar un error sintáctico con
 * un mensaje descriptivo y, a continuación, se lanza una excepción std::runtime_error para interrumpir el análisis.
 *
 * @param type El tipo de token que se espera consumir en este momento.
 * @param message Mensaje descriptivo que se usará en caso de error.
 * @return El token consumido satisfactoriamente si coincide con @p type.
 * @throws std::runtime_error Si el token actual no coincide con el tipo esperado.
 */
Lexer::Token Parser::consume(TokenType type, const std::string& message) {
    Lexer::Token currentToken = peek();

    if (check(type)) {
        advance();
        return currentToken;
    }
    errorManager->addError(
        std::make_unique<CompilerError>(ErrorType::SYNTACTIC, 
        message, peek().line, peek().column));
    throw std::runtime_error(message);
}

bool Parser::isTypeToken(const Lexer::Token& token) {
    return token.type == TokenType::TOK_INT 
    || token.type == TokenType::TOK_FLOAT 
    || token.type == TokenType::TOK_BOOL 
    || token.type == TokenType::TOK_CHAR 
    || token.type == TokenType::TOK_STRING 
    || token.type == TokenType::TOK_ARRAY 
    || token.type == TokenType::TOK_VOID;
}

//<program> ::= { <function_definition> }
std::unique_ptr<ProgramNode> Parser::parseProgram() {
    std::vector<std::unique_ptr<FunctionDefinition>> functionDefinitions;

    while (!isAtEnd()) {
        functionDefinitions.push_back(parseFunctionDefinition());
        if(match(TokenType::TOK_NEWLINE)) {
            continue;
        }

    }
    return std::make_unique<ProgramNode>(std::move(functionDefinitions));
}
//<function_definition> ::= "func" <identifier> "(" [ <parameter_list> ] ")" "->" <type> "{" <statement_list> "}"
std::unique_ptr<FunctionDefinition> Parser::parseFunctionDefinition(){

    if(!match(TokenType::TOK_FUNC)){
        throw std::runtime_error("Expected 'func' keyword");
    }

    Lexer::Token name = consume(TokenType::TOK_IDENTIFIER,
    "Expected function name");

    consume(TokenType::TOK_LEFT_PAREN, 
    "Expected '(' after function name");

    std::vector<std::pair<std::unique_ptr<Type>, std::unique_ptr<Identifier>>> parameters;

    if(!check(TokenType::TOK_RIGHT_PAREN)){ //Parse params
        do{
            auto paramType = parseType();
            Lexer::Token paramName = consume(TokenType::TOK_IDENTIFIER,
                "Expected parameter name");
            auto paramId = std::make_unique<Identifier>(paramName.lexeme);
            parameters.push_back(std::make_pair(std::move(paramType), 
            std::move(paramId)));
        } while(match(TokenType::TOK_COMMA));
    }

    consume(TokenType::TOK_RIGHT_PAREN, "Expected ')' after parameter list");
    consume(TokenType::TOK_ARROW, "Expected '->' after parameter list");

    auto returnType = parseType();

    consume(TokenType::TOK_LEFT_BRACE, "Expected '{' before function body");
    
    skipNewLines();

    auto body = parseStatementList();
    auto returnVal = returnType->builtinType == BuiltinType::Void ? nullptr : parseReturnExpression();
    match(TokenType::TOK_RETURN);
    skipNewLines();

    consume(TokenType::TOK_RIGHT_BRACE, "Expected '}' after function body");

    return std::make_unique<FunctionDefinition>(
        std::make_unique<Identifier>(name.lexeme),
        std::make_unique<ParameterList>(
            std::move(parameters)
        ), std::move(returnType), 
        std::move(body),
        std::move(returnVal) );

}

std::unique_ptr<Type> Parser::parseType(){

    if(isTypeToken(peek()) == false){
        throw std::runtime_error("Expected type token");
    }
    auto typeStr = tokenTypeToBuiltin(peek());
    advance();
    return std::make_unique<Type>(typeStr);
}

std::vector<std::unique_ptr<Statement>> Parser::parseStatementList(){
    std::vector<std::unique_ptr<Statement>> statements;
    while(!isAtEnd()){

        auto statement = parseStatement();
        if(statement == nullptr){

            return statements;
        }
        if(!match(TokenType::TOK_NEWLINE)){
            error("Expected newline after statement", peek().line, peek().column);
        }

        statements.push_back(std::move(statement));
    } 
    return statements;
}

std::unique_ptr<Statement> Parser::parseStatement(){

    if(isTypeToken(peek())){

        return parseVariableDeclaration();
    }if(check(TokenType::TOK_IDENTIFIER)){
        return std::make_unique<ExpressionStatement>(parseFunctionCall());
    }
    return nullptr;
}

std::unique_ptr<ReturnExpression> Parser::parseReturnExpression(){
    consume(TokenType::TOK_RETURN, "Expected 'return' keyword");
    auto returnValue = parseExpression();
    return std::make_unique<ReturnExpression>(std::move(returnValue));
}

std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration(){
    auto type = parseType();
    Lexer::Token name = consume(TokenType::TOK_IDENTIFIER, "Expected variable name");

    if(match(TokenType::TOK_ASSIGN)){

        auto initializer = parseExpression();
        if(!initializer){
            error("Expected expression after '='", peek().line, peek().column);
            synchronize();
            return nullptr;
        }

        return std::make_unique<VariableDeclaration>(std::move(type), 
        std::make_unique<Identifier>(name.lexeme), std::move(initializer));
    }
    return std::make_unique<VariableDeclaration>(std::move(type), 
    std::make_unique<Identifier>(name.lexeme), nullptr);
}

/**
 * @brief Inicia el análisis de una expresión.
 * @details Esta función es la entrada para el análisis de expresiones y se apoya
 * en otros métodos (por ejemplo, parseLogicalOr) para construir la jerarquía de la expresión.
 *
 * @return std::unique_ptr<Expression> Un puntero único a la expresión parseada.
 */
std::unique_ptr<Expression> Parser::parseExpression(){
    return parseLogicalOr();
}

/**
 * @brief Parsea una expresión lógica OR.
 * @details Esta función parsea expresiones que involucran el operador lógico OR. 
 * Llama a parseLogicalAnd() para la parte izquierda y, mientras se encuentre el token OR,
 * construye de forma recursiva un objeto BinaryExpression que representa la operación.
 *
 * @return std::unique_ptr<Expression> Un puntero único a un objeto Expression que representa 
 * la expresión lógica OR resultante.
 */
std::unique_ptr<Expression> Parser::parseLogicalOr(){

    auto expr = parseLogicalAnd();
    while(match(TokenType::TOK_OR)){
        auto op = previous();
        auto right = parseLogicalAnd();
        expr = std::make_unique<BinaryExpression>(op.lexeme, std::move(expr), std::move(right));
    }
    return expr;
}

/**
 * @brief Parsea una expresión lógica AND.
 * @details Esta función se encarga de procesar expresiones que utilizan el operador lógico AND.
 * A partir de una expresión de igualdad (con parseEquality()), se construye un árbol
 * de expresiones BinaryExpression mientras se encuentren tokens AND.
 *
 * @return std::unique_ptr<Expression> Un puntero único a la expresión lógica AND resultante.
 */
std::unique_ptr<Expression> Parser::parseLogicalAnd(){
    auto expr = parseEquality();
    while(match(TokenType::TOK_AND)){
        auto op = previous();
        auto right = parseEquality();
        expr = std::make_unique<BinaryExpression>(op.lexeme, std::move(expr), std::move(right));
    }
    return expr;
}

/**
 * @brief Parsea una expresión de igualdad.
 * @details Procesa expresiones que comparan la igualdad utilizando el token TOK_EQUAL. 
 * A partir de una expresión relacional (obtenida con parseRelational), se construye
 * un árbol de BinaryExpression que representa todas las comparaciones de igualdad consecutivas.
 *
 * @return std::unique_ptr<Expression> Un puntero único a la expresión de igualdad resultante.
 */
std::unique_ptr<Expression> Parser::parseEquality(){

    auto expr = parseRelational();
    while(match(TokenType::TOK_EQUAL)){
        auto op = previous();
        auto right = parseRelational();
        expr = std::make_unique<BinaryExpression>(op.lexeme, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseRelational(){

    auto expr = parseAditive();
    while(check(TokenType::TOK_LESS) || check(TokenType::TOK_GREATER) || check(TokenType::TOK_LESS_EQ) || check(TokenType::TOK_GREATER_EQ)){
        auto op = advance();
        auto right = parseAditive();
        expr = std::make_unique<BinaryExpression>(op.lexeme, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseAditive(){

    auto left = parseMultiplicative();
    while(check(TokenType::TOK_ADD) || check(TokenType::TOK_MINUS)){
        auto op = advance();
        auto right = parseMultiplicative();

        left = std::make_unique<BinaryExpression>(op.lexeme, std::move(left), std::move(right));
    }
    return left;
}

std::unique_ptr<Expression> Parser::parseMultiplicative(){

    auto expr = parseUnary();

    while(match(TokenType::TOK_MULT) || match(TokenType::TOK_DIV) || match(TokenType::TOK_MOD)){
        auto op = previous();
        auto right = parseUnary();
        expr = std::make_unique<BinaryExpression>(op.lexeme, std::move(expr), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expression> Parser::parseUnary(){
    if(check(TokenType::TOK_PTR) || check(TokenType::TOK_REF) || check(TokenType::TOK_ACCESS)){
        auto op = advance();
        auto operand = parsePrimary();
        return std::make_unique<UnaryExpression>(op.lexeme, std::move(operand));
    }
    return parsePrimary();
}

std::unique_ptr<Expression> Parser::parsePrimary(){

    if(check(TokenType::TOK_IDENTIFIER)){
        if(lookAhead(1).type == TokenType::TOK_LEFT_PAREN){
            return parseFunctionCall();
        }
        return parseIdentifier();

    }if(check(TokenType::TOK_NUMBER) 
    || check(TokenType::TOK_CHAR)   
    || check(TokenType::TOK_STRING_LITERAL)
    || check(TokenType::TOK_TRUE)
    || check(TokenType::TOK_FALSE)){
        return parseLiteral();
    }if(check(TokenType::TOK_LEFT_PAREN)){
        advance();
        auto expr = parseExpression();
        consume(TokenType::TOK_RIGHT_PAREN, "Expected ')' after expression");
        return std::make_unique<PrimaryExpression>(std::move(expr));
    }
    return nullptr;
}

std::unique_ptr<Literal> Parser::parseLiteral(){

    auto literal = advance();

    switch (literal.type){
        case TokenType::TOK_NUMBER:
            return std::make_unique<NumericLiteral>(std::stoi(literal.lexeme), BuiltinType::Int);
        case TokenType::TOK_FLOAT:
            return std::make_unique<NumericLiteral>(std::stof(literal.lexeme), BuiltinType::Float);
        case TokenType::TOK_CHAR:
            return std::make_unique<CharLiteral>(literal.lexeme[0]);
        case TokenType::TOK_STRING_LITERAL:

            return std::make_unique<StringLiteral>(literal.lexeme);
        default:
            if(literal.type == TokenType::TOK_TRUE || literal.type == TokenType::TOK_FALSE){
                return std::make_unique<BooleanLiteral>(literal.type == TokenType::TOK_TRUE);
            }
            else{
                error("Expected literal", literal.line, literal.column);
                return nullptr;
            }
    }
}

//<function_call> ::= <identifier> "(" [ <argument_list> ] ")"
std::unique_ptr<Expression> Parser::parseFunctionCall(){
    auto id = parseIdentifier();
    consume(TokenType::TOK_LEFT_PAREN, "Expected ( in expression");
    std::vector<std::unique_ptr<Expression>> args;
    
    skipNewLines();
    
    if (!check(TokenType::TOK_RIGHT_PAREN)) {
        do {
            auto arg = parseExpression();
            if (arg != nullptr) {
                args.push_back(std::move(arg));
            } else {
                break;
            }

            if (match(TokenType::TOK_COMMA)) {
                skipNewLines();
            }
        } while(!check(TokenType::TOK_RIGHT_PAREN) && !isAtEnd());
    }

    skipNewLines();
    consume(TokenType::TOK_RIGHT_PAREN, "Expected ) in expression");

    auto functionCall = std::make_unique<FunctionCall>(std::move(id), std::move(args));
    
    return std::make_unique<PrimaryExpression>(std::move(functionCall));
}

/**
 * @brief Parsea un identificador y lo envuelve en un objeto Identifier.
 * @details Esta función consume el token actual, que se espera sea de tipo
 * TOK_IDENTIFIER, y crea una instancia de Identifier con su lexema. Si el token
 * no es un identificador, se genera un error (esperado que ya haya sido verificado previamente).
 *
 * @return std::unique_ptr<Identifier> Un puntero único a un objeto Identifier con el lexema del token consumido.
 */
std::unique_ptr<Identifier> Parser::parseIdentifier(){

    auto id = consume(TokenType::TOK_IDENTIFIER, "Expected identifier");
    return std::make_unique<Identifier>(id.lexeme);
}

/**
 * @brief Registra un error sintáctico en el ErrorManager.
 * @details Toma un mensaje de error, número de línea y columna y crea un objeto CompilerError
 * que es agregado al ErrorManager. Esta función sirve para centralizar la notificación de errores
 * durante el análisis sintáctico.
 *
 * @param message Descripción del error.
 * @param line Número de línea donde se produjo el error.
 * @param column Columna en la línea donde se detectó el error.
 */
void Parser::error(const std::string& message, int line, int column) {
    errorManager->addError(std::make_unique<CompilerError>(ErrorType::SYNTACTIC, message, line, column));
}

/**
 * @brief Intenta sincronizar el parser después de un error de análisis.
 * @details Avanza el iterador de tokens hasta encontrar un punto de sincronización.
 * Se utiliza para evitar cascadas de errores cuando se ha detectado un fallo en la sintaxis.
 * Considera terminar la sincronización si se encuentra un salto de línea o ciertos tokens clave 
 * como TOK_FUNC, TOK_IF, TOK_ELSEIF, TOK_ELSE, TOK_REPEAT o TOK_RETURN.
 */
void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::TOK_NEWLINE) {
            return;
        }
        switch (peek().type) {
            case TokenType::TOK_FUNC:
            case TokenType::TOK_IF:
            case TokenType::TOK_ELSEIF:
            case TokenType::TOK_ELSE:
            case TokenType::TOK_REPEAT:
            case TokenType::TOK_RETURN:
                return;
            default:
                break;
        }
        advance();
    }
}

}
