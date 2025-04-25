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
        
    // Conversiones implícitas permitidas
    if (declType.builtinType == BuiltinType::Float && exprType == RvalExpressionType::INTEGER) return true;
    
    return false;
}

}