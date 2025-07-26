#include "umbra/codegen/visitors/CodegenVisitor.h"
#include "umbra/codegen/context/CodegenContext.h"
#include <cstdint>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>
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


    }
}
