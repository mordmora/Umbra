


#ifndef AST_NODES_HPP
#define AST_NODES_HPP

#include <string>
#include <vector>
#include <memory>
#include "umbra/ast/ASTNode.h"
#include "Types.h"

#include<iostream>

namespace umbra {
    class SemanticVisitor;  // Forward declaration
    // Forward declarations
    class FunctionDefinition;
    class ParameterList;
    class Statement;
    class Type;
    class Identifier;
    class Literal;
    class FunctionCall;
    class ArrayAccessExpression;
    class TernaryExpression;
    class CastExpression;
    class MemberAccessExpression;
    class ReturnExpression;

    class Symbol;
    // Expression base class
    class Expression : public ASTNode {
    public:
        Expression(NodeKind kind) : ASTNode(kind) {}


        void setKind(NodeKind k){
            this->kind = k;
        }

         BuiltinType builtinExpressionType = BuiltinType::None;
    };

    // Program node
    class ProgramNode : public ASTNode {
    public:
        ProgramNode(std::vector<std::unique_ptr<FunctionDefinition>> functions)
            : ASTNode(NodeKind::PROGRAM), functions(std::move(functions)) {}

        std::vector<std::unique_ptr<FunctionDefinition>> functions;
    };

    // Function definition node
    class FunctionDefinition : public ASTNode {
    public:
        FunctionDefinition(std::unique_ptr<Identifier> name, std::unique_ptr<ParameterList> parameters,
            std::unique_ptr<Type> returnType, std::vector<std::unique_ptr<Statement>> body,
            std::unique_ptr<ReturnExpression> returnValue)
            : ASTNode(NodeKind::FUNCTION_DEFINITION), name(std::move(name)),
              parameters(std::move(parameters)), returnType(std::move(returnType)),
              body(std::move(body)), returnValue(std::move(returnValue)) {}

        std::unique_ptr<Identifier> name;
        std::unique_ptr<ParameterList> parameters;
        std::unique_ptr<Type> returnType;
        std::unique_ptr<ReturnExpression> returnValue;
        std::vector<std::unique_ptr<Statement>> body;
    };

    // Parameter list node
    class ParameterList : public ASTNode {
    public:
        ParameterList(std::vector<std::pair<std::unique_ptr<Type>, std::unique_ptr<Identifier>>> parameters)
            : ASTNode(NodeKind::PARAMETER_LIST), parameters(std::move(parameters)) {}

        std::vector<std::pair<std::unique_ptr<Type>, std::unique_ptr<Identifier>>> parameters;
    };

    // Type node
    class Type : public ASTNode {
    public:
        Type(BuiltinType builtinType, int arrayDimensions = 0)
            : ASTNode(NodeKind::TYPE), builtinType(builtinType), arrayDimensions(arrayDimensions) {}

        BuiltinType builtinType;
        int arrayDimensions = 0; // Number of dimensions if it's an array

    };

    // Identifier node
    class Identifier : public Expression {
    public:
        Symbol* resolvedSymbol = nullptr; // Pointer to the resolved symbol
        Identifier(std::string name) : Expression(NodeKind::IDENTIFIER), name(std::move(name)) {}

        std::string name;
    };

    // Statement base class
    class Statement : public ASTNode {
    public:
        Statement(NodeKind kind) : ASTNode(kind) {}

    };

    // Variable declaration node
    class VariableDeclaration : public Statement {
    public:
        VariableDeclaration(std::unique_ptr<Type> type, std::unique_ptr<Identifier> name,
            std::unique_ptr<Expression> initializer)
            : Statement(NodeKind::VARIABLE_DECLARATION), type(std::move(type)),
              name(std::move(name)), initializer(std::move(initializer)) {}


        std::unique_ptr<Type> type;
        std::unique_ptr<Identifier> name;
        std::unique_ptr<Expression> initializer;
    };

    // Assignment statement node
    class AssignmentStatement : public Statement {
    public:
        AssignmentStatement(std::unique_ptr<Identifier> target, std::unique_ptr<Expression> value)
            : Statement(NodeKind::ASSIGNMENT_STATEMENT), target(std::move(target)), value(std::move(value)) {}

        std::unique_ptr<Identifier> target;
        std::unique_ptr<Expression> index; // For array assignment
        std::unique_ptr<Expression> value;
    };

    // IfStatement statement node
    struct Branch {
        std::unique_ptr<Expression> condition;
        std::vector<std::unique_ptr<Statement>> body;
    };

    class IfStatement : public Statement {
    public:
        IfStatement(std::vector<Branch> branches,
            std::vector<std::unique_ptr<Statement>> elseBranch)
            : Statement(NodeKind::IF_STATEMENT), branches(std::move(branches)), elseBranch(std::move(elseBranch)) {}


        std::vector<Branch> branches;
        std::vector<std::unique_ptr<Statement>> elseBranch;
    };


    // Memory management statement node
    class MemoryManagement : public Statement {
    public:
        enum ActionType { ALLOCATE, DEALLOCATE };
        MemoryManagement(ActionType action, std::unique_ptr<Type> type, std::unique_ptr<Expression> size,
            std::unique_ptr<Identifier> target) : Statement(NodeKind::MEMORY_MANAGEMENT),
                                                action(action),
                                                type(std::move(type)),
                                                size(std::move(size)) {}

        ActionType action;
        std::unique_ptr<Type> type;
        std::unique_ptr<Expression> size; // For allocation
        std::unique_ptr<Identifier> target; // For deallocation
    };

    // Repeat times statement node (For)
    class RepeatTimesStatement : public Statement {
    public:
        RepeatTimesStatement(std::unique_ptr<Expression> times,
                            std::vector<std::unique_ptr<Statement>> body) : Statement(NodeKind::REPEAT_TIMES_STATEMENT),
                                                                            times(std::move(times)),
                                                                            body(std::move(body)) {}

        std::unique_ptr<Expression> times;
        std::vector<std::unique_ptr<Statement>> body;
    };

    // Repeat if statement node (While)
    class RepeatIfStatement : public Statement {
    public:
        RepeatIfStatement(std::unique_ptr<Expression> condition,
                            std::vector<std::unique_ptr<Statement>> body) : Statement(NodeKind::REPEAT_IF_STATEMENT),
                                                                            condition(std::move(condition)),
                                                                            body(std::move(body)) {}

        std::unique_ptr<Expression> condition;
        std::vector<std::unique_ptr<Statement>> body;
    };

    // Return statement node
    class ReturnExpression : public Expression{
    public:
        ReturnExpression(std::unique_ptr<Expression> returnValue) : Expression(NodeKind::RETURN_EXPRESSION), returnValue(std::move(returnValue)) {}

        std::unique_ptr<Expression> returnValue;
    };

    // Binary expression node
    class BinaryExpression : public Expression {
    public:
        BinaryExpression(std::string op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
            : Expression(NodeKind::BINARY_EXPRESSION), op(std::move(op)),
              left(std::move(left)), right(std::move(right)) {}

        std::string op; // Operator (e.g., +, -, *, etc.)
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;
    };

    // Unary expression node
    class UnaryExpression : public Expression {
    public:
        UnaryExpression(std::string op, std::unique_ptr<Expression> operand)
            : Expression(NodeKind::UNARY_EXPRESSION), op(std::move(op)), operand(std::move(operand)) {}

        std::string op; // Operator (e.g., ptr, ref, access)
        std::unique_ptr<Expression> operand;
    };

    // Primary expression node
    class PrimaryExpression : public Expression {
    public:


        enum Type {
            IDENTIFIER,
            LITERAL,
            EXPRESSION_CALL,
            PARENTHESIZED,
            ARRAY_ACCESS,
            MEMBER_ACCESS,
            CAST_EXPRESSION,
            TERNARY_EXPRESSION,

        } exprType;

        // Builder
        PrimaryExpression(std::unique_ptr<Identifier> identifier) : Expression(NodeKind::IDENTIFIER), identifier(std::move(identifier)) {} ;
        PrimaryExpression(std::unique_ptr<Literal> literal) : Expression(NodeKind::LITERAL), literal(std::move(literal)) {} ;
        PrimaryExpression(std::unique_ptr<Expression> parenthesized) : Expression(NodeKind::PARENTHESIZED), parenthesized(std::move(parenthesized)) {};
        PrimaryExpression(std::unique_ptr<FunctionCall> functionCall) : Expression(NodeKind::FUNCTION_CALL), functionCall(std::move(functionCall)) {};
        PrimaryExpression(std::unique_ptr<ArrayAccessExpression> arrayAccess) : Expression(NodeKind::ARRAY_ACCESS_EXPRESSION), arrayAccess(std::move(arrayAccess)) {};
        PrimaryExpression(std::unique_ptr<MemberAccessExpression> memberAccess) : Expression(NodeKind::MEMBER_ACCESS_EXPRESSION), memberAccess(std::move(memberAccess)){};
        PrimaryExpression(std::unique_ptr<CastExpression> castExpr) : Expression(NodeKind::CAST_EXPRESSION), castExpression(std::move(castExpr)) {};
        PrimaryExpression(std::unique_ptr<TernaryExpression> ternaryExpr) : Expression(NodeKind::TERNARY_EXPRESSION), ternaryExpression(std::move(ternaryExpr)) {};

        // Members
        std::unique_ptr<Identifier> identifier;
        std::unique_ptr<Literal> literal;
        std::unique_ptr<Expression> parenthesized;
        std::unique_ptr<FunctionCall> functionCall;
        std::unique_ptr<ArrayAccessExpression> arrayAccess;
        std::unique_ptr<MemberAccessExpression> memberAccess;
        std::unique_ptr<CastExpression> castExpression;
        std::unique_ptr<TernaryExpression> ternaryExpression;
    };

    // Literal node
    class Literal : public Expression {
    public:

        BuiltinType builtinExpressionType;
        BuiltinType builtinType;
        Literal(NodeKind kind) : Expression(kind) {}

    };

    // Utility nodes for other elements
    class FunctionCall : public Expression {
    public:
        FunctionCall(std::unique_ptr<Identifier> functionName, std::vector<std::unique_ptr<Expression>> arguments) : Expression(NodeKind::FUNCTION_CALL),
        functionName(std::move(functionName)),
        arguments(std::move(arguments)) {};

        std::unique_ptr<Identifier> functionName;
        std::vector<std::unique_ptr<Expression>> arguments;
    };

    class ExpressionStatement : public Statement {
        public:
        ExpressionStatement(std::unique_ptr<Expression> exp) : Statement(NodeKind::EXPRESSION_STATEMENT), exp(std::move(exp)) {}

        std::unique_ptr<Expression> exp;
    };

    // Numeric literal node
    class NumericLiteral : public Literal {
    public:
        BuiltinType builtinType;
        NumericLiteral(double value, BuiltinType numericType) : Literal(NodeKind::NUMERIC_LITERAL), value(value) {}

        double value;
    };

    // Boolean literal node
    class BooleanLiteral : public Literal {
    public:
        BooleanLiteral(bool value) : Literal(NodeKind::BOOLEAN_LITERAL), value(value) {}

        bool value;
    };

    // Char literal node
    class CharLiteral : public Literal {
    public:
        CharLiteral(char value) : Literal(NodeKind::CHAR_LITERAL), value(value) {};

        char value;
    };

    // String literal node
    class StringLiteral : public Literal {
    public:
        StringLiteral(const std::string& value) : Literal(NodeKind::STRING_LITERAL), value(std::move(value)) {};

        std::string value;
    };

    // Array access expression node
    class ArrayAccessExpression : public Expression {
    public:
        ArrayAccessExpression(std::unique_ptr<Expression> array,
                            std::unique_ptr<Expression> index) : Expression(NodeKind::ARRAY_ACCESS_EXPRESSION),
                                                                 array(std::move(array)), index(std::move(index)){};

        std::unique_ptr<Expression> array;
        std::unique_ptr<Expression> index;
    };

    // Ternary conditional expression node
    class TernaryExpression : public Expression {
    public:
        TernaryExpression(std::unique_ptr<Expression> condition,
                        std::unique_ptr<Expression> trueExpr,
                        std::unique_ptr<Expression> falseExpr) : Expression(NodeKind::TERNARY_EXPRESSION), condition(std::move(condition)),
                                                                 trueExpr(std::move(trueExpr)),
                                                                 falseExpr(std::move(falseExpr)) {};

        std::unique_ptr<Expression> condition;
        std::unique_ptr<Expression> trueExpr;
        std::unique_ptr<Expression> falseExpr;
    };

    // Cast expression node
    class CastExpression : public Expression {
    public:
        CastExpression(std::unique_ptr<Type> targetType,
                    std::unique_ptr<Expression> expression) : Expression(NodeKind::CAST_EXPRESSION),
                                                              targetType(std::move(targetType)),
                                                              expression(std::move(expression)) {};

        std::unique_ptr<Type> targetType;
        std::unique_ptr<Expression> expression;
    };

    class MemberAccessExpression : public Expression {
    public:
        MemberAccessExpression(std::unique_ptr<Expression> object,
                            std::unique_ptr<Identifier> member) : Expression(NodeKind::MEMBER_ACCESS_EXPRESSION),
                                                                  object(std::move(object)), member(std::move(member)) {};

        std::unique_ptr<Expression> object;
        std::unique_ptr<Identifier> member;
    };



};

#endif // AST_NODES_HPP
