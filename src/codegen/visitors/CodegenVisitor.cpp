#include "CodegenVisitor.h"
#include "../context/CodegenContext.h"
#include "../../semantic/Symbol.h" // Include the Symbol header

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
                default:
                    throw std::runtime_error("Unsupported type");
            }
        }

        void CodegenVisitor::visit(umbra::StringLiteral& node){
            lastLLVMValue = context.llvmBuilder.CreateGlobalStringPtr(node.value, "generic_string");
            
            if(context.globalStrings.find(node.value) == context.globalStrings.end()){
                context.globalStrings[node.value] = lastLLVMValue;
            }

        }

        void CodegenVisitor::visit(umbra::FunctionDefinition& node){
            llvm::Function* theFunction = context.llvmModule.getFunction(node.name->name);
            if(!theFunction){
                std::vector<llvm::Type*> argTypes;
                for(const auto& param : node.parameters->parameters){
                    argTypes.push_back(mapBuiltinTypeToLLVMType(param.first->builtinType));
                }

                llvm::Type* returnType = mapBuiltinTypeToLLVMType(node.returnType->builtinType);

               theFunction = llvm::Function::Create(
                   llvm::FunctionType::get(returnType, argTypes, false),
                   llvm::Function::ExternalLinkage,
                   node.name->name,
                   &context.llvmModule
               );

               llvm::BasicBlock *BB = llvm::BasicBlock::Create(context.llvmContext, "entry", theFunction);
                context.llvmBuilder.SetInsertPoint(BB);
            }

            // Create a new scope for the function
            context.namedValues.clear();
 
            for (size_t i = 0; i < node.parameters->parameters.size(); ++i) {
                const auto& param = node.parameters->parameters[i];
                llvm::Value* alloca = context.llvmBuilder.CreateAlloca(
                    mapBuiltinTypeToLLVMType(*param.first), nullptr, param.second->name);
                context.namedValues[param.second->name] = alloca;
            }

            for (const auto& stmt : node.body) {
                stmt->accept(*this);
            }
            if (node.returnValue) {
                node.returnValue->accept(*this);
                llvm::Value* returnValue = getLastValue();
                context.llvmBuilder.CreateRet(returnValue);
            } else {
                context.llvmBuilder.CreateRetVoid();
            }
            lastLLVMValue = theFunction;
        }
    }
}