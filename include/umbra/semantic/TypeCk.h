#pragma once

#include"umbra/ast/Visitor.h"
#include"umbra/ast/Nodes.h"
#include<memory>
#include"umbra/semantic/SemanticType.h"

namespace umbra {

    class TypeCk : public BaseV<std::unique_ptr, TypeCk, SemanticType> {
    public:

        SemanticType getRvalExprType();

        SemanticType visitBinaryExpression(BinaryExpression* node);

        SemanticType visitNumericLiteral(NumericLiteral* node);

        SemanticType visitStringLiteral(StringLiteral* node);

    };


}
