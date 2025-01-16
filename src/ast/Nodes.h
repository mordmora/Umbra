#ifndef AST_NODES_HPP
#define AST_NODES_HPP

#include <string>
#include <vector>
#include <memory>
#include "ASTNode.h"

namespace umbra {


    // Forward declarations
    class FunctionDefinition;
    class ParameterList;
    class Statement;
    class Expression;
    class Type;
    class Identifier;
    class Literal;

    // Program node
    class ProgramNode : public ASTNode {
    public:
        void accept(ASTVisitor& visitor) override {}
        std::vector<std::unique_ptr<FunctionDefinition>> functions;
    };

    // Function definition node
    class FunctionDefinition : public ASTNode {
    public:
        void accept(ASTVisitor& visitor) override {}
        std::unique_ptr<Identifier> name;
        std::unique_ptr<ParameterList> parameters;
        std::unique_ptr<Type> returnType;
        std::vector<std::unique_ptr<Statement>> body;
    };

    // Parameter list node
    class ParameterList : public ASTNode {
    public:
        void accept(ASTVisitor& visitor) override {}
        std::vector<std::pair<std::unique_ptr<Type>, std::unique_ptr<Identifier>>> parameters;
    };

    // Type node
    class Type : public ASTNode {
    public:
        void accept(ASTVisitor& visitor) override {}
        std::string baseType; // e.g., int, float, etc.
        int arrayDimensions = 0; // Number of dimensions if it's an array
    };

    // Identifier node
    class Identifier : public ASTNode {
    public:
        void accept(ASTVisitor& visitor) override {}
        std::string name;
    };

    // Statement base class
    class Statement : public ASTNode {};

    // Variable declaration node
    class VariableDeclaration : public Statement {
    public:
        void accept(ASTVisitor& visitor) override {}
        std::unique_ptr<Type> type;
        std::unique_ptr<Identifier> name;
        std::unique_ptr<Expression> initializer;
    };

    // Assignment statement node
    class AssignmentStatement : public Statement {
    public:
        void accept(ASTVisitor& visitor) override {}
        std::unique_ptr<Identifier> target;
        std::unique_ptr<Expression> index; // For array assignment
        std::unique_ptr<Expression> value;
    };

    // Conditional statement node
    class Conditional : public Statement {
    public:
        void accept(ASTVisitor& visitor) override {}
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
        void accept(ASTVisitor& visitor) override {}
        std::unique_ptr<Expression> condition;
        std::vector<std::unique_ptr<Statement>> body;
        bool isRepeatTimes = false; // true if "repeat N times", false if "repeat if"
    };

    // Memory management statement node
    class MemoryManagement : public Statement {
    public:
        void accept(ASTVisitor& visitor) override {}
        enum ActionType { ALLOCATE, DEALLOCATE } action;
        std::unique_ptr<Type> type;
        std::unique_ptr<Expression> size; // For allocation
        std::unique_ptr<Identifier> target; // For deallocation
    };

    // Return statement node
    class ReturnStatement : public Statement {
    public:
        void accept(ASTVisitor& visitor) override {}
        std::unique_ptr<Expression> returnValue;
    };

    // Expression base class
    class Expression : public ASTNode {};

    // Binary expression node
    class BinaryExpression : public Expression {
    public:
        void accept(ASTVisitor& visitor) override {}
        std::string op; // Operator (e.g., +, -, *, etc.)
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;
    };

    // Unary expression node
    class UnaryExpression : public Expression {
    public:

        void accept(ASTVisitor& visitor) override {}
        std::string op; // Operator (e.g., ptr, ref, access)
        std::unique_ptr<Expression> operand;
    };

    // Primary expression node
    class PrimaryExpression : public Expression {
    public:
        void accept(ASTVisitor& visitor) override {}
        enum Type { IDENTIFIER, LITERAL, FUNCTION_CALL, PARENTHESIZED } exprType;
        std::unique_ptr<Identifier> identifier;
        std::unique_ptr<Literal> literal;
        std::unique_ptr<Expression> parenthesized;
        std::vector<std::unique_ptr<Expression>> functionArguments;
    };

    // Literal node
    class Literal : public ASTNode {
    public:
        void accept(ASTVisitor& visitor) override {}
        enum Type { INTEGER, FLOAT, BOOLEAN, CHAR, STRING } literalType;
        std::string value;
    };

    // Utility nodes for other elements
    class FunctionCall : public Expression {
    public:
        void accept(ASTVisitor& visitor) override {}
        std::unique_ptr<Identifier> functionName;
        std::vector<std::unique_ptr<Expression>> arguments;
    };

    // Numeric literal node
    class NumericLiteral : public Literal {
    public:
        void accept(ASTVisitor& visitor) override {}
        double value;
    };

    // Boolean literal node
    class BooleanLiteral : public Literal {
    public:
        void accept(ASTVisitor& visitor) override {}
        bool value;
    };

    // Char literal node
    class CharLiteral : public Literal {
    public:
        void accept(ASTVisitor& visitor) override {}
        char value;
    };

    // String literal node
    class StringLiteral : public Literal {
    public:
        void accept(ASTVisitor& visitor) override {}
        std::string value;
    };
}
#endif // AST_NODES_HPP
