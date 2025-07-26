#include "umbra/semantic/TypeCk.h"
#include "umbra/semantic/SemanticType.h"
#include "umbra/ast/Nodes.h"
#include "umbra/semantic/SemanticAnalyzer.h"
namespace umbra{



    SemanticType TypeCk::visitNumericLiteral(NumericLiteral* node){
        return convertBuiltinToSemanticType(node->builtinType);
    }

    SemanticType TypeCk::visitBinaryExpression(BinaryExpression* node){
        auto lType = visit(node->left.get());
        auto rType = visit(node->right.get());
        // Simple type check: require both sides equal
        if (lType == rType) {
            return lType;
        }
        // Mismatch yields error
        return SemanticType::ERROR;
    }

    // Handle string literals
    SemanticType TypeCk::visitStringLiteral(StringLiteral* node) {

        std::cout << "Getting string type" << std::endl;

        return SemanticType::STRING;
    }

}
