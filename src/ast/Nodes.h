#ifndef AST_NODES_HPP
#define AST_NODES_HPP

#include <string>
#include <vector>
#include <memory>
#include "ASTNode.h"
#include "ASTVisitor.h"

namespace umbra {


    // Forward declarations
    class FunctionDefinition;
    class ParameterList;
    class Statement;
    class Type;
    class Identifier;
    class Literal;
    class FunctionCall;

    // Expression base class
    class Expression : public ASTNode {
    public:
        void accept(ASTVisitor& visitor) override = 0;
    };

    // Program node
    class ProgramNode : public ASTNode {
    public:
        ProgramNode(std::vector<std::unique_ptr<FunctionDefinition>> functions);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::vector<std::unique_ptr<FunctionDefinition>> functions;
    };

    // Function definition node
    class FunctionDefinition : public ASTNode {
    public:
        FunctionDefinition(std::unique_ptr<Identifier> name, std::unique_ptr<ParameterList> parameters,
            std::unique_ptr<Type> returnType, std::vector<std::unique_ptr<Statement>> body);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::unique_ptr<Identifier> name;
        std::unique_ptr<ParameterList> parameters;
        std::unique_ptr<Type> returnType;
        std::vector<std::unique_ptr<Statement>> body;
    };

    // Parameter list node
    class ParameterList : public ASTNode {
    public:
        ParameterList(std::vector<std::pair<std::unique_ptr<Type>, std::unique_ptr<Identifier>>> parameters);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::vector<std::pair<std::unique_ptr<Type>, std::unique_ptr<Identifier>>> parameters;
    };

    // Type node
    class Type : public ASTNode {
    public:
        Type(std::string baseType, int arrayDimensions = 0);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::string baseType; // e.g., int, float, etc.
        int arrayDimensions = 0; // Number of dimensions if it's an array
    };

    // Identifier node
    class Identifier : public Expression {
    public:
        Identifier(std::string name);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::string name;
    };

    // Statement base class
    class Statement : public ASTNode {
    public:
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
    };

    // Variable declaration node
    class VariableDeclaration : public Statement {
    public:
        VariableDeclaration(std::unique_ptr<Type> type, std::unique_ptr<Identifier> name,
            std::unique_ptr<Expression> initializer);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::unique_ptr<Type> type;
        std::unique_ptr<Identifier> name;
        std::unique_ptr<Expression> initializer;
    };

    // Assignment statement node
    class AssignmentStatement : public Statement {
    public:
        AssignmentStatement(std::unique_ptr<Identifier> target, std::unique_ptr<Expression> value);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::unique_ptr<Identifier> target;
        std::unique_ptr<Expression> index; // For array assignment
        std::unique_ptr<Expression> value;
    };

    // Conditional statement node
    class Conditional : public Statement {
    public:
        Conditional(std::vector<std::pair<std::unique_ptr<Expression>, std::vector<std::unique_ptr<Statement>>>> branches,
            std::vector<std::unique_ptr<Statement>> elseBranch);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        struct Branch {
            std::unique_ptr<Expression> condition;
            std::vector<std::unique_ptr<Statement>> body;
        };

        std::vector<Branch> branches;
        std::vector<std::unique_ptr<Statement>> elseBranch;
    };

    // Loop statement node
    class Loop : public Statement {
    public:
        Loop(std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<Statement>> body);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::unique_ptr<Expression> condition;
        std::vector<std::unique_ptr<Statement>> body;
        bool isRepeatTimes = false; // true if "repeat N times", false if "repeat if"
    };

    // Memory management statement node
    class MemoryManagement : public Statement {
    public:
        enum ActionType { ALLOCATE, DEALLOCATE };
        MemoryManagement(ActionType action, std::unique_ptr<Type> type, std::unique_ptr<Expression> size,
            std::unique_ptr<Identifier> target);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        ActionType action;
        std::unique_ptr<Type> type;
        std::unique_ptr<Expression> size; // For allocation
        std::unique_ptr<Identifier> target; // For deallocation
    };

    // Return statement node
    class ReturnStatement : public Statement {
    public:
        ReturnStatement(std::unique_ptr<Expression> returnValue);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::unique_ptr<Expression> returnValue;
    };

    // Binary expression node
    class BinaryExpression : public Expression {
    public:
        BinaryExpression(std::string op, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::string op; // Operator (e.g., +, -, *, etc.)
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;
    };

    // Unary expression node
    class UnaryExpression : public Expression {
    public:
        UnaryExpression(std::string op, std::unique_ptr<Expression> operand);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::string op; // Operator (e.g., ptr, ref, access)
        std::unique_ptr<Expression> operand;
    };

    // Primary expression node
    class PrimaryExpression : public Expression {
    public:
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

        enum Type { 
            IDENTIFIER, 
            LITERAL, 
            EXPRESSION_CALL, 
            PARENTHESIZED, 
            ARRAY_ACCESS, 
            MEMBER_ACCESS, 
            CAST_EXPRESSION, 
            TERNARY_EXPRESSION 
        } exprType;

        // Builder
        PrimaryExpression(std::unique_ptr<Identifier> identifier);
        PrimaryExpression(std::unique_ptr<Literal> literal);
        PrimaryExpression(std::unique_ptr<Expression> parenthesized);
        PrimaryExpression(std::unique_ptr<FunctionCall> functionCall);
        PrimaryExpression(std::unique_ptr<ArrayAccessExpression> arrayAccess);
        PrimaryExpression(std::unique_ptr<MemberAccessExpression> memberAccess);
        PrimaryExpression(std::unique_ptr<CastExpression> castExpr);
        PrimaryExpression(std::unique_ptr<TernaryExpression> ternaryExpr);

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
    class Literal : public ASTNode {
    public:

        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        enum Type { INTEGER, FLOAT, BOOLEAN, CHAR, STRING } literalType;

        Literal(std::string value);

        std::string value;
    };

    // Utility nodes for other elements
    class FunctionCall : public Expression {
    public:
        FunctionCall(std::unique_ptr<Identifier> functionName, std::vector<std::unique_ptr<Expression>> arguments);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::unique_ptr<Identifier> functionName;
        std::vector<std::unique_ptr<Expression>> arguments;
    };

    // Numeric literal node
    class NumericLiteral : public Literal {
    public:
        NumericLiteral(double value);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        double value;
    };

    // Boolean literal node
    class BooleanLiteral : public Literal {
    public:
        BooleanLiteral(bool value);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        bool value;
    };

    // Char literal node
    class CharLiteral : public Literal {
    public:
        CharLiteral(char value);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        char value;
    };

    // String literal node
    class StringLiteral : public Literal {
    public:
        StringLiteral(std::string value);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::string value;
    };

    // Array access expression node
    class ArrayAccessExpression : public Expression {
    public:
        ArrayAccessExpression(std::unique_ptr<Expression> array, 
                            std::unique_ptr<Expression> index);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::unique_ptr<Expression> array;
        std::unique_ptr<Expression> index;
    };

    // Ternary conditional expression node
    class TernaryExpression : public Expression {
    public:
        TernaryExpression(std::unique_ptr<Expression> condition,
                        std::unique_ptr<Expression> trueExpr,
                        std::unique_ptr<Expression> falseExpr);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Expression> trueExpr;
        std::unique_ptr<Expression> falseExpr;
    };

    // Cast expression node
    class CastExpression : public Expression {
    public:
        CastExpression(std::unique_ptr<Type> targetType,
                    std::unique_ptr<Expression> expression);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::unique_ptr<Type> targetType;
        std::unique_ptr<Expression> expression;
    };

    class MemberAccessExpression : public Expression {
    public:
        MemberAccessExpression(std::unique_ptr<Expression> object,
                            std::unique_ptr<Identifier> member);
        void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
        std::unique_ptr<Expression> object;
        std::unique_ptr<Identifier> member;
    };

};

#endif // AST_NODES_HPP
