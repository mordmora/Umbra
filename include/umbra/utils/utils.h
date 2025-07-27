#pragma once
#include "umbra/ast/Types.h"
#include "umbra/semantic/SemanticType.h"
namespace umbra {

    inline constexpr BuiltinType semaTypeToBuiltinType(SemanticType sType){

        switch (sType)
        {
            #define X(T) case SemanticType::T: return BuiltinType::T;
            #include "umbra/utils/builtin_types.def"
            #undef X
            default: return BuiltinType::Error;
        }
    }

    inline constexpr SemanticType builtinTypeToSemaType(BuiltinType bType){
        switch(bType){
            #define X(T) case BuiltinType::T: return SemanticType::T;
            #include "umbra/utils/builtin_types.def"
            #undef X
            default: return SemanticType::Error;
        }
    }

}
