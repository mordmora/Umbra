#ifndef ASTNODE_H
#define ASTNODE_H

namespace umbra {

enum class NodeKind {
    // Program and structure
    PROGRAM,
    FUNCTION_DEFINITION,
    PARAMETER_LIST,
    TYPE,

    // Statements
    VARIABLE_DECLARATION,
    ASSIGNMENT_STATEMENT,
    IF_STATEMENT,
    REPEAT_TIMES_STATEMENT,
    REPEAT_IF_STATEMENT,
    EXPRESSION_STATEMENT,
    MEMORY_MANAGEMENT,

    // Expressions
    IDENTIFIER,
    BINARY_EXPRESSION,
    UNARY_EXPRESSION,
    PRIMARY_EXPRESSION,
    FUNCTION_CALL,
    RETURN_EXPRESSION,
    ARRAY_ACCESS_EXPRESSION,
    TERNARY_EXPRESSION,
    CAST_EXPRESSION,
    MEMBER_ACCESS_EXPRESSION,
    PARENTHESIZED,

    // Literals
    LITERAL,
    NUMERIC_LITERAL,
    BOOLEAN_LITERAL,
    CHAR_LITERAL,
    STRING_LITERAL
};

class ASTVisitor;

class ASTNode {
  public:
    virtual ~ASTNode() = default;
    ASTNode(NodeKind kind) : kind(kind) {}
    NodeKind kind;

    NodeKind getKind() const { return kind; }

    // Funciones auxiliares para verificar tipos de nodos
    bool isExpression() const {
        return kind >= NodeKind::IDENTIFIER && kind <= NodeKind::STRING_LITERAL;
    }

    bool isStatement() const {
        return kind >= NodeKind::VARIABLE_DECLARATION && kind <= NodeKind::MEMORY_MANAGEMENT;
    }

    bool isLiteral() const {
        return kind >= NodeKind::LITERAL && kind <= NodeKind::STRING_LITERAL;
    }

    const char* getNodeTypeName() const {
        switch (kind) {
            case NodeKind::PROGRAM: return "Program";
            case NodeKind::FUNCTION_DEFINITION: return "FunctionDefinition";
            case NodeKind::PARAMETER_LIST: return "ParameterList";
            case NodeKind::TYPE: return "Type";
            case NodeKind::VARIABLE_DECLARATION: return "VariableDeclaration";
            case NodeKind::ASSIGNMENT_STATEMENT: return "AssignmentStatement";
            case NodeKind::IF_STATEMENT: return "IfStatement";
            case NodeKind::REPEAT_TIMES_STATEMENT: return "RepeatTimesStatement";
            case NodeKind::REPEAT_IF_STATEMENT: return "RepeatIfStatement";
            case NodeKind::EXPRESSION_STATEMENT: return "ExpressionStatement";
            case NodeKind::MEMORY_MANAGEMENT: return "MemoryManagement";
            case NodeKind::IDENTIFIER: return "Identifier";
            case NodeKind::BINARY_EXPRESSION: return "BinaryExpression";
            case NodeKind::UNARY_EXPRESSION: return "UnaryExpression";
            case NodeKind::PRIMARY_EXPRESSION: return "PrimaryExpression";
            case NodeKind::FUNCTION_CALL: return "FunctionCall";
            case NodeKind::RETURN_EXPRESSION: return "ReturnExpression";
            case NodeKind::ARRAY_ACCESS_EXPRESSION: return "ArrayAccessExpression";
            case NodeKind::TERNARY_EXPRESSION: return "TernaryExpression";
            case NodeKind::CAST_EXPRESSION: return "CastExpression";
            case NodeKind::MEMBER_ACCESS_EXPRESSION: return "MemberAccessExpression";
            case NodeKind::LITERAL: return "Literal";
            case NodeKind::NUMERIC_LITERAL: return "NumericLiteral";
            case NodeKind::BOOLEAN_LITERAL: return "BooleanLiteral";
            case NodeKind::CHAR_LITERAL: return "CharLiteral";
            case NodeKind::STRING_LITERAL: return "StringLiteral";
            default: return "Unknown";
        }
    }

};

} // namespace umbra

#endif // ASTNODE_H
