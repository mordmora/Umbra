#ifndef LOOKUP_KEYWORD_H
#define LOOKUP_KEYWORD_H

#include "Tokens.h"
#include <cstdint>

namespace umbra {

    constexpr uint32_t fnv1a_hash(const char* str, std::size_t len) {
        uint32_t hash = 2166136261u;
        for(std::size_t i = 0; i < len; ++i) {
            hash ^= static_cast<uint32_t>(str[i]);
            hash *= 16777619u;
        }
        return hash;
    }

    constexpr std::size_t const_strlen(const char* str) {
        std::size_t len = 0;
        while(str[len] != '\0') { ++len; }
        return len;
    }

    constexpr TokenType lookupKeyword(const char* str, std::size_t len) {
        switch(fnv1a_hash(str, len)) {
            case fnv1a_hash("if", const_strlen("if")):             return TokenType::TOK_IF;
            case fnv1a_hash("else", const_strlen("else")):         return TokenType::TOK_ELSE;
            case fnv1a_hash("elseif", const_strlen("elseif")):     return TokenType::TOK_ELSEIF;
            case fnv1a_hash("repeat", const_strlen("repeat")):     return TokenType::TOK_REPEAT;
            case fnv1a_hash("func", const_strlen("func")):         return TokenType::TOK_FUNC;
            case fnv1a_hash("return", const_strlen("return")):     return TokenType::TOK_RETURN;
            case fnv1a_hash("true", const_strlen("true")):         return TokenType::TOK_TRUE;
            case fnv1a_hash("false", const_strlen("false")):       return TokenType::TOK_FALSE;
            case fnv1a_hash("bool", const_strlen("bool")):         return TokenType::TOK_BOOL;
            case fnv1a_hash("int", const_strlen("int")):           return TokenType::TOK_INT;
            case fnv1a_hash("float", const_strlen("float")):       return TokenType::TOK_FLOAT;
            case fnv1a_hash("string", const_strlen("string")):              return TokenType::TOK_STRING;
            case fnv1a_hash("char", const_strlen("char")):         return TokenType::TOK_CHAR;
            case fnv1a_hash("array", const_strlen("array")):       return TokenType::TOK_ARRAY;
            case fnv1a_hash("void", const_strlen("void")):         return TokenType::TOK_VOID;
            case fnv1a_hash("times", const_strlen("times")):       return TokenType::TOK_TIMES;
            case fnv1a_hash("new", const_strlen("new")):           return TokenType::TOK_NEW;
            case fnv1a_hash("delete", const_strlen("delete")):     return TokenType::TOK_DELETE;
            case fnv1a_hash("and", const_strlen("and")):           return TokenType::TOK_AND;
            case fnv1a_hash("or", const_strlen("or")):             return TokenType::TOK_OR;
            case fnv1a_hash("equal", const_strlen("equal")):       return TokenType::TOK_EQUAL;
            case fnv1a_hash("not", const_strlen("not")):           return TokenType::TOK_NOT;
            case fnv1a_hash("less_than", const_strlen("less_than")): return TokenType::TOK_LESS;
            case fnv1a_hash("greater_than", const_strlen("greater_than")): return TokenType::TOK_GREATER;
            case fnv1a_hash("less_or_equal", const_strlen("less_or_equal")): return TokenType::TOK_LESS_EQ;
            case fnv1a_hash("greater_or_equal", const_strlen("greater_or_equal")): return TokenType::TOK_GREATER_EQ;
            case fnv1a_hash("different", const_strlen("different")): return TokenType::TOK_DIFFERENT;

            case fnv1a_hash("ptr", const_strlen("ptr")):           return TokenType::TOK_PTR;
            case fnv1a_hash("ref", const_strlen("ref")):           return TokenType::TOK_REF;
            case fnv1a_hash("access", const_strlen("access")):     return TokenType::TOK_ACCESS;
            default:                                                                      return TokenType::TOK_IDENTIFIER;
        }
    }

} // namespace umbra

#endif
