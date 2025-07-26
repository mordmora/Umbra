#pragma once


/*
    Implementacion sencilla del patron visitante generico para faciliar el recorrido por el AST.

    Ptr

*/

#include <memory>
#include <utility>
#include <vector>
#include <type_traits>
#include "umbra/ast/ASTNode.h"
// Incluir definiciones de nodos para que el visitor genérico los conozca
#include "umbra/ast/Nodes.h"

namespace umbra{

template<typename T>
struct PtrTraits {
    using type = T*;
};

template<typename T>
struct PtrTraits<std::unique_ptr<T>> {
    using type = T*;
};

template<typename T>
struct PtrTraits<std::shared_ptr<T>> {
    using type = T*;
};

template<template <typename> class Ptr,
    typename ImplClass,
    typename RetTy = void,
    class... ParamTys>
class BaseV {
    public:
    #define PTR(CLASS) typename PtrTraits<Ptr<CLASS>>::type
    #define DISPATCH(NAME)  \
        return static_cast<ImplClass*>(this)->visit##NAME(static_cast<PTR(NAME)>(A))

    // Visitor genérico: llama a visitXxx o a el handler por defecto
    RetTy visit(PTR(ASTNode) A){

        switch(A->getKind()){

            case NodeKind::PROGRAM:
                DISPATCH(ProgramNode);
            case NodeKind::FUNCTION_DEFINITION:
                DISPATCH(FunctionDefinition);
            case NodeKind::PARAMETER_LIST:
                DISPATCH(ParameterList);
            case NodeKind::TYPE:
                DISPATCH(Type);
            case NodeKind::VARIABLE_DECLARATION:
                DISPATCH(VariableDeclaration);
            case NodeKind::ASSIGNMENT_STATEMENT:
                DISPATCH(AssignmentStatement);
            case NodeKind::IF_STATEMENT:
                DISPATCH(IfStatement);
            case NodeKind::REPEAT_TIMES_STATEMENT:
                DISPATCH(RepeatTimesStatement);
            case NodeKind::REPEAT_IF_STATEMENT:
                DISPATCH(RepeatIfStatement);
            case NodeKind::EXPRESSION_STATEMENT:
                DISPATCH(ExpressionStatement);
            case NodeKind::MEMORY_MANAGEMENT:
                DISPATCH(MemoryManagement);
            case NodeKind::IDENTIFIER:
                DISPATCH(Identifier);
            case NodeKind::BINARY_EXPRESSION:
                DISPATCH(BinaryExpression);
            case NodeKind::UNARY_EXPRESSION:
                DISPATCH(UnaryExpression);
            case NodeKind::PRIMARY_EXPRESSION:
                DISPATCH(PrimaryExpression);
            case NodeKind::FUNCTION_CALL:
                DISPATCH(FunctionCall);
            case NodeKind::RETURN_EXPRESSION:
                DISPATCH(ReturnExpression);
            case NodeKind::ARRAY_ACCESS_EXPRESSION:
                DISPATCH(ArrayAccessExpression);
            case NodeKind::TERNARY_EXPRESSION:
                DISPATCH(TernaryExpression);
            case NodeKind::CAST_EXPRESSION:
                DISPATCH(CastExpression);
            case NodeKind::MEMBER_ACCESS_EXPRESSION:
                DISPATCH(MemberAccessExpression);
            case NodeKind::PARENTHESIZED:
                // Handle parenthesized expressions as PrimaryExpression
                DISPATCH(PrimaryExpression);
            case NodeKind::LITERAL:
                DISPATCH(Literal);
            case NodeKind::NUMERIC_LITERAL:
                DISPATCH(NumericLiteral);
            case NodeKind::BOOLEAN_LITERAL:
                DISPATCH(BooleanLiteral);
            case NodeKind::CHAR_LITERAL:
                DISPATCH(CharLiteral);
            case NodeKind::STRING_LITERAL:
                DISPATCH(StringLiteral);
        }
    }
    // Default no-op implementations para nodos no sobreescritos
    // Helper that returns RetTy() for non-void or does nothing for void
    template<typename U = RetTy>
    std::enable_if_t<std::is_void_v<U>, void> defaultVisit() {}

    template<typename U = RetTy>
    std::enable_if_t<!std::is_void_v<U>, U> defaultVisit() { return U(); }

    // Stubs forwarding to defaultVisit()
    // Top-level nodes
    RetTy visitProgramNode(ProgramNode*)              { return defaultVisit(); }
    RetTy visitFunctionDefinition(FunctionDefinition*) { return defaultVisit(); }
    RetTy visitParameterList(ParameterList*)               { return defaultVisit(); }
    RetTy visitType(Type*)                                 { return defaultVisit(); }
    RetTy visitVariableDeclaration(VariableDeclaration*)   { return defaultVisit(); }
    RetTy visitAssignmentStatement(AssignmentStatement*)   { return defaultVisit(); }
    RetTy visitIfStatement(IfStatement*)                   { return defaultVisit(); }
    RetTy visitRepeatTimesStatement(RepeatTimesStatement*) { return defaultVisit(); }
    RetTy visitRepeatIfStatement(RepeatIfStatement*)       { return defaultVisit(); }
    RetTy visitExpressionStatement(ExpressionStatement*)   { return defaultVisit(); }
    RetTy visitMemoryManagement(MemoryManagement*)         { return defaultVisit(); }
    RetTy visitIdentifier(Identifier*)                     { return defaultVisit(); }
    RetTy visitBinaryExpression(BinaryExpression*)         { return defaultVisit(); }
    RetTy visitUnaryExpression(UnaryExpression*)           { return defaultVisit(); }
    RetTy visitPrimaryExpression(PrimaryExpression*)       { return defaultVisit(); }
    RetTy visitFunctionCall(FunctionCall*)                 { return defaultVisit(); }
    RetTy visitReturnExpression(ReturnExpression*)         { return defaultVisit(); }
    RetTy visitArrayAccessExpression(ArrayAccessExpression*) { return defaultVisit(); }
    RetTy visitTernaryExpression(TernaryExpression*)       { return defaultVisit(); }
    RetTy visitCastExpression(CastExpression*)             { return defaultVisit(); }
    RetTy visitMemberAccessExpression(MemberAccessExpression*) { return defaultVisit(); }
    RetTy visitLiteral(Literal*)                           { return defaultVisit(); }
    RetTy visitNumericLiteral(NumericLiteral*)             { return defaultVisit(); }
    RetTy visitBooleanLiteral(BooleanLiteral*)             { return defaultVisit(); }
    RetTy visitCharLiteral(CharLiteral*)                   { return defaultVisit(); }
    RetTy visitStringLiteral(StringLiteral*)               { return defaultVisit(); }
};

}
