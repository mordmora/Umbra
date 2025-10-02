#pragma once
#include "umbra/ast/Types.h"
#include "umbra/semantic/SemanticType.h"
#include <string>

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

    inline std::string semanticTypeToString(SemanticType sType){
        switch(sType){
            case SemanticType::None: return "None";
            case SemanticType::Int: return "Int";
            case SemanticType::Float: return "Float";
            case SemanticType::String: return "String";
            case SemanticType::Char: return "Char";
            case SemanticType::Double: return "Double";
            case SemanticType::Bool: return "Bool";
            case SemanticType::Array: return "Array";
            case SemanticType::Null: return "Null";
            case SemanticType::Void: return "Void";
            case SemanticType::Undef: return "Undef";
            case SemanticType::UserDefType: return "UserDefType";
            case SemanticType::Error: return "Error";
            default: return "Unknown";
        }
    }

    inline std::string getPrintArgsFormat(std::string& str){

        //Algoritmo aun en construcción, no tocar por favor :)

        std::string output = "";
        std::string currFormat = "";
        bool normalMode = true;
        size_t l = str.length();
        for(size_t i = 0; i < l; i++){
            if(normalMode){
                if(str[i] == '{'){
                    normalMode = false;
                    currFormat+=str[i];
                    continue;
                }
                output+=str[i];
            }else{ // $1}
                if(currFormat.length() < 4){
                    currFormat+=str[i];
                }else{

                }
            }
        }
        return output; // devolver lo acumulado por ahora
    }

}
