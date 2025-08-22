#pragma once

/**
 * @file SemanticType.h
 * @brief Enumeración de tipos semánticos usados durante el análisis.
 * @details
 * Los valores se generan expandiendo la lista en `umbra/utils/builtin_types.def`.
 * Representan los tipos que maneja el pipeline semántico (variables, retornos, expresiones).
 */
namespace umbra {

/**
 * @enum SemanticType
 * @brief Tipos semánticos canónicos del compilador.
 * @note La lista proviene de builtin_types.def y debe estar sincronizada con BuiltinType.
 */
enum class SemanticType {
    #define X(T) T,
    #include "umbra/utils/builtin_types.def"
    #undef X
};

}
