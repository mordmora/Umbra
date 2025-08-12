#include "umbra/codegen/visitors/CodegenVisitor.h"
#include "umbra/codegen/context/CodegenContext.h"
#include "umbra/ast/Visitor.h"
#include "umbra/semantic/SymbolTable.h"
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Casting.h>

namespace umbra {
namespace code_gen {

    llvm::Value* CodegenVisitor::visitProgramNode(ProgramNode* node){
        // Visitar todas las funciones
        for(auto& F : node->functions){
            visit(F.get());
        }
        return nullptr;
    }

    llvm::Value* CodegenVisitor::visitFunctionDefinition(FunctionDefinition* node){
        // Tipos de retorno y params a partir de la firma semántica
        llvm::Type* retTy = builtinTypeToLLVMType(node->returnType->builtinType, Ctxt.llvmContext);

        std::vector<llvm::Type*> paramTys;
        if (node->parameters) {
            for (auto& p : node->parameters->parameters) {
                paramTys.push_back(builtinTypeToLLVMType(p.first->builtinType, Ctxt.llvmContext));
            }
        }
        auto* FT = llvm::FunctionType::get(retTy, paramTys, false);
        llvm::Function* F = llvm::Function::Create(
            FT,
            llvm::Function::ExternalLinkage,
            node->name->name,
            Ctxt.llvmModule
        );

        // Nombrar argumentos
        if (node->parameters) {
            unsigned idx = 0;
            for (auto& arg : F->args()) {
                arg.setName(node->parameters->parameters[idx++].second->name);
            }
        }

        // Crear bloque de entrada
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(Ctxt.llvmContext, "entry", F);
        Ctxt.llvmBuilder.SetInsertPoint(entry);

        // Emitir cuerpo
        for (auto& S : node->body) {
            visit(S.get());
        }

        // Manejar retorno implícito si la función es void y no hubo return
        if (retTy->isVoidTy()) {
            if (!entry->getTerminator()) {
                Ctxt.llvmBuilder.CreateRetVoid();
            }
        }

        return F;
    }

    llvm::Value* CodegenVisitor::visitExpressionStatement(ExpressionStatement* node) {
        if (node->exp) {
            return visit(node->exp.get());
        }
        return nullptr;
    }

    static llvm::Constant* getOrCreateGlobalString(CodegenContext& Ctxt, const std::string& str, const std::string& nameHint){
        auto it = Ctxt.globalStrings.find(str);
        if (it != Ctxt.globalStrings.end()) {
            return llvm::cast<llvm::Constant>(it->second);
        }
        auto* constant = Ctxt.llvmBuilder.CreateGlobalStringPtr(str, nameHint);
        Ctxt.globalStrings[str] = constant;
        return constant;
    }

    llvm::Value* CodegenVisitor::visitStringLiteral(StringLiteral* node){
        return getOrCreateGlobalString(Ctxt, node->value, "str");
    }

    llvm::Value* CodegenVisitor::visitPrimaryExpression(PrimaryExpression* node){
        // Delegar según el contenido real
        if (node->functionCall) return visit(node->functionCall.get());
        if (node->literal) return visit(node->literal.get());
        if (node->parenthesized) return emitExpr(node->parenthesized.get());
        // Identificadores, arrays, etc., aún no implementados para codegen mínimo
        return nullptr;
    }

    llvm::Value* CodegenVisitor::emitExpr(Expression* expr){
        if (!expr) return nullptr;
        return visit(expr);
    }

    llvm::Value* CodegenVisitor::visitFunctionCall(FunctionCall* node){
        if (!node || !node->functionName) return nullptr;
        const std::string& fname = node->functionName->name;
        // Por ahora solo soportamos print(string) -> void mapeado a printf
        if (fname == "print") {
            if (node->arguments.size() == 1) {
                auto* fmt = getOrCreateGlobalString(Ctxt, "%s", "fmt");
                llvm::Value* argV = emitExpr(node->arguments[0].get());
                if (!argV) return nullptr;
                llvm::Function* printfFn = Ctxt.getPrintfFunction();
                return Ctxt.llvmBuilder.CreateCall(printfFn, {fmt, argV});
            }
            return nullptr;
        }
        // el soporte es mínimo. En el futuro, usar getFunction/getOrInsertFunction.
        return nullptr;
    }

    llvm::Value* CodegenVisitor::visitReturnExpression(ReturnExpression* node){
        if (!node->returnValue) {
            Ctxt.llvmBuilder.CreateRetVoid();
            return nullptr;
        }
        llvm::Value* v = emitExpr(node->returnValue.get());
        Ctxt.llvmBuilder.CreateRet(v);
        return v;
    }

} // namespace code_gen
} // namespace umbra
