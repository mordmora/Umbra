/*
 * ASTVisitor Header for Umbra Language Compiler
 * Copyright (C) 2023 Free Software Foundation, Inc.
 * 
 * This file is part of the Umbra Compiler Project.
 * Contributed by Ginozza (jsimancas@unimagdalena.edu.co).
 * 
 * The Umbra Compiler is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of the
 * License, or (at your option) any later version.
 * 
 * The Umbra Compiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Umbra Compiler; see the file COPYING.LIB. If
 * not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UMBRA_AST_VISITOR_H
#define UMBRA_AST_VISITOR_H

namespace umbra {

/**
 * @brief Abstract base class for visiting nodes in an Abstract Syntax Tree (AST).
 * 
 * The `ASTVisitor` class defines a set of virtual methods for visiting different
 * types of nodes in an AST. Subclasses of `ASTVisitor` implement these methods to
 * perform specific actions, such as generating code, performing semantic analysis,
 * or printing the AST.
 */
class ASTVisitor {
public:
    /**
     * @brief Virtual destructor for the ASTVisitor class.
     * 
     * Ensures proper cleanup of resources when a subclass instance is destroyed.
     */
    virtual ~ASTVisitor() = default;

    /**
     * @brief Visits a ProgramNode to process the entire program structure.
     * @param node The ProgramNode representing the program.
     */
    virtual void visit(ProgramNode& node) = 0;

    /**
     * @brief Visits a FunctionDefinition node to process a function definition.
     * @param node The FunctionDefinition node containing the function's metadata.
     */
    virtual void visit(FunctionDefinition& node) = 0;

    /**
     * @brief Visits a ParameterList node to process a function's parameters.
     * @param node The ParameterList node containing the function's parameters.
     */
    virtual void visit(ParameterList& node) = 0;

    /**
     * @brief Visits a Type node to process a type declaration.
     * @param node The Type node representing a variable or function type.
     */
    virtual void visit(Type& node) = 0;

    /**
     * @brief Visits an Identifier node to process a variable or function name.
     * @param node The Identifier node representing a name.
     */
    virtual void visit(Identifier& node) = 0;

    /**
     * @brief Visits a Statement node to process a single statement in the program.
     * @param node The Statement node representing a generic statement.
     */
    virtual void visit(Statement& node) = 0;

    /**
     * @brief Visits a VariableDeclaration node to process a variable declaration.
     * @param node The VariableDeclaration node containing the variable's metadata.
     */
    virtual void visit(VariableDeclaration& node) = 0;

    /**
     * @brief Visits an AssignmentStatement node to process an assignment operation.
     * @param node The AssignmentStatement node containing the target and value.
     */
    virtual void visit(AssignmentStatement& node) = 0;

    /**
     * @brief Visits a Conditional node to process a conditional statement (if-else).
     * @param node The Conditional node containing the condition and branches.
     */
    virtual void visit(Conditional& node) = 0;

    /**
     * @brief Visits a Loop node to process a loop construct.
     * @param node The Loop node containing the condition and body.
     */
    virtual void visit(Loop& node) = 0;

    /**
     * @brief Visits a MemoryManagement node to process memory allocation/deallocation.
     * @param node The MemoryManagement node specifying the action (allocate/deallocate).
     */
    virtual void visit(MemoryManagement& node) = 0;

    /**
     * @brief Visits a ReturnStatement node to process a return statement.
     * @param node The ReturnStatement node containing the return value (can be nullptr).
     */
    virtual void visit(ReturnStatement& node) = 0;

    /**
     * @brief Visits a FunctionCall node to process a function call.
     * @param node The FunctionCall node containing the callee and arguments.
     */
    virtual void visit(FunctionCall& node) = 0;

    /**
     * @brief Visits a Literal node to process a generic literal.
     * Specific literals are handled by their respective visitor methods.
     * @param node The Literal node.
     */
    virtual void visit(Literal& node) = 0;

    /**
     * @brief Visits a NumericLiteral node to process a numeric literal.
     * @param node The NumericLiteral node containing the numeric value.
     */
    virtual void visit(NumericLiteral& node) = 0;

    /**
     * @brief Visits a BooleanLiteral node to process a boolean literal.
     * @param node The BooleanLiteral node containing the boolean value.
     */
    virtual void visit(BooleanLiteral& node) = 0;

    /**
     * @brief Visits a CharLiteral node to process a character literal.
     * @param node The CharLiteral node containing the character value.
     */
    virtual void visit(CharLiteral& node) = 0;

    /**
     * @brief Visits a StringLiteral node to process a string literal.
     * @param node The StringLiteral node containing the string value.
     */
    virtual void visit(StringLiteral& node) = 0;

    /**
     * @brief Visits a BinaryExpression node to process a binary expression.
     * @param node The BinaryExpression node containing the operator and operands.
     */
    virtual void visit(BinaryExpression& node) = 0;

    /**
     * @brief Visits a UnaryExpression node to process a unary expression.
     * @param node The UnaryExpression node containing the operator and operand.
     */
    virtual void visit(UnaryExpression& node) = 0;

    /**
     * @brief Visits a PrimaryExpression node to process a primary expression.
     * @param node The PrimaryExpression node containing the expression type.
     */
    virtual void visit(PrimaryExpression& node) = 0;

    /**
     * @brief Visits an ArrayAccessExpression node to process an array access.
     * @param node The ArrayAccessExpression node containing the array and index.
     */
    virtual void visit(ArrayAccessExpression& node) = 0;

    /**
     * @brief Visits a TernaryExpression node to process a ternary expression.
     * @param node The TernaryExpression node containing the condition and branches.
     */
    virtual void visit(TernaryExpression& node) = 0;

    /**
     * @brief Visits a CastExpression node to process a type cast.
     * @param node The CastExpression node containing the target type and expression.
     */
    virtual void visit(CastExpression& node) = 0;

    /**
     * @brief Visits a MemberAccessExpression node to process member access.
     * @param node The MemberAccessExpression node containing the object and member.
     */
    virtual void visit(MemberAccessExpression& node) = 0;
};

} // namespace umbra

#endif // UMBRA_AST_VISITOR_H