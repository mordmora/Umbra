#ifndef UMBRA_AST_VISITOR_H
#define UMBRA_AST_VISITOR_H

namespace umbra {
/// Forward declarations
class ProgramNode;
class FunctionDefinition;
class ParameterList;
class Type;
class Identifier;
class VariableDeclaration;
class AssignmentStatement;
class Conditional;
class Loop;
class MemoryManagement;
class ReturnStatement;
class FunctionCall;
class Literal;
class Statement;
class NumericLiteral;
class BooleanLiteral;
class CharLiteral;
class StringLiteral;
class BinaryExpression;
class UnaryExpression;
class PrimaryExpression;
class ArrayAccessExpression;
class TernaryExpression;
class CastExpression;
class MemberAccessExpression;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    // Declaraciones de metodos de visita para cada nodo
    virtual void visit(ProgramNode& node) = 0;
    virtual void visit(FunctionDefinition& node) = 0;
    virtual void visit(ParameterList& node) = 0;
    virtual void visit(Type& node) = 0;
    virtual void visit(Identifier& node) = 0;
    virtual void visit(Statement& node) = 0;
    virtual void visit(VariableDeclaration& node) = 0;
    virtual void visit(AssignmentStatement& node) = 0;
    virtual void visit(Conditional& node) = 0;
    virtual void visit(Loop& node) = 0;
    virtual void visit(MemoryManagement& node) = 0;
    virtual void visit(ReturnStatement& node) = 0;
    virtual void visit(FunctionCall& node) = 0;
    virtual void visit(Literal& node) = 0;
    virtual void visit(NumericLiteral& node) = 0;
    virtual void visit(BooleanLiteral& node) = 0;
    virtual void visit(CharLiteral& node) = 0;
    virtual void visit(StringLiteral& node) = 0;
    virtual void visit(BinaryExpression& node) = 0;
    virtual void visit(UnaryExpression& node) = 0;
    virtual void visit(PrimaryExpression& node) = 0;
    virtual void visit(ArrayAccessExpression& node) = 0;
    virtual void visit(TernaryExpression& node) = 0;
    virtual void visit(CastExpression& node) = 0;
    virtual void visit(MemberAccessExpression& node) = 0;   
};

}

#endif