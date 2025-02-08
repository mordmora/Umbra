/*
 * LLVMVisitor Implementation for Umbra Language Compiler
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

/*
 * This file contains the implementation of the LLVMVisitor class, which
 * generates LLVM Intermediate Representation (IR) code from an Abstract
 * Syntax Tree (AST). It traverses the AST nodes and emits corresponding
 * LLVM IR instructions using the LLVM API.
 * 
 * The LLVMVisitor supports various constructs such as function definitions,
 * variable declarations, assignments, conditionals, loops, memory management,
 * and expressions. It uses LLVM's IRBuilder to construct the IR dynamically.
 * 
 * Key Features:
 * - Generates LLVM IR for a subset of the Umbra language.
 * - Supports basic control flow constructs like if-else and loops.
 * - Handles memory allocation and deallocation using malloc/free.
 * - Implements array indexing and pointer arithmetic.
 * - Provides support for binary and unary expressions, literals, and function calls.
 * 
 * Contents:
 * 1. Program and Function Definitions
 *    - visit(ProgramNode& node)
 *    - visit(FunctionDefinition& node)
 * 
 * 2. Variable Declarations and Assignments
 *    - visit(VariableDeclaration& node)
 *    - visit(AssignmentStatement& node)
 * 
 * 3. Control Flow Constructs
 *    - visit(Conditional& node)
 *    - visit(Loop& node)
 * 
 * 4. Memory Management
 *    - visit(MemoryManagement& node)
 * 
 * 5. Expressions
 *    - visit(BinaryExpression& node)
 *    - visit(UnaryExpression& node)
 *    - visit(ArrayAccessExpression& node)
 *    - visit(TernaryExpression& node)
 *    - visit(CastExpression& node)
 * 
 * 6. Literals and Function Calls
 *    - visit(Literal& node)
 *    - visit(FunctionCall& node)
 * 
 * 7. Miscellaneous
 *    - visit(ReturnStatement& node)
 *    - visit(MemberAccessExpression& node)
 * 
 * Vital Statistics:
 * - Supported pointer representation:       8 bytes (LLVM default)
 * - Supported size_t representation:        8 bytes (LLVM default)
 * - Alignment:                              Determined by LLVM types
 * - Thread-safety:                          Not inherently thread-safe
 * 
 * Quickstart:
 * To compile this implementation, ensure you have LLVM installed and properly
 * configured. Use CMake or a Makefile to build the project. Example:
 *   mkdir build && cd build
 *   cmake ..
 *   make
 * 
 * Why use this LLVMVisitor?
 * This implementation provides a foundation for generating LLVM IR from an AST,
 * enabling optimization and machine code generation through LLVM's powerful tools.
 * It is designed to be extensible, allowing additional language features to be
 * supported with minimal changes.
 * 
 * For more information on LLVM, see:
 * https://llvm.org/docs/
 */
/**
 * @brief LLVMVisitor.cpp Pending Tasks
 * TODO: Complete the implementation of all visit methods
 * TODO: Implement error handling 
 * TODO: Check LLVM code and add optimizations
 * TODO: Implement runtime library for security checks
 * TODO: Integrate complete type system
 */
#include "LLVMVisitor.h"
#include "Nodes.h" 
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <memory> // Para std::unique_ptr
#include <map>
#include <string>

using namespace llvm;

namespace umbra {

/**
 * @brief Constructs an LLVMVisitor object.
 * Initializes the LLVM context, module, and IRBuilder.
 */
LLVMVisitor::LLVMVisitor() : builder(context) {
    module = std::make_unique<llvm::Module>("UmbraModule", context);
}

/**
 * @brief Visits a ProgramNode to generate LLVM IR for the entire program.
 * @param node The ProgramNode representing the program structure.
 */
void LLVMVisitor::visit(ProgramNode& node) {
    for (auto& func : node.functions) {
        func->accept(*this);
    }
}

/**
 * @brief Generates LLVM IR for a function definition.
 * @param node The FunctionDefinition node containing the function's metadata.
 */
void LLVMVisitor::visit(FunctionDefinition& node) {
    llvm::Type* llvmReturnType = typeToLLVM(*node.returnType);
    std::vector<llvm::Type*> paramTypes;
    for (auto& param : node.parameters->parameters) {
        paramTypes.push_back(typeToLLVM(*param.first));
    }
    FunctionType* funcType = FunctionType::get(llvmReturnType, paramTypes, false);
    Function* func = Function::Create(
        funcType,
        Function::ExternalLinkage,
        node.name->name,
        module.get()
    );
    BasicBlock* entry = BasicBlock::Create(context, "entry", func);
    builder.SetInsertPoint(entry);

    // Assign names to arguments based on the parameter list.
    unsigned idx = 0;
    for (auto& arg : func->args()) {
        arg.setName(node.parameters->parameters[idx++].second->name);
    }

    // Generate the function body.
    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }

    // If the function is void and no terminator was generated, add a RetVoid.
    if (llvmReturnType->isVoidTy() && !builder.GetInsertBlock()->getTerminator()) {
        builder.CreateRetVoid();
    }
}

/**
 * @brief Generates LLVM IR for a variable declaration.
 * @param node The VariableDeclaration node containing the variable's metadata.
 */
void LLVMVisitor::visit(VariableDeclaration& node) {
    llvm::Type* llvmType = typeToLLVM(*node.type);
    AllocaInst* alloca = builder.CreateAlloca(llvmType, nullptr, node.name->name);
    symbolTable[node.name->name] = alloca;
    if (node.initializer) {
        node.initializer->accept(*this);
        builder.CreateStore(currentValue, alloca);
    }
}

/**
 * @brief Generates LLVM IR for an assignment statement.
 * @param node The AssignmentStatement node containing the target and value.
 */
void LLVMVisitor::visit(AssignmentStatement& node) {
    node.value->accept(*this);
    llvm::Value* rhs = currentValue;
    if (symbolTable.find(node.target->name) != symbolTable.end()) {
        llvm::Value* ptr = symbolTable[node.target->name];
        if (node.index) {
            node.index->accept(*this);
            llvm::Value* indexVal = currentValue;
            std::vector<llvm::Value*> indices;
            indices.push_back(builder.getInt32(0)); // Zero index for the array's first dimension.
            indices.push_back(indexVal);
            llvm::Type* pointeeType = ptr->getType()->getPointerElementType();
            llvm::Value* elemPtr = builder.CreateGEP(pointeeType, ptr, indices, "elemPtr");
            builder.CreateStore(rhs, elemPtr);
        } else {
            builder.CreateStore(rhs, ptr);
        }
    } else {
        currentValue = nullptr;
    }
}

/**
 * @brief Generates LLVM IR for a conditional statement (if-else).
 * @param node The Conditional node containing the condition and branches.
 */
void LLVMVisitor::visit(Conditional& node) {
    Function* parentFunc = builder.GetInsertBlock()->getParent();
    BasicBlock* mergeBB = BasicBlock::Create(context, "ifcont", parentFunc);

    if (!node.branches.empty()) {
        auto& firstBranch = node.branches[0];
        firstBranch.condition->accept(*this);
        llvm::Value* condVal = currentValue;
        BasicBlock* thenBB = BasicBlock::Create(context, "then", parentFunc);
        BasicBlock* elseBB = node.elseBranch.empty() ? mergeBB : BasicBlock::Create(context, "else", parentFunc);
        builder.CreateCondBr(condVal, thenBB, elseBB);

        // "Then" block.
        builder.SetInsertPoint(thenBB);
        for (auto& stmt : firstBranch.body) {
            stmt->accept(*this);
        }
        if (!builder.GetInsertBlock()->getTerminator())
            builder.CreateBr(mergeBB);

        // "Else" block, if it exists.
        if (!node.elseBranch.empty()) {
            builder.SetInsertPoint(elseBB);
            for (auto& stmt : node.elseBranch) {
                stmt->accept(*this);
            }
            if (!builder.GetInsertBlock()->getTerminator())
                builder.CreateBr(mergeBB);
        }
        builder.SetInsertPoint(mergeBB);
    }
}

/**
 * @brief Generates LLVM IR for a loop construct.
 * @param node The Loop node containing the condition and body.
 */
void LLVMVisitor::visit(Loop& node) {
    Function* parentFunc = builder.GetInsertBlock()->getParent();
    BasicBlock* condBB = BasicBlock::Create(context, "loopcond", parentFunc);
    BasicBlock* loopBB = BasicBlock::Create(context, "loop", parentFunc);
    BasicBlock* afterBB = BasicBlock::Create(context, "afterloop", parentFunc);
    builder.CreateBr(condBB);

    // Condition block.
    builder.SetInsertPoint(condBB);
    node.condition->accept(*this);
    llvm::Value* condVal = currentValue;
    builder.CreateCondBr(condVal, loopBB, afterBB);

    // Loop body block.
    builder.SetInsertPoint(loopBB);
    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(condBB);

    // After-loop block.
    builder.SetInsertPoint(afterBB);
}

/**
 * @brief Generates LLVM IR for memory management operations (malloc/free).
 * @param node The MemoryManagement node specifying the action (allocate/deallocate).
 */
void LLVMVisitor::visit(MemoryManagement& node) {
    if (node.action == MemoryManagement::ALLOCATE) {
        node.size->accept(*this);
        llvm::Value* sizeVal = currentValue;
        Function* mallocFunc = module->getFunction("malloc");
        if (!mallocFunc) {
            llvm::Type* i64Type = llvm::Type::getInt64Ty(context);
            llvm::FunctionType* mallocType =
                llvm::FunctionType::get(llvm::Type::getInt8PtrTy(context), {i64Type}, false);
            mallocFunc = Function::Create(mallocType, Function::ExternalLinkage, "malloc", module.get());
        }
        currentValue = builder.CreateCall(mallocFunc, {sizeVal}, "mallocCall");
    } else if (node.action == MemoryManagement::DEALLOCATE) {
        node.target->accept(*this);
        llvm::Value* ptr = currentValue;
        Function* freeFunc = module->getFunction("free");
        if (!freeFunc) {
            llvm::FunctionType* freeType =
                llvm::FunctionType::get(llvm::Type::getVoidTy(context), {llvm::Type::getInt8PtrTy(context)}, false);
            freeFunc = Function::Create(freeType, Function::ExternalLinkage, "free", module.get());
        }
        builder.CreateCall(freeFunc, {ptr});
    }
}

/**
 * @brief Generates LLVM IR for a return statement.
 * @param node The ReturnStatement node containing the return value (can be nullptr).
 */
void LLVMVisitor::visit(ReturnStatement& node) {
    if (node.returnValue) {
        node.returnValue->accept(*this);
        builder.CreateRet(currentValue);
    } else {
        builder.CreateRetVoid();
    }
}

/**
 * @brief Generates LLVM IR for a function call.
 * @param node The FunctionCall node containing the callee and arguments.
 */
void LLVMVisitor::visit(FunctionCall& node) {
    Function* callee = module->getFunction(node.functionName->name);
    if (!callee) {
        currentValue = nullptr;
        return;
    }
    std::vector<llvm::Value*> args;
    for (auto& arg : node.arguments) {
        arg->accept(*this);
        args.push_back(currentValue);
    }
    currentValue = builder.CreateCall(callee, args, "calltmp");
}

/**
 * @todo
 * @brief Base implementation for literal nodes.
 * Specific literals are handled by their respective visitor methods.
 * @param node The Literal node.
 */
void LLVMVisitor::visit(Literal& node) {
    // Base implementation; specific literals have their own visitor.
}

/**
 * @brief Generates LLVM IR for a boolean literal.
 * @param node The BooleanLiteral node containing the boolean value.
 */
void LLVMVisitor::visit(BooleanLiteral& node) {
    currentValue = ConstantInt::get(llvm::Type::getInt1Ty(context), node.value);
}

/**
 * @brief Generates LLVM IR for a character literal.
 * @param node The CharLiteral node containing the character value.
 */
void LLVMVisitor::visit(CharLiteral& node) {
    currentValue = ConstantInt::get(llvm::Type::getInt8Ty(context), node.value);
}

/**
 * @brief Generates LLVM IR for a string literal.
 * @param node The StringLiteral node containing the string value.
 */
void LLVMVisitor::visit(StringLiteral& node) {
    currentValue = builder.CreateGlobalStringPtr(node.value, "str");
}

/**
 * @brief Generates LLVM IR for a binary expression.
 * @param node The BinaryExpression node containing the operator and operands.
 */
void LLVMVisitor::visit(BinaryExpression& node) {
    node.left->accept(*this);
    llvm::Value* leftVal = currentValue;
    node.right->accept(*this);
    llvm::Value* rightVal = currentValue;

    if (node.op == "+") {
        currentValue = builder.CreateFAdd(leftVal, rightVal, "addtmp");
    } else if (node.op == "-") {
        currentValue = builder.CreateFSub(leftVal, rightVal, "subtmp");
    } else if (node.op == "*") {
        currentValue = builder.CreateFMul(leftVal, rightVal, "multmp");
    } else if (node.op == "/") {
        currentValue = builder.CreateFDiv(leftVal, rightVal, "divtmp");
    } else if (node.op == "==") {
        currentValue = builder.CreateFCmpOEQ(leftVal, rightVal, "eqtmp");
    } else if (node.op == "!=") {
        currentValue = builder.CreateFCmpONE(leftVal, rightVal, "netmp");
    } else if (node.op == "<") {
        currentValue = builder.CreateFCmpOLT(leftVal, rightVal, "lttmp");
    } else if (node.op == "<=") {
        currentValue = builder.CreateFCmpOLE(leftVal, rightVal, "letmp");
    } else if (node.op == ">") {
        currentValue = builder.CreateFCmpOGT(leftVal, rightVal, "gttmp");
    } else if (node.op == ">=") {
        currentValue = builder.CreateFCmpOGE(leftVal, rightVal, "getmp");
    } else {
        currentValue = nullptr; // Unsupported operator.
    }
}

/**
 * @brief Generates LLVM IR for a unary expression.
 * @param node The UnaryExpression node containing the operator and operand.
 */
void LLVMVisitor::visit(UnaryExpression& node) {
    node.operand->accept(*this);
    llvm::Value* operandVal = currentValue;
    if (node.op == "-") {
        currentValue = builder.CreateFNeg(operandVal, "negtmp");
    } else {
        currentValue = nullptr; // Unsupported operator.
    }
}

/**
 * @brief Generates LLVM IR for a primary expression.
 * @param node The PrimaryExpression node containing the expression type.
 */
void LLVMVisitor::visit(PrimaryExpression& node) {
    switch (node.exprType) {
        case PrimaryExpression::IDENTIFIER:
            node.identifier->accept(*this);
            break;
        case PrimaryExpression::LITERAL:
            node.literal->accept(*this);
            break;
        case PrimaryExpression::PARENTHESIZED:
            node.parenthesized->accept(*this);
            break;
        case PrimaryExpression::EXPRESSION_CALL:
            node.functionCall->accept(*this);
            break;
        case PrimaryExpression::ARRAY_ACCESS:
            node.arrayAccess->accept(*this);
            break;
        case PrimaryExpression::MEMBER_ACCESS:
            node.memberAccess->accept(*this);
            break;
        case PrimaryExpression::CAST_EXPRESSION:
            node.castExpression->accept(*this);
            break;
        case PrimaryExpression::TERNARY_EXPRESSION:
            node.ternaryExpression->accept(*this);
            break;
        default:
            currentValue = nullptr;
            break;
    }
}

/**
 * @brief Generates LLVM IR for an array access expression.
 * @param node The ArrayAccessExpression node containing the array and index.
 */
void LLVMVisitor::visit(ArrayAccessExpression& node) {
    node.array->accept(*this);
    llvm::Value* arrayPtr = currentValue;
    node.index->accept(*this);
    llvm::Value* indexVal = currentValue;

    std::vector<llvm::Value*> indices;
    indices.push_back(indexVal);
    llvm::Type* elemType = arrayPtr->getType()->getPointerElementType();
    llvm::Value* elemPtr = builder.CreateGEP(elemType, arrayPtr, indices, "arrayElem");
    currentValue = builder.CreateLoad(elemType, elemPtr, "loadArrayElem");
}

/**
 * @brief Generates LLVM IR for a ternary expression.
 * @param node The TernaryExpression node containing the condition and branches.
 */
void LLVMVisitor::visit(TernaryExpression& node) {
    Function* parentFunc = builder.GetInsertBlock()->getParent();
    node.condition->accept(*this);
    llvm::Value* condVal = currentValue;

    BasicBlock* trueBB = BasicBlock::Create(context, "ternaryTrue", parentFunc);
    BasicBlock* falseBB = BasicBlock::Create(context, "ternaryFalse", parentFunc);
    BasicBlock* mergeBB = BasicBlock::Create(context, "ternaryMerge", parentFunc);

    builder.CreateCondBr(condVal, trueBB, falseBB);

    builder.SetInsertPoint(trueBB);
    node.trueExpr->accept(*this);
    llvm::Value* trueVal = currentValue;
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(mergeBB);

    builder.SetInsertPoint(falseBB);
    node.falseExpr->accept(*this);
    llvm::Value* falseVal = currentValue;
    if (!builder.GetInsertBlock()->getTerminator())
        builder.CreateBr(mergeBB);

    builder.SetInsertPoint(mergeBB);
    PHINode* phi = builder.CreatePHI(trueVal->getType(), 2, "ternaryResult");
    phi->addIncoming(trueVal, trueBB);
    phi->addIncoming(falseVal, falseBB);
    currentValue = phi;
}

/**
 * @brief Generates LLVM IR for a cast expression.
 * @param node The CastExpression node containing the target type and expression.
 */
void LLVMVisitor::visit(CastExpression& node) {
    node.expression->accept(*this);
    llvm::Value* exprVal = currentValue;
    llvm::Type* targetLLVMType = typeToLLVM(*node.targetType);
    currentValue = builder.CreateBitCast(exprVal, targetLLVMType, "casttmp");
}

/**
 * @brief Generates LLVM IR for a member access expression.
 * @param node The MemberAccessExpression node containing the object and member.
 */
void LLVMVisitor::visit(MemberAccessExpression& node) {
    node.object->accept(*this);
    llvm::Value* objectVal = currentValue;
    // For a complete implementation, the member index should be retrieved and a GEP generated.
    // Here, the object is returned without modification.
    currentValue = objectVal;
}

} // namespace umbra