#ifndef TYPE_NODE_H
#define TYPE_NODE_H

#include <string>
#include "../ASTNode.h"
#include <memory>

namespace umbra {

class TypeNode : public ASTNode {
public:
    enum class BaseType {
        INT,
        FLOAT,
        BOOL,
        CHAR,
        STRING,
        ARRAY
    };

    TypeNode(BaseType baseType, std::unique_ptr<TypeNode> elementType = nullptr)
        : baseType(baseType), elementType(std::move(elementType)) {}

    BaseType getBaseType() const { return baseType; }
    const TypeNode* getElementType() const { return elementType.get(); }


private:
    BaseType baseType;
    std::unique_ptr<TypeNode> elementType;
};

} // namespace umbra

#endif // TYPE_NODE_H