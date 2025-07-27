#pragma once

namespace umbra {
enum class SemanticType {
    #define X(T) T,
    #include "umbra/utils/builtin_types.def"
    #undef X
};

}
