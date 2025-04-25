#pragma once

#include "../ast/Nodes.h"
#include "RvalExpressionType.h"

namespace umbra{

/**
 * Clase estática que verifica la compatibilidad entre tipos
 */
class TypeCompatibility {
    public:
        // Verifica si el tipo de expresión es compatible con el tipo declarado
        static bool areTypesCompatible(const Type& declType, RvalExpressionType exprType);
        static bool areTypesCompatible(const Type& declType, const BuiltinType& exprType);
    };
    

}