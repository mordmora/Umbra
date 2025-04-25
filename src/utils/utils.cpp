

#include "utils.h"

namespace umbra {


    BuiltinType rvalExpressionTypeToBuiltin(RvalExpressionType type) {
        switch (type) {
            case RvalExpressionType::INTEGER:
                return BuiltinType::Int;
            case RvalExpressionType::FLOAT:
                return BuiltinType::Float;
            case RvalExpressionType::STRING:
                return BuiltinType::String;
            case RvalExpressionType::BOOLEAN:
                return BuiltinType::Bool;
            case RvalExpressionType::CHAR:
                return BuiltinType::Char;
            default:
                return BuiltinType::Undef; // Default case
        }
    }
    


}