#pragma once

#include"umbra/ast/Visitor.h"
#include"umbra/ast/Nodes.h"
#include<memory>
#include"umbra/semantic/SemanticType.h"

/*
    TypeCk es un visitante sencillo que se encarga de inferir y calcular los tipos de
    las expresiones de lado derecho, por ejemplo: int a = 10 + b
    para esa expresion, este mecanismo debe comprobar primero si b es de tipo entero, y si la
    operacion entre 10 + b es compatible, adicionalmente, comprueba si a y el resultado
    de la operacion derecha tambien es compatible.
*/

namespace umbra {

    class TypeCk : public BaseV<std::unique_ptr, TypeCk, SemanticType> {
    public:

        SemanticType getRvalExprType();

        SemanticType visitBinaryExpression(BinaryExpression* node);

        SemanticType visitNumericLiteral(NumericLiteral* node);

        SemanticType visitStringLiteral(StringLiteral* node);

    };


}
