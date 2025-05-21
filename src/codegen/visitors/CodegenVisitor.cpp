#include "CodegenVisitor.h"
#include "../context/CodegenContext.h"
#include "../../semantic/Symbol.h" // Include the Symbol header
#include <cstdint>
#include <llvm-14/llvm/ADT/APInt.h>
#include <llvm-14/llvm/ADT/ArrayRef.h>
#include <llvm-14/llvm/IR/BasicBlock.h>
#include <llvm-14/llvm/IR/Constant.h>
#include <llvm-14/llvm/IR/Constants.h>
#include <llvm-14/llvm/IR/DerivedTypes.h>
#include <llvm-14/llvm/IR/Instructions.h>
#include <llvm-14/llvm/IR/Type.h>
#include <llvm-14/llvm/IR/Value.h>
#include <llvm-14/llvm/Support/Casting.h>

namespace umbra{

    namespace code_gen{


        CodegenVisitor::CodegenVisitor(CodegenContext& context) : context(context), lastLLVMValue(nullptr) {}

        llvm::Value* CodegenVisitor::getLastValue() const {
            return lastLLVMValue;
        }

        llvm::Type* CodegenVisitor::mapBuiltinTypeToLLVMType(const umbra::Type& type) const {
            switch (type.builtinType) {
                case BuiltinType::Int:
                    return llvm::Type::getInt32Ty(context.llvmContext);
                case BuiltinType::Float:
                    return llvm::Type::getFloatTy(context.llvmContext);
                case BuiltinType::Double:
                    return llvm::Type::getDoubleTy(context.llvmContext);
                case BuiltinType::Char:
                    return llvm::Type::getInt8Ty(context.llvmContext);
                case BuiltinType::Bool:
                    return llvm::Type::getInt1Ty(context.llvmContext);
                case BuiltinType::Void:
                    return llvm::Type::getVoidTy(context.llvmContext);
                default:
                    throw std::runtime_error("Unsupported type");
            }
        }

        std::string CodegenVisitor::createFormatStringSpecifier(llvm::Type* type){
            if (type->isIntegerTy(32)) {
                return "%d";
            } else if (type->isFloatTy()) {
                return "%f";
            } else if (type->isDoubleTy()) {
                return "%lf";
            } else if (type->isIntegerTy(8)) {
                return "%c";
            } else if (type->isIntegerTy(1)) {
                return "%d";
            } else {
                throw std::runtime_error("Unsupported type for print format specifier.");
            }
        }

        void CodegenVisitor::visit(umbra::ProgramNode& node){
            for (const auto& function : node.functions) {
                function->accept(*this);
            }
        }

        void CodegenVisitor::visit(umbra::StringLiteral& node){
            lastLLVMValue = context.llvmBuilder.CreateGlobalStringPtr(node.value, "generic_string");
            
            if(context.globalStrings.find(node.value) == context.globalStrings.end()){
                context.globalStrings[node.value] = lastLLVMValue;
            }

        }

        void CodegenVisitor::visit(umbra::NumericLiteral& node) {
            if (node.builtinType == BuiltinType::Int) {
                lastLLVMValue = llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(context.llvmContext), 
                    node.value, 
                    true
                );
            }
            else if (node.builtinType == BuiltinType::Float) {
                lastLLVMValue = llvm::ConstantFP::get(
                    llvm::Type::getFloatTy(context.llvmContext),
                    node.value
                );
            } else if (node.builtinType == BuiltinType::Double) {
                lastLLVMValue = llvm::ConstantFP::get(
                    llvm::Type::getDoubleTy(context.llvmContext),
                    node.value
                );
            } else {
                throw std::runtime_error("NumericLiteral with type not supported");
            }
        }
        
        void CodegenVisitor::visit(umbra::BooleanLiteral& node) {
            lastLLVMValue = llvm::ConstantInt::get(
                llvm::Type::getInt1Ty(context.llvmContext), node.value);
        }
        
        void CodegenVisitor::visit(umbra::CharLiteral& node) {
            lastLLVMValue = llvm::ConstantInt::get(
                llvm::Type::getInt8Ty(context.llvmContext), static_cast<uint8_t>(node.value));
        }        

        void CodegenVisitor::visit(umbra::FunctionDefinition& node){
            llvm::Function* functionBeingDefined = context.llvmModule.getFunction(node.name->name);

            if (functionBeingDefined && !functionBeingDefined->empty()) {
                std::cerr << "Warning: Function " << node.name->name << " already defined. Skipping redefinition." << std::endl;
                lastLLVMValue = functionBeingDefined;
                return;
            }

            std::vector<llvm::Type*> argTypes;
            if (node.parameters) {
                for(const auto& param : node.parameters->parameters){
                    if (!param.first) {
                        throw std::runtime_error("Null type for parameter " + param.second->name + " in function " + node.name->name);
                    }
                    argTypes.push_back(mapBuiltinTypeToLLVMType(*param.first));
                }
            }

            if (!node.returnType) {
                throw std::runtime_error("Null return type for function " + node.name->name);
            }
            llvm::Type* returnType = mapBuiltinTypeToLLVMType(*node.returnType);

            llvm::FunctionType* FT = llvm::FunctionType::get(returnType, argTypes, false);


            if (!functionBeingDefined) {
                functionBeingDefined = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, node.name->name, &context.llvmModule);
            } else if (functionBeingDefined->getFunctionType() != FT) {

                throw std::runtime_error("Function " + node.name->name + " redeclared with different signature.");
            }



            llvm::BasicBlock *BB = llvm::BasicBlock::Create(context.llvmContext, "entry", functionBeingDefined);
            context.llvmBuilder.SetInsertPoint(BB);

            context.namedValues.clear();
            if (node.parameters) {
                unsigned Idx = 0;
                for (auto &Arg : functionBeingDefined->args()) {

                    Arg.setName(node.parameters->parameters[Idx].second->name);

                    llvm::AllocaInst *Alloca = context.llvmBuilder.CreateAlloca(Arg.getType(), nullptr, Arg.getName());
                    context.llvmBuilder.CreateStore(&Arg, Alloca);
                    context.namedValues[std::string(Arg.getName())] = Alloca;
                    Idx++;
                }
            }

            for (const auto& stmt : node.body) {
                stmt->accept(*this);
            }


            if (node.returnValue) {
                node.returnValue->accept(*this);
                llvm::Value* retLLVMValue = getLastValue();
                if (retLLVMValue) { 
                    context.llvmBuilder.CreateRet(retLLVMValue);
                } else {

                    if (returnType->isVoidTy()) {
                        context.llvmBuilder.CreateRetVoid();
                    } else {
                        std::cerr << "Warning: Non-void function " << node.name->name << " might be missing a return value generation." << std::endl;

                        context.llvmBuilder.CreateRet(llvm::UndefValue::get(returnType));
                    }
                }
            } else {

                if (returnType->isVoidTy()) {
                    context.llvmBuilder.CreateRetVoid();
                } else {

                    std::cerr << "Error: Non-void function " << node.name->name << " is missing an explicit return value node and does not return void." << std::endl;

                     context.llvmBuilder.CreateRet(llvm::UndefValue::get(returnType));
                }
            }
            lastLLVMValue = functionBeingDefined;
        }

        void CodegenVisitor::visit(umbra::FunctionCall& node){
            llvm::Function* targetFunction = nullptr;
            const std::string& calledFunctionName = node.functionName->name;
            
            if (calledFunctionName == "print") {
                targetFunction = context.getPrintfFunction();
                if (!targetFunction) {
                    throw std::runtime_error("Failed to get or declare 'printf' function from context.");
                }
                
                std::string formatString = "";
                std::vector<llvm::Value*> printfArgs;
                
                for (const auto& argExpr : node.arguments) {
                    argExpr->accept(*this);
                    llvm::Value* argValue = getLastValue();

                    if (!argValue) {
                         throw std::runtime_error("Attempted to print a null value.");
                    }

                    llvm::Type* argLLVMType = argValue->getType();
                    
                    if (argLLVMType->isPointerTy()) {
                        if (argLLVMType->getPointerElementType()->isIntegerTy(8)) {
                            formatString += "%s";
                            printfArgs.push_back(argValue);
                        } else {
                            argValue = context.llvmBuilder.CreateLoad(
                                argLLVMType->getPointerElementType(),
                                argValue
                            );
                            argLLVMType = argValue->getType();
                            formatString += createFormatStringSpecifier(argLLVMType);
                            printfArgs.push_back(argValue);
                        }
                    } else {
                        formatString += createFormatStringSpecifier(argLLVMType);
                        printfArgs.push_back(argValue);
                    }
                    formatString += " ";
                }
                
                if (!formatString.empty()) {
                    formatString.pop_back(); 
                }

                llvm::Value* formatStrPtr = context.llvmBuilder.CreateGlobalStringPtr(formatString.c_str(), "printf_format_str");
                
                std::vector<llvm::Value*> callArgs;
                callArgs.push_back(formatStrPtr);
                
                for (llvm::Value* val : printfArgs) {
                    callArgs.push_back(val);
                }

                lastLLVMValue = context.llvmBuilder.CreateCall(targetFunction, callArgs, "calltmp_" + calledFunctionName);
                
            } else {
                std::vector<llvm::Value*> llvmArgs;
                for (const auto& argExpr : node.arguments) {
                    argExpr->accept(*this);
                    llvmArgs.push_back(getLastValue());
                }
                targetFunction = context.llvmModule.getFunction(calledFunctionName);
                if (!targetFunction) {
                    throw std::runtime_error("User-defined function not found in LLVM module: " + calledFunctionName);
                }
                lastLLVMValue = context.llvmBuilder.CreateCall(targetFunction, llvmArgs, "calltmp_" + calledFunctionName);
            }
        }

        void CodegenVisitor::visit(umbra::ExpressionStatement& node){
            node.exp->accept(*this);
            lastLLVMValue = getLastValue();
        }

        void CodegenVisitor::visit(umbra::PrimaryExpression& node){
            if (node.identifier) {
                node.identifier->accept(*this);
                lastLLVMValue = getLastValue();
            } else if (node.literal) {
                node.literal->accept(*this);
                lastLLVMValue = getLastValue();
            } else if (node.functionCall) {
                node.functionCall->accept(*this);
                lastLLVMValue = getLastValue();
            }
        }

        void CodegenVisitor::visit(umbra::VariableDeclaration& node){
            
            llvm::Type* llvmType = mapBuiltinTypeToLLVMType(*node.type);

            llvm::AllocaInst* alloca = context.llvmBuilder.CreateAlloca(
                llvmType,
                nullptr,
                node.name->name
            );

            if (!node.initializer) {
                if (llvmType->isIntegerTy()) {
                    context.llvmBuilder.CreateStore(
                        llvm::ConstantInt::get(llvmType, 0),
                        alloca
                    );
                } else if (llvmType->isFloatingPointTy()) {
                    context.llvmBuilder.CreateStore(
                        llvm::ConstantFP::get(llvmType, 0.0),
                        alloca
                    );
                } else if (llvmType->isPointerTy()) {
                    context.llvmBuilder.CreateStore(
                        llvm::ConstantPointerNull::get(
                        llvm::cast<llvm::PointerType>(llvmType)),
                        alloca
                    );
                }
            } else {
                node.initializer->accept(*this);
                llvm::Value* initValue = getLastValue();
                context.llvmBuilder.CreateStore(initValue, alloca);
            }

            context.namedValues[node.name->name] = alloca;
            lastLLVMValue = alloca;
        }

        void CodegenVisitor::visit(umbra::AssignmentStatement& node){
            node.value->accept(*this);
            llvm::Value* valueToStore = getLastValue();

            llvm::Value* targetAlloca = context.namedValues[node.target->name];
            if (!targetAlloca) {
                throw std::runtime_error("Unknown variable name: " + node.target->name);
            }

            llvm::Type* targetType = targetAlloca->getType()->getPointerElementType();

            if (node.index) {
                node.index->accept(*this);
                llvm::Value* indexValue = getLastValue();

                // Index to dereference the pointer and actual array index value
                std::vector<llvm::Value*> indices = {
                    llvm::ConstantInt::get(context.llvmContext, llvm::APInt(32, 0)),
                    indexValue
                };

                    llvm::Value* elementPtr = context.llvmBuilder.CreateGEP(
                        targetType,
                        targetAlloca,
                        indices,
                        node.target->name + ".idx"
                    );

                    context.llvmBuilder.CreateStore(valueToStore, elementPtr);
            } else {
                context.llvmBuilder.CreateStore(valueToStore, targetAlloca);
            }

            lastLLVMValue = valueToStore;
        }
        
        void CodegenVisitor::visit(umbra::RepeatTimesStatement& node){
            node.times->accept(*this);
            llvm::Value* timesValue = getLastValue();

            llvm::Function* currentFunction = context.llvmBuilder.GetInsertBlock()->getParent();

            llvm::BasicBlock* conditionBB = llvm::BasicBlock::Create(
                context.llvmContext, 
                "repeat.times.cond", 
                currentFunction);
            llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(
                context.llvmContext, 
                "repeat.times.body");
            llvm::BasicBlock* endBB = llvm::BasicBlock::Create(
                context.llvmContext, 
                "repeat.times.end");

            llvm::AllocaInst* counter = context.llvmBuilder.CreateAlloca(
                llvm::Type::getInt32Ty(context.llvmContext),
                nullptr, "repeat.counter");
            context.llvmBuilder.CreateStore(
                llvm::ConstantInt::get(context.llvmContext, llvm::APInt(32, 0)),
                counter);

            context.llvmBuilder.CreateBr(conditionBB);
            context.llvmBuilder.SetInsertPoint(conditionBB);

            llvm::Value* counterValue = context.llvmBuilder.CreateLoad(
                llvm::Type::getInt32Ty(context.llvmContext),
                counter,
                "counter.load"
            );
            llvm::Value* condValue = context.llvmBuilder.CreateICmpSLT(
                counterValue,
                timesValue,
                "repeat.times.cond");
            
            context.llvmBuilder.CreateCondBr(condValue, bodyBB, endBB);

            currentFunction->getBasicBlockList().push_back(bodyBB);
            context.llvmBuilder.SetInsertPoint(bodyBB);

            for (const auto& statement : node.body) {
                statement->accept(*this);
            }

            llvm::Value* newCounterValue = context.llvmBuilder.CreateAdd(
                counterValue,
                llvm::ConstantInt::get(context.llvmContext, llvm::APInt(32, 1)),
                "counter.increment");
            context.llvmBuilder.CreateStore(newCounterValue, counter);

            context.llvmBuilder.CreateBr(conditionBB);

            currentFunction->getBasicBlockList().push_back(endBB);
            context.llvmBuilder.SetInsertPoint(endBB);

            lastLLVMValue = nullptr;
        }

        void CodegenVisitor::visit(umbra::RepeatIfStatement& node) {

        }
    }
}