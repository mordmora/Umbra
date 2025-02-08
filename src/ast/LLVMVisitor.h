/*
 * LLVMVisitor Header for Umbra Language Compiler
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
#ifndef UMBRA_LLVM_VISITOR_H
#define UMBRA_LLVM_VISITOR_H

#include "ASTVisitor.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <memory> // For std::unique_ptr
#include <map>
#include <string>

namespace umbra {

/**
 * @brief Implements LLVM IR code generation from an AST.
 * 
 * The `LLVMVisitor` class inherits from `ASTVisitor` and provides methods to traverse
 * an Abstract Syntax Tree (AST) and generate corresponding LLVM Intermediate
 * Representation (IR) code. It uses LLVM's `IRBuilder` to dynamically construct IR
 * instructions for various language constructs such as functions, variables, control
 * flow, expressions, and memory management.
 */
class LLVMVisitor : public ASTVisitor {
public:
    /**
     * @brief Constructs an LLVMVisitor object.
     * 
     * Initializes the LLVM context, module, and IRBuilder. This constructor sets up
     * the necessary components for generating LLVM IR code.
     */
    LLVMVisitor();

    /**
     * @brief Destructor for LLVMVisitor.
     * 
     * Explicitly declared to match the exception specification of the base class
     * `ASTVisitor`.
     */
    virtual ~LLVMVisitor() noexcept override = default;

    /**
     * @brief Visits a ProgramNode to generate LLVM IR for the entire program.
     * @param node The ProgramNode representing the program structure.
     */
    void visit(ProgramNode& node) override;

    /**
     * @brief Generates LLVM IR for a function definition.
     * @param node The FunctionDefinition node containing the function's metadata.
     */
    void visit(FunctionDefinition& node) override;

    /**
     * @brief Generates LLVM IR for a parameter list.
     * @param node The ParameterList node containing the function's parameters.
     */
    void visit(ParameterList& node) override;

    /**
     * @brief Generates LLVM IR for a type declaration.
     * @param node The Type node representing a variable or function type.
     */
    void visit(Type& node) override;

    /**
     * @brief Generates LLVM IR for an identifier.
     * @param node The Identifier node representing a variable or function name.
     */
    void visit(Identifier& node) override;

    /**
     * @brief Generates LLVM IR for a generic statement.
     * @param node The Statement node representing a single statement in the program.
     */
    void visit(Statement& node) override;

    /**
     * @brief Generates LLVM IR for a variable declaration.
     * @param node The VariableDeclaration node containing the variable's metadata.
     */
    void visit(VariableDeclaration& node) override;

    /**
     * @brief Generates LLVM IR for an assignment statement.
     * @param node The AssignmentStatement node containing the target and value.
     */
    void visit(AssignmentStatement& node) override;

    /**
     * @brief Generates LLVM IR for a conditional statement (if-else).
     * @param node The Conditional node containing the condition and branches.
     */
    void visit(Conditional& node) override;

    /**
     * @brief Generates LLVM IR for a loop construct.
     * @param node The Loop node containing the condition and body.
     */
    void visit(Loop& node) override;

    /**
     * @brief Generates LLVM IR for memory management operations (malloc/free).
     * @param node The MemoryManagement node specifying the action (allocate/deallocate).
     */
    void visit(MemoryManagement& node) override;

    /**
     * @brief Generates LLVM IR for a return statement.
     * @param node The ReturnStatement node containing the return value (can be nullptr).
     */
    void visit(ReturnStatement& node) override;

    /**
     * @brief Generates LLVM IR for a function call.
     * @param node The FunctionCall node containing the callee and arguments.
     */
    void visit(FunctionCall& node) override;

    /**
     * @brief Base implementation for literal nodes.
     * Specific literals are handled by their respective visitor methods.
     * @param node The Literal node.
     */
    void visit(Literal& node) override;

    /**
     * @brief Generates LLVM IR for a numeric literal.
     * @param node The NumericLiteral node containing the numeric value.
     */
    void visit(NumericLiteral& node) override;

    /**
     * @brief Generates LLVM IR for a boolean literal.
     * @param node The BooleanLiteral node containing the boolean value.
     */
    void visit(BooleanLiteral& node) override;

    /**
     * @brief Generates LLVM IR for a character literal.
     * @param node The CharLiteral node containing the character value.
     */
    void visit(CharLiteral& node) override;

    /**
     * @brief Generates LLVM IR for a string literal.
     * @param node The StringLiteral node containing the string value.
     */
    void visit(StringLiteral& node) override;

    /**
     * @brief Generates LLVM IR for a binary expression.
     * @param node The BinaryExpression node containing the operator and operands.
     */
    void visit(BinaryExpression& node) override;

    /**
     * @brief Generates LLVM IR for a unary expression.
     * @param node The UnaryExpression node containing the operator and operand.
     */
    void visit(UnaryExpression& node) override;

    /**
     * @brief Generates LLVM IR for a primary expression.
     * @param node The PrimaryExpression node containing the expression type.
     */
    void visit(PrimaryExpression& node) override;

    /**
     * @brief Generates LLVM IR for an array access expression.
     * @param node The ArrayAccessExpression node containing the array and index.
     */
    void visit(ArrayAccessExpression& node) override;

    /**
     * @brief Generates LLVM IR for a ternary expression.
     * @param node The TernaryExpression node containing the condition and branches.
     */
    void visit(TernaryExpression& node) override;

    /**
     * @brief Generates LLVM IR for a cast expression.
     * @param node The CastExpression node containing the target type and expression.
     */
    void visit(CastExpression& node) override;

    /**
     * @brief Generates LLVM IR for a member access expression.
     * @param node The MemberAccessExpression node containing the object and member.
     */
    void visit(MemberAccessExpression& node) override;

    /**
     * @brief Retrieves the generated LLVM module.
     * 
     * This method returns the LLVM module containing the generated IR code. After
     * calling this method, the ownership of the module is transferred to the caller.
     * 
     * @return std::unique_ptr<llvm::Module> The generated LLVM module.
     */
    std::unique_ptr<llvm::Module> getModule() { return std::move(module); }

private:
    /**
     * @brief The LLVM module containing the generated IR code.
     */
    std::unique_ptr<llvm::Module> module;

    /**
     * @brief The LLVM context used to create types and values.
     */
    llvm::LLVMContext context;

    /**
     * @brief The IRBuilder for dynamically constructing LLVM IR instructions.
     */
    llvm::IRBuilder<> builder;

    /**
     * @brief The current value generated during the traversal of a node.
     */
    llvm::Value* currentValue = nullptr;

    /**
     * @brief Symbol table to store variables and their memory addresses.
     */
    std::map<std::string, llvm::Value*> symbolTable;

    /**
     * @brief Converts Umbra language types to LLVM types.
     * 
     * This helper function maps types from the Umbra language to their equivalent
     * LLVM types.
     * 
     * @param type The Umbra language type.
     * @return llvm::Type* The equivalent LLVM type.
     */
    llvm::Type* typeToLLVM(Type& type);

    /**
     * @brief Creates runtime checks required for the program.
     * 
     * This method generates any necessary runtime checks, such as bounds checking
     * for arrays or null pointer checks.
     */
    void createRuntimeChecks();
};

} // namespace umbra

#endif // UMBRA_LLVM_VISITOR_H