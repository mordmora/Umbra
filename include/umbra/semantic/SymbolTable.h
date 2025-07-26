#pragma once

#include<optional>
#include<vector>
#include"umbra/semantic/SemanticType.h"
#include<string>
#include<unordered_map>

/*
    En este archivo se encuentran las clases relacionalas a la tabla de simbolos.
    Se usa una estructura con informacion semantica rica para realizar un analisis mas agresivo
    y reducir la complejidad a la hora de generar codigo.
*/

namespace umbra{



    enum class SymbolKind {
        VARIABLE,
        FUCNTION
    };

    struct FunctionSignature {
        SemanticType returnType;
        std::vector<SemanticType> argTypes;
    };

    struct Symbol{
        SemanticType type;
        SymbolKind kind;
        FunctionSignature signature;
        int line;
        int col;
    };

    class SymbolTable{

    public:
        // Constructor que inicializa con scope global
        SymbolTable();

        void enterScrope();
        void exitScope();
        void insert(const std::string& name, Symbol);
        Symbol lookup(const std::string& name) const;
    private:
        std::vector<std::unordered_map<std::string, Symbol>> scopes;
    };

}
