#include "TypeCompatibility.h"


namespace umbra {
bool 
TypeCompatibility::areTypesCompatible(const Type& declType, RvalExpressionType exprType) {
    // Verificación directa de tipos
    if (declType.builtinType ==  BuiltinType::Int && exprType == RvalExpressionType::INTEGER) return true;
    if (declType.builtinType ==  BuiltinType::Float && exprType == RvalExpressionType::FLOAT) return true;
    if (declType.builtinType ==  BuiltinType::String && exprType == RvalExpressionType::STRING) return true;
    if (declType.builtinType ==  BuiltinType::Bool && exprType == RvalExpressionType::BOOLEAN) return true;
    if (declType.builtinType ==  BuiltinType::Char && exprType == RvalExpressionType::CHAR) return true;
        
    if (declType.builtinType == BuiltinType::Float && exprType == RvalExpressionType::INTEGER) return true;
    if (declType.builtinType == BuiltinType::Int && exprType == RvalExpressionType::FLOAT) return true;
    
    return false;
}

bool 
TypeCompatibility::areTypesCompatible(const Type& declType, const BuiltinType& exprType) {

    // Verificación directa de tipos
    if (declType.builtinType == exprType) return true;
    if (declType.builtinType == BuiltinType::Int && exprType == BuiltinType::Float) return true;
    if (declType.builtinType == BuiltinType::Float && exprType == BuiltinType::Int) return true;
    if (declType.builtinType == BuiltinType::String && exprType == BuiltinType::String) return true;
    if (declType.builtinType == BuiltinType::Bool && exprType == BuiltinType::Bool) return true;
    if (declType.builtinType == BuiltinType::Char && exprType == BuiltinType::Char) return true;

    if (declType.builtinType == BuiltinType::Float && exprType == BuiltinType::Int) return true;
    if (declType.builtinType == BuiltinType::Int && exprType == BuiltinType::Float) return true;
    return false;
}



}