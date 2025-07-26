#include "umbra/ast/Nodes.h"
#include "umbra/semantic/SymbolCollector.h"
#include "umbra/ast/Visitor.h"
#include "umbra/semantic/SemanticAnalyzer.h"

namespace umbra {

    void SymbolCollector::visitProgramNode(ProgramNode* node) {
        std::cout << "SymbolCollector: visitProgramNode called, functions count = " << node->functions.size() << std::endl;
        for(auto &F : node->functions){
            std::cout << "SymbolCollector: visiting function " << F->name->name << std::endl;
            visit(F.get());
        }
    }

    void SymbolCollector::visitFunctionDefinition(FunctionDefinition* node) {

        std::cout << "Entry in func def" << std::endl;

        SemanticType returnType = convertBuiltinToSemanticType(node->returnType->builtinType);

        FunctionSignature signature{
            .returnType = returnType,
            .argTypes = {}
        };

        if (node->parameters) {
            for(auto& param : node->parameters->parameters) {
                SemanticType paramType = convertBuiltinToSemanticType(param.first->builtinType);
                signature.argTypes.push_back(paramType);
            }
        }

        theContext.enterScope();

        for(auto& S: node->body){
            visit(S.get());
        }

        Symbol functionSymbol{
            .type = returnType,
            .kind = SymbolKind::FUCNTION,
            .signature = signature,
            .line = 0,  // TODO: obtener línea real
            .col = 0    // TODO: obtener columna real
        };

        symTable.insert(node->name->name, functionSymbol);
    }

    void SymbolCollector::visitVariableDeclaration(VariableDeclaration* node){
        Symbol varSymb {
            .type=convertBuiltinToSemanticType(node->type->builtinType),
            .kind=SymbolKind::VARIABLE,
            .signature={},
            .line=0,
            .col=0
        };

        if(node->initializer != nullptr){
            std::cout << "Var " << node->name->name << "has initializer" << std::endl;
            auto rvalType = typeCk.visit(node->initializer.get());

        }

        symTable.insert(node->name->name, varSymb);
    }

};
