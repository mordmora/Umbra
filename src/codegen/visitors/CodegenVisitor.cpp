#include "umbra/codegen/visitors/CodegenVisitor.h"
#include "umbra/ast/Visitor.h"
#include "umbra/codegen/context/CodegenContext.h"
#include "umbra/semantic/SymbolTable.h"
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>

namespace umbra {
namespace code_gen {

llvm::Value *CodegenVisitor::visitProgramNode(ProgramNode *node) {
    // Visitar todas las funciones
    for (auto &F : node->functions) {
        visit(F.get());
    }
    return nullptr;
}

llvm::Value *CodegenVisitor::visitFunctionDefinition(FunctionDefinition *node) {
    // Tipos de retorno y params a partir de la firma semántica
    llvm::Type *retTy = builtinTypeToLLVMType(node->returnType->builtinType, Ctxt.llvmContext);

    std::vector<llvm::Type *> paramTys;
    if (node->parameters) {
        for (auto &p : node->parameters->parameters) {
            paramTys.push_back(builtinTypeToLLVMType(p.first->builtinType, Ctxt.llvmContext));
        }
    }
    auto *FT = llvm::FunctionType::get(retTy, paramTys, false);
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage,
                                               node->name->name, Ctxt.llvmModule);

    // Nombrar argumentos y meterlos al mapa namedValues como locales
    if (node->parameters) {
        unsigned idx = 0;
        for (auto &arg : F->args()) {
            auto &pname = node->parameters->parameters[idx++].second->name;
            arg.setName(pname);
            Ctxt.namedValues[pname] = &arg;
        }
    }

    // Crear bloque de entrada
    llvm::BasicBlock *entry = llvm::BasicBlock::Create(Ctxt.llvmContext, "entry", F);
    Ctxt.llvmBuilder.SetInsertPoint(entry);

    // Emitir cuerpo
    for (auto &S : node->body) {
        visit(S.get());
    }


    // Si el bloque actual no tiene terminador, insertar retorno por defecto acorde al tipo
    llvm::BasicBlock *curBB = Ctxt.llvmBuilder.GetInsertBlock();
    if (curBB && !curBB->getTerminator()) {
        if (retTy->isVoidTy()) {
            Ctxt.llvmBuilder.CreateRetVoid();
        } else if (retTy->isIntegerTy(32)) {
            // retorno por defecto 0 si no se emitió return
            Ctxt.llvmBuilder.CreateRet(
                llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctxt.llvmContext), 0, true));
        } else {
            // fallback para otros tipos no soportados aún
            Ctxt.llvmBuilder.CreateRetVoid();
        }
    }

    // Limpiar valores nombrados de parámetros para el próximo contexto de función
    if (node->parameters) {
        for (auto &p : node->parameters->parameters) {
            Ctxt.namedValues.erase(p.second->name);
        }
    }

    return F;
}

llvm::Value *CodegenVisitor::visitExpressionStatement(ExpressionStatement *node) {
    if (node->exp) {
        return visit(node->exp.get());
    }
    return nullptr;
}

static llvm::Constant *getOrCreateGlobalString(CodegenContext &Ctxt, const std::string &str,
                                               const std::string &nameHint) {
    auto it = Ctxt.globalStrings.find(str);
    if (it != Ctxt.globalStrings.end()) {
        return llvm::cast<llvm::Constant>(it->second);
    }
    // Create a constant [N x i8] with the contents of str (including null)
    llvm::Constant *gv = Ctxt.llvmBuilder.CreateGlobalString(str, nameHint);
    Ctxt.globalStrings[str] = gv;
    return gv;
}

llvm::Value *CodegenVisitor::visitStringLiteral(StringLiteral *node) {
    return getOrCreateGlobalString(Ctxt, node->value, "str");
}

llvm::Value *CodegenVisitor::visitBooleanLiteral(BooleanLiteral *node) {
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(Ctxt.llvmContext), node->value);
}

llvm::Value *CodegenVisitor::visitNumericLiteral(NumericLiteral *node) {
    switch (node->builtinType) {
    case BuiltinType::Int:
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctxt.llvmContext), (int)node->value,
                                      true);
    case BuiltinType::Float: {
        return llvm::ConstantFP::get(llvm::Type::getFloatTy(Ctxt.llvmContext), node->value);
    }
    default:
        return nullptr;
    }
}

llvm::Value *CodegenVisitor::visitIdentifier(Identifier *node) {
    auto it = Ctxt.namedValues.find(node->name);
    if(it == Ctxt.namedValues.end()) return nullptr;

    llvm::Value* val = it->second;
    if(auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(val)){
        return Ctxt.llvmBuilder.CreateLoad(alloca->getAllocatedType(), alloca, node->name + ".ld");
    }
    return val;
}

llvm::Value *CodegenVisitor::visitPrimaryExpression(PrimaryExpression *node) {
    if (node->functionCall)
        return visit(node->functionCall.get());
    if (node->literal)
        return visit(node->literal.get());
    if (node->identifier)
        return visit(node->identifier.get());
    if (node->parenthesized)
        return emitExpr(node->parenthesized.get());
    if (node->arrayAccess)
        return visitArrayAccess(node->arrayAccess.get());
    return nullptr;
}

llvm::Value *CodegenVisitor::emitExpr(Expression *expr) {
    if (!expr)
        return nullptr;
    return visit(expr);
}

static bool isIntTy(llvm::Value *V, unsigned bits) { return V && V->getType()->isIntegerTy(bits); }

static bool isBoolLike(llvm::Value *V) { return V && V->getType()->isIntegerTy(1); }

static llvm::Value *toBool(llvm::IRBuilder<> &B, llvm::Value *V) {
    if (!V)
        return V;
    if (isBoolLike(V))
        return V;
    if (isIntTy(V, 32)) {
        return B.CreateICmpNE(V, llvm::ConstantInt::get(V->getType(), 0), "tobool");
    }
    // Fallback: no soportado aún
    return V;
}

llvm::Value *CodegenVisitor::visitBinaryExpression(BinaryExpression *node) {
    llvm::Value *L = emitExpr(node->left.get());
    llvm::Value *R = emitExpr(node->right.get());
    if (!L || !R)
        return nullptr;
    const std::string &Op = node->op;

    // Aritméticos básicos
    if (Op == "+")
        return Ctxt.llvmBuilder.CreateAdd(L, R, "addtmp");
    if (Op == "-")
        return Ctxt.llvmBuilder.CreateSub(L, R, "subtmp");
    if (Op == "*")
        return Ctxt.llvmBuilder.CreateMul(L, R, "multmp");
    if (Op == "/")
        return Ctxt.llvmBuilder.CreateSDiv(L, R, "divtmp");
    if (Op == "%")
        return Ctxt.llvmBuilder.CreateSRem(L, R, "modtmp");

    // Lógicos palabra clave
    if (Op == "and") {
        L = toBool(Ctxt.llvmBuilder, L);
        R = toBool(Ctxt.llvmBuilder, R);
        return Ctxt.llvmBuilder.CreateAnd(L, R, "andtmp");
    }
    if (Op == "or") {
        L = toBool(Ctxt.llvmBuilder, L);
        R = toBool(Ctxt.llvmBuilder, R);
        return Ctxt.llvmBuilder.CreateOr(L, R, "ortmp");
    }

    // Comparadores palabra clave
    if (Op == "less_than")
        return Ctxt.llvmBuilder.CreateICmpSLT(L, R, "cmptmp");
    if (Op == "greater_than")
        return Ctxt.llvmBuilder.CreateICmpSGT(L, R, "cmptmp");
    if (Op == "less_or_equal")
        return Ctxt.llvmBuilder.CreateICmpSLE(L, R, "cmptmp");
    if (Op == "greater_or_equal")
        return Ctxt.llvmBuilder.CreateICmpSGE(L, R, "cmptmp");
    if (Op == "equal" || Op == "==")
        return Ctxt.llvmBuilder.CreateICmpEQ(L, R, "cmptmp");
    if (Op == "different" || Op == "!=")
        return Ctxt.llvmBuilder.CreateICmpNE(L, R, "cmptmp");

    return nullptr;
}

llvm::Value *CodegenVisitor::visitFunctionCall(FunctionCall *node) {
    if (!node || !node->functionName)
        return nullptr;
    const std::string &fname = node->functionName->name;
    // Por ahora solo soportamos print(string|int, ...) -> void mapeado a printf
    if (fname == "print") {
        if (!node->arguments.empty()) {
            // Construir formato dinámico
            std::string fmtStr;
            std::vector<llvm::Value *> callArgs;

            // Obtener el formato de string
            auto *strLit = dynamic_cast<StringLiteral *>(node->arguments[0].get());
            if (!strLit) return nullptr;

            fmtStr = strLit->value;
            size_t argIdx = 1;
            size_t pos = 0;

            while((pos = fmtStr.find("{}")) != std::string::npos && argIdx < node->argTypes.size()) {
                auto &arg = node->argTypes[argIdx];
                std::string fmtCode = "%d";
                if (arg == SemanticType::String)
                    fmtCode = "%s";
                else if (arg == SemanticType::Int || arg == SemanticType::Bool)
                    fmtCode = "%d";
                else if (arg == SemanticType::Float)
                    fmtCode = "%f";
                else if (arg == SemanticType::Char)
                    fmtCode = "%c";
                fmtStr.replace(pos, 2, fmtCode);
                ++argIdx;
            }
            fmtStr += "\n";
            auto *fmt = getOrCreateGlobalString(Ctxt, fmtStr, "fmt");
            callArgs.push_back(fmt);

            // Agregar los argumentos
            for (size_t i = 1; i < node->arguments.size(); ++i) {
                llvm::Value *v = emitExpr(node->arguments[i].get());
                if (v && v->getType()->isIntegerTy(1)) {
                    // Extender bool a int32 para printf
                    v = Ctxt.llvmBuilder.CreateZExt(v, llvm::Type::getInt32Ty(Ctxt.llvmContext));
                }
                callArgs.push_back(v);
            }
            // Llamar a printf
            llvm::Function *printfFunc = Ctxt.getPrintfFunction();
            return Ctxt.llvmBuilder.CreateCall(printfFunc, callArgs);
        }
        return nullptr;
    }
    // Funciones del usuario: buscar en el módulo y llamar
    llvm::Function *callee = Ctxt.llvmModule.getFunction(fname);
    if (!callee) {
        return nullptr;
    }
    std::vector<llvm::Value *> argsV;
    argsV.reserve(node->arguments.size());
    for (auto &a : node->arguments) {
        llvm::Value *v = emitExpr(a.get());
        argsV.push_back(v);
    }
    return Ctxt.llvmBuilder.CreateCall(callee, argsV);
}

llvm::Value *CodegenVisitor::visitReturnExpression(ReturnExpression *node) {
    if (!node->returnValue) {
        Ctxt.llvmBuilder.CreateRetVoid();
        return nullptr;
    }
    llvm::Value *v = emitExpr(node->returnValue.get());
    // Ajustar bool -> i32 si la función retorna i32
    llvm::Function *F = Ctxt.llvmBuilder.GetInsertBlock()->getParent();
    if (F && F->getReturnType()->isIntegerTy(32) && v && v->getType()->isIntegerTy(1)) {
        v = Ctxt.llvmBuilder.CreateZExt(v, llvm::Type::getInt32Ty(Ctxt.llvmContext));
    }
    Ctxt.llvmBuilder.CreateRet(v);
    return v;
}

llvm::Value* CodegenVisitor::visitVariableDeclaration(VariableDeclaration* node){

    const std::string& vName = node->name->name;
    llvm::Type* baseType = builtinTypeToLLVMType(node->type.get()->builtinType, Ctxt.llvmContext);
    llvm::Type* vType = baseType;
    
    if(node->type->arrayDimensions > 0 && !node->type->arraySizes.empty()){
        for(int i = node->type->arrayDimensions - 1; i >= 0; --i){
            llvm::Value* sizeVal = emitExpr(node->type->arraySizes[i].get());
            uint64_t size = 0;
            
            if(auto constInt = llvm::dyn_cast<llvm::ConstantInt>(sizeVal)){
                size = constInt->getZExtValue();
            } else {
                size = 1;
            }
            
            vType = llvm::ArrayType::get(vType, size);
        }
    }

    llvm::Function* F = Ctxt.llvmBuilder.GetInsertBlock()->getParent();
    llvm::IRBuilder<> entryBuilder(&F->getEntryBlock(), F->getEntryBlock().begin());
    auto* alloca = entryBuilder.CreateAlloca(vType, nullptr, vName);

    Ctxt.namedValues[vName] = alloca;
    Ctxt.valueTypes[alloca] = vType;

    if(node->type->arrayDimensions > 0){
        return alloca;
    }

    llvm::Value* initVal = nullptr;
    if(node->initializer){
        initVal = emitExpr(node->initializer.get());

        if(initVal && initVal->getType()->isVoidTy()){
            initVal = llvm::Constant::getNullValue(baseType);
        }
        else if(initVal && baseType->isIntegerTy(32) && initVal->getType()->isIntegerTy(1)){
            initVal = Ctxt.llvmBuilder.CreateZExt(initVal, baseType, vName + ".zext");
        }else if(initVal && baseType->isFloatTy() && initVal->getType()->isIntegerTy(32)){
            initVal = Ctxt.llvmBuilder.CreateSIToFP(initVal, baseType, vName + ".sitofp");
        }
    }else{
        initVal = llvm::Constant::getNullValue(baseType);
    }

    if(initVal){
        Ctxt.llvmBuilder.CreateStore(initVal, alloca);
    }
    return alloca;

}

llvm::Value* CodegenVisitor::visitAssignmentStatement(AssignmentStatement* node){

    llvm::Value* rhs = emitExpr(node->value.get());
    if(!rhs) return nullptr;

    if(auto id = dynamic_cast<Identifier*>(node->target.get())){
        auto it = Ctxt.namedValues.find(id->name);
        if(it == Ctxt.namedValues.end()){
            return nullptr;
        }

        llvm::Value* ptr = it->second;
        auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(ptr);
        if(!alloca) return nullptr;

        llvm::Type* destTy = alloca->getAllocatedType();
        if(destTy->isIntegerTy(32) && rhs->getType()->isIntegerTy(32)){
            rhs = Ctxt.llvmBuilder.CreateZExt(rhs, destTy, "bool_to_i32");
        }else if (destTy->isFloatTy() && rhs->getType()->isIntegerTy(32)){
            rhs = Ctxt.llvmBuilder.CreateSIToFP(rhs, destTy, "i32_to_float");
        }
        return Ctxt.llvmBuilder.CreateStore(rhs, alloca);
    }
    
    if(auto primaryExpr = dynamic_cast<PrimaryExpression*>(node->target.get())){
        if(primaryExpr->exprType == PrimaryExpression::ARRAY_ACCESS && primaryExpr->arrayAccess){
            llvm::Value* elementPtr = getArrayElementPtr(primaryExpr->arrayAccess.get());
            if(!elementPtr) return nullptr;
            return Ctxt.llvmBuilder.CreateStore(rhs, elementPtr);
        }
    }
    
    return nullptr;
}

llvm::Value* CodegenVisitor::getArrayElementPtr(ArrayAccessExpression* node){
    llvm::Value* basePtr = nullptr;
    llvm::Type* baseType = nullptr;
    
    if(auto id = dynamic_cast<Identifier*>(node->array.get())){
        auto it = Ctxt.namedValues.find(id->name);
        if(it == Ctxt.namedValues.end()){
            return nullptr;
        }
        basePtr = it->second;
        
        auto typeIt = Ctxt.valueTypes.find(basePtr);
        if(typeIt != Ctxt.valueTypes.end()){
            baseType = typeIt->second;
        } else if(auto alloca = llvm::dyn_cast<llvm::AllocaInst>(basePtr)){
            baseType = alloca->getAllocatedType();
        }
    } else if(auto primaryExpr = dynamic_cast<PrimaryExpression*>(node->array.get())){
        if(primaryExpr->exprType == PrimaryExpression::ARRAY_ACCESS && primaryExpr->arrayAccess){
            basePtr = getArrayElementPtr(primaryExpr->arrayAccess.get());
            if(!basePtr) return nullptr;
            
            auto typeIt = Ctxt.valueTypes.find(basePtr);
            if(typeIt != Ctxt.valueTypes.end()){
                baseType = typeIt->second;
            }
        }
    } else {
        basePtr = emitExpr(node->array.get());
        if(!basePtr) return nullptr;
        
        auto typeIt = Ctxt.valueTypes.find(basePtr);
        if(typeIt != Ctxt.valueTypes.end()){
            baseType = typeIt->second;
        }
    }
    
    if(!basePtr || !baseType) return nullptr;
    
    llvm::Value* indexVal = emitExpr(node->index.get());
    if(!indexVal) return nullptr;
    // GEP expects 64-bit indices for pointer arithmetic
    if(indexVal->getType()->isIntegerTy() && indexVal->getType()->getIntegerBitWidth() < 64){
        indexVal = Ctxt.llvmBuilder.CreateZExt(indexVal, llvm::Type::getInt64Ty(Ctxt.llvmContext), "idx64");
    }

    std::vector<llvm::Value*> indices;
    // Always start with zero when indexing into an array value
    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(Ctxt.llvmContext), 0));
    indices.push_back(indexVal);
    
    llvm::Type* elementType = nullptr;
    if(auto arrayType = llvm::dyn_cast<llvm::ArrayType>(baseType)){
        elementType = arrayType->getElementType();
        
        llvm::Value* elementPtr = Ctxt.llvmBuilder.CreateInBoundsGEP(
            baseType,
            basePtr,
            indices,
            "arrayidx"
        );
        
        Ctxt.valueTypes[elementPtr] = elementType;
        return elementPtr;
    }
    
    return nullptr;
}

llvm::Value* CodegenVisitor::visitArrayAccess(ArrayAccessExpression* node){
    llvm::Value* elementPtr = getArrayElementPtr(node);
    if(!elementPtr) return nullptr;
    
    auto typeIt = Ctxt.valueTypes.find(elementPtr);
    if(typeIt == Ctxt.valueTypes.end()){
        return nullptr;
    }
    
    llvm::Type* elementType = typeIt->second;
    
    if(llvm::isa<llvm::ArrayType>(elementType)){
        return elementPtr;
    }
    
    return Ctxt.llvmBuilder.CreateLoad(
        elementType,
        elementPtr,
        "arrayload"
    );
}

llvm::Value *CodegenVisitor::visitIfStatement(IfStatement *node) {
    llvm::Function *F = Ctxt.llvmBuilder.GetInsertBlock()->getParent();
    // Crear bloques para cada rama y un bloque final de merge
    std::vector<llvm::BasicBlock *> thenBlocks;
    thenBlocks.reserve(node->branches.size());
    for (size_t i = 0; i < node->branches.size(); ++i) {
        thenBlocks.push_back(
            llvm::BasicBlock::Create(Ctxt.llvmContext, "if.then" + std::to_string(i), F));
    }
    llvm::BasicBlock *elseBlock = nullptr;
    if (!node->elseBranch.empty()) {
        elseBlock = llvm::BasicBlock::Create(Ctxt.llvmContext, "if.else", F);
    }
    llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(Ctxt.llvmContext, "if.end", F);

    // Emisión en cascada: cond0 ? then0 : cond1 ? then1 : ... : else/merge
    llvm::BasicBlock *nextCondBB = nullptr;
    for (size_t i = 0; i < node->branches.size(); ++i) {
        // Evaluar condición i en el bloque actual
        llvm::Value *condV = emitExpr(node->branches[i].condition.get());
        condV = toBool(Ctxt.llvmBuilder, condV);
        if (!condV)
            condV = llvm::ConstantInt::getFalse(Ctxt.llvmContext);

        // Preparar bloque para la siguiente condición o else/merge
        if (i + 1 < node->branches.size()) {
            nextCondBB =
                llvm::BasicBlock::Create(Ctxt.llvmContext, "if.cond" + std::to_string(i + 1), F);
        } else {
            nextCondBB = elseBlock ? elseBlock : mergeBB;
        }

        Ctxt.llvmBuilder.CreateCondBr(condV, thenBlocks[i], nextCondBB);

        // Emitir cuerpo de la rama i
        Ctxt.llvmBuilder.SetInsertPoint(thenBlocks[i]);
        for (auto &S : node->branches[i].body) {
            visit(S.get());
        }
        if (!thenBlocks[i]->getTerminator()) {
            Ctxt.llvmBuilder.CreateBr(mergeBB);
        }

        // Establecer punto de inserción para evaluar la siguiente condición
        Ctxt.llvmBuilder.SetInsertPoint(nextCondBB);
    }

    // Else opcional
    if (elseBlock) {
        for (auto &S : node->elseBranch) {
            visit(S.get());
        }
        if (!elseBlock->getTerminator()) {
            Ctxt.llvmBuilder.CreateBr(mergeBB);
        }
    }

    // Continuación
    Ctxt.llvmBuilder.SetInsertPoint(mergeBB);
    return nullptr;
}

llvm::Value *CodegenVisitor::visitRepeatTimesStatement(RepeatTimesStatement *node) {
    llvm::Function *F = Ctxt.llvmBuilder.GetInsertBlock()->getParent();

    // Evaluar la expresion times (numero de repeticiones)
    llvm::Value *timesVal = emitExpr(node->times.get());
    if (!timesVal)
        return nullptr;

    // Crear variable temporal para el contador
    llvm::AllocaInst *counterAlloca = Ctxt.llvmBuilder.CreateAlloca(
        llvm::Type::getInt32Ty(Ctxt.llvmContext), nullptr, "for.counter");
    Ctxt.llvmBuilder.CreateStore(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctxt.llvmContext), 0), counterAlloca);

    // Crear bloques baciso para el bucle
    llvm::BasicBlock *loopCondBB = llvm::BasicBlock::Create(Ctxt.llvmContext, "for.cond", F);
    llvm::BasicBlock *loopBodyBB = llvm::BasicBlock::Create(Ctxt.llvmContext, "for.body", F);
    llvm::BasicBlock *loopEndBB = llvm::BasicBlock::Create(Ctxt.llvmContext, "for.end", F);

    Ctxt.llvmBuilder.CreateBr(loopCondBB);

    Ctxt.llvmBuilder.SetInsertPoint(loopCondBB);
    llvm::Value *counterVal = Ctxt.llvmBuilder.CreateLoad(llvm::Type::getInt32Ty(Ctxt.llvmContext),
                                                          counterAlloca, "counter.load");
    llvm::Value *cond = Ctxt.llvmBuilder.CreateICmpSLT(counterVal, timesVal, "for.cmp");
    Ctxt.llvmBuilder.CreateCondBr(cond, loopBodyBB, loopEndBB);

    // loop.body: emitir cuerpo y aumentar contador
    Ctxt.llvmBuilder.SetInsertPoint(loopBodyBB);
    for (auto &stmt : node->body) {
        visit(stmt.get());
    }

    llvm::Value *currentCounterVal = Ctxt.llvmBuilder.CreateLoad(
        llvm::Type::getInt32Ty(Ctxt.llvmContext), counterAlloca, "current.counter.load");
    llvm::Value *inc = Ctxt.llvmBuilder.CreateAdd(
        currentCounterVal, llvm::ConstantInt::get(llvm::Type::getInt32Ty(Ctxt.llvmContext), 1),
        "for.inc");
    Ctxt.llvmBuilder.CreateStore(inc, counterAlloca);

    Ctxt.llvmBuilder.CreateBr(loopCondBB);

    // loop.end: continuar
    Ctxt.llvmBuilder.SetInsertPoint(loopEndBB);
    return nullptr;
}

llvm::Value* CodegenVisitor::visitIncrementExpression(IncrementExpression* node){
    llvm::Value* varPtr = nullptr;
    llvm::Type* varType = nullptr;
    
    if(auto id = dynamic_cast<Identifier*>(node->operand.get())){
        auto it = Ctxt.namedValues.find(id->name);
        if(it == Ctxt.namedValues.end()){
            return nullptr;
        }
        varPtr = it->second;
        if(auto alloca = llvm::dyn_cast<llvm::AllocaInst>(varPtr)){
            varType = alloca->getAllocatedType();
        }
    }
    else if(auto primaryExpr = dynamic_cast<PrimaryExpression*>(node->operand.get())){
        if(primaryExpr->exprType == PrimaryExpression::ARRAY_ACCESS && primaryExpr->arrayAccess){
            varPtr = getArrayElementPtr(primaryExpr->arrayAccess.get());
            if(!varPtr) return nullptr;
            
            auto typeIt = Ctxt.valueTypes.find(varPtr);
            if(typeIt != Ctxt.valueTypes.end()){
                varType = typeIt->second;
            }
        }
    }
    
    if(!varPtr || !varType){
        return nullptr;
    }
    
    llvm::Value* oldValue = Ctxt.llvmBuilder.CreateLoad(varType, varPtr, "inc.old");
    llvm::Value* newValue = nullptr;
    
    if(varType->isIntegerTy()){
        newValue = Ctxt.llvmBuilder.CreateAdd(
            oldValue,
            llvm::ConstantInt::get(varType, 1),
            "inc.result"
        );
    } else if(varType->isFloatTy()){
        newValue = Ctxt.llvmBuilder.CreateFAdd(
            oldValue,
            llvm::ConstantFP::get(varType, 1.0),
            "inc.result"
        );
    } else {
        return nullptr;
    }
    
    Ctxt.llvmBuilder.CreateStore(newValue, varPtr);
    
    return node->isPrefix ? newValue : oldValue;
}

llvm::Value* CodegenVisitor::visitDecrementExpression(DecrementExpression* node){
    llvm::Value* varPtr = nullptr;
    llvm::Type* varType = nullptr;
    
    if(auto id = dynamic_cast<Identifier*>(node->operand.get())){
        auto it = Ctxt.namedValues.find(id->name);
        if(it == Ctxt.namedValues.end()){
            return nullptr;
        }
        varPtr = it->second;
        if(auto alloca = llvm::dyn_cast<llvm::AllocaInst>(varPtr)){
            varType = alloca->getAllocatedType();
        }
    }
    else if(auto primaryExpr = dynamic_cast<PrimaryExpression*>(node->operand.get())){
        if(primaryExpr->exprType == PrimaryExpression::ARRAY_ACCESS && primaryExpr->arrayAccess){
            varPtr = getArrayElementPtr(primaryExpr->arrayAccess.get());
            if(!varPtr) return nullptr;
            
            auto typeIt = Ctxt.valueTypes.find(varPtr);
            if(typeIt != Ctxt.valueTypes.end()){
                varType = typeIt->second;
            }
        }
    }
    
    if(!varPtr || !varType){
        return nullptr;
    }
    
    llvm::Value* oldValue = Ctxt.llvmBuilder.CreateLoad(varType, varPtr, "dec.old");
    llvm::Value* newValue = nullptr;
    
    if(varType->isIntegerTy()){
        newValue = Ctxt.llvmBuilder.CreateSub(
            oldValue,
            llvm::ConstantInt::get(varType, 1),
            "dec.result"
        );
    } else if(varType->isFloatTy()){
        newValue = Ctxt.llvmBuilder.CreateFSub(
            oldValue,
            llvm::ConstantFP::get(varType, 1.0),
            "dec.result"
        );
    } else {
        return nullptr;
    }
    
    Ctxt.llvmBuilder.CreateStore(newValue, varPtr);
    
    return node->isPrefix ? newValue : oldValue;
}

} // namespace code_gen
} // namespace umbra
