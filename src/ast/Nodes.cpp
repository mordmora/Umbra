/**
 * @file Nodes.cpp
 * @brief Implementation of Abstract Syntax Tree nodes for the Umbra compiler
 * @details Contains implementations for all AST node types used in parsing
 * @date 2023
 */

#include "Nodes.h"

namespace umbra {

    /**
     * @brief Constructs a program node containing function definitions
     * @param functions Vector of function definitions that make up the program
     */
    ProgramNode::ProgramNode(std::vector<std::unique_ptr<FunctionDefinition>> functions) 
        : functions(std::move(functions)) {}

    /**
     * @brief Constructs a function definition node
     * @param name Function identifier
     * @param parameters List of function parameters
     * @param returnType Function return type
     * @param body Vector of function statements
     */
    FunctionDefinition::FunctionDefinition(std::unique_ptr<Identifier> name, 
        std::unique_ptr<ParameterList> parameters,
        std::unique_ptr<Type> returnType, 
        std::vector<std::unique_ptr<Statement>> body)
        : name(std::move(name)), parameters(std::move(parameters)), 
          returnType(std::move(returnType)), body(std::move(body)) {}

    /**
     * @brief Constructs a parameter list node
     * @param parameters Vector of parameter pairs (type and identifier)
     */
    ParameterList::ParameterList(
        std::vector<std::pair<std::unique_ptr<Type>, std::unique_ptr<Identifier>>> parameters) 
        : parameters(std::move(parameters)) {}

    /**
     * @brief Constructs a type node
     * @param baseType Base type name
     * @param arrayDimensions Number of array dimensions (0 for non-array types)
     */
    Type::Type(std::string baseType, int arrayDimensions) 
        : baseType(std::move(baseType)), arrayDimensions(arrayDimensions) {}

    /**
     * @brief Constructs an identifier node
     * @param name Identifier name
     */
    Identifier::Identifier(std::string name) : name(std::move(name)) {}

    /**
     * @brief Constructs a variable declaration node
     * @param type Variable type
     * @param name Variable identifier
     * @param initializer Initial value expression (can be nullptr)
     */
    VariableDeclaration::VariableDeclaration(std::unique_ptr<Type> type, 
        std::unique_ptr<Identifier> name,
        std::unique_ptr<Expression> initializer)
        : type(std::move(type)), name(std::move(name)), 
          initializer(std::move(initializer)) {}

    /**
     * @brief Constructs an assignment statement node
     * @param target Target identifier
     * @param value Value expression to assign
     */
    AssignmentStatement::AssignmentStatement(std::unique_ptr<Identifier> target, 
        std::unique_ptr<Expression> value)
        : target(std::move(target)), value(std::move(value)) {}

    /**
     * @brief Constructs a conditional statement node
     * @param branches Vector of condition-statement pairs for if/else-if branches
     * @param elseBranch Vector of statements for the else branch
     */
    Conditional::Conditional(
        std::vector<std::pair<std::unique_ptr<Expression>, std::vector<std::unique_ptr<Statement>>>> branches,
        std::vector<std::unique_ptr<Statement>> elseBranch)
        : elseBranch(std::move(elseBranch)) {
        for (auto& branch : branches) {
            this->branches.push_back(Branch{
                std::move(branch.first),
                std::move(branch.second)
            });
        }
    }

    /**
     * @brief Constructs a loop statement node
     * @param condition Loop continuation condition
     * @param body Vector of loop body statements
     */
    Loop::Loop(std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Statement>> body) 
        : condition(std::move(condition)), body(std::move(body)) {}

    /**
     * @brief Constructs a memory management node
     * @param action Type of memory operation (allocate/deallocate)
     * @param type Type being allocated
     * @param size Size expression for allocation
     * @param target Target identifier for operation
     */
    MemoryManagement::MemoryManagement(ActionType action, std::unique_ptr<Type> type, 
        std::unique_ptr<Expression> size, std::unique_ptr<Identifier> target) 
        : action(action), type(std::move(type)), size(std::move(size)), target(std::move(target)) {}

    /**
     * @brief Constructs a return statement node
     * @param returnValue Expression to return
     */
    ReturnStatement::ReturnStatement(std::unique_ptr<Expression> returnValue) 
        : returnValue(std::move(returnValue)) {}

    /**
     * @brief Constructs a function call node
     * @param functionName Name of the function being called
     * @param arguments Vector of argument expressions
     */
    FunctionCall::FunctionCall(std::unique_ptr<Identifier> functionName, 
        std::vector<std::unique_ptr<Expression>> arguments) 
        : functionName(std::move(functionName)), arguments(std::move(arguments)) {}

    /**
     * @brief Constructs a literal node
     * @param value String representation of the literal value
     */
    Literal::Literal(std::string value) 
        : value(std::move(value)) {}

    /**
     * @brief Constructs a numeric literal node
     * @param value Numeric value of the literal
     */

    NumericLiteral::NumericLiteral(double value) : Literal(std::to_string(value)), value(value) {}

    /**
     * @brief Constructs a boolean literal node
     * @param value Boolean value of the literal
     */

    BooleanLiteral::BooleanLiteral(bool value) : Literal(value ? "true" : "false"), value(value) {}

    /**
     * @brief Constructs a character literal node
     * @param value Character value of the literal
     */

    CharLiteral::CharLiteral(char value) : Literal(std::string(1, value)), value(value) {}

    /**
     * @brief Constructs a string literal node
     * @param value String value of the literal
     */

    StringLiteral::StringLiteral(std::string value) : Literal(std::move(value)) {}

    /**
     * @brief Constructs a binary expression node
     * @param op Operator string
     * @param left Left operand expression
     * @param right Right operand expression
     */

    BinaryExpression::BinaryExpression(std::string op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right) :
        op(std::move(op)), left(std::move(left)), right(std::move(right)) {
    }

    /**
     * @brief Constructs a unary expression node
     * @param op Operator string
     * @param operand Operand expression
     */

    UnaryExpression::UnaryExpression(std::string op, std::unique_ptr<Expression> operand) :
        op(std::move(op)), operand(std::move(operand)) {
    }

    /**
     * @brief Constructs a primary expression node
     * @param identifier Identifier expression
     */

    PrimaryExpression::PrimaryExpression(std::unique_ptr<Identifier> identifier)
        : exprType(IDENTIFIER), identifier(std::move(identifier)) {}

    /**
     * @brief Constructs a primary expression node
     * @param literal Literal expression
     */

    PrimaryExpression::PrimaryExpression(std::unique_ptr<Literal> literal)
        : exprType(LITERAL), literal(std::move(literal)) {}

    /**
     * @brief Constructs a primary expression node
     * @param parenthesized Parenthesized expression
     */

    PrimaryExpression::PrimaryExpression(std::unique_ptr<Expression> parenthesized)
        : exprType(PARENTHESIZED), parenthesized(std::move(parenthesized)) {}
    
} // namespace umbra
