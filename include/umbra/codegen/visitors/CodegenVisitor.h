#pragma once
#include "umbra/ast/Types.h"
#include "umbra/codegen/context/CodegenContext.h"
#include "umbra/ast/Nodes.h"
#include "umbra/ast/Visitor.h"

#include <unordered_map>
#include <string>
namespace umbra {
    template<template <typename> class Ptr, typename ImplClass, typename RetTy, class... ParamTys>
    class BaseV;
}

namespace umbra {
namespace code_gen {

    inline llvm::Type* builtinTypeToLLVMType(BuiltinType bT, llvm::LLVMContext& Ctxt){
        switch (bT){
            case BuiltinType::Int:
                return llvm::Type::getInt32Ty(Ctxt);
            case BuiltinType::Void:
                return llvm::Type::getVoidTy(Ctxt);
            case BuiltinType::Bool:
                return llvm::Type::getInt1Ty(Ctxt);
            case BuiltinType::Char:
                return llvm::Type::getInt8Ty(Ctxt);
            case BuiltinType::String:
                return llvm::Type::getInt8Ty(Ctxt)->getPointerTo();
            default:
                return llvm::Type::getVoidTy(Ctxt);
        }
    }


    class CodegenVisitor : public umbra::BaseV<std::unique_ptr, CodegenVisitor, llvm::Value*> {
        public:
        explicit CodegenVisitor(CodegenContext& Ctxt) : Ctxt(Ctxt) {}

        llvm::Value* visitProgramNode(ProgramNode* node);
        llvm::Value* visitFunctionDefinition(FunctionDefinition* node);
        llvm::Value* visitExpressionStatement(ExpressionStatement* node);
        llvm::Value* visitPrimaryExpression(PrimaryExpression* node);
        llvm::Value* visitStringLiteral(StringLiteral* node);
        llvm::Value* visitFunctionCall(FunctionCall* node);
        llvm::Value* visitReturnExpression(ReturnExpression* node);

        private:
        llvm::Value* emitExpr(Expression* expr);
        CodegenContext& Ctxt;
    };

} // namespace code_gen
} // namespace umbra
