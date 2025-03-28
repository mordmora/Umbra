#include "SemanticVisitor.h"
#include "../error/CompilerError.h"

namespace umbra{

    SemanticVisitor::SemanticVisitor(StringInterner& interner, ScopeManager& scopeManager) 
    : 
    interner(interner), 
    symbolTable(interner),
    scopeManager(scopeManager) {}

    void 
    ProgramChecker::visit(ProgramNode& node){
        for(auto& child : node.functions){
            child->accept(*this);
        }
    }

    void 
    ProgramChecker::visit(ParameterList& node){
        for(auto& param : node.parameters){
            bool status = symbolTable.addSymbol(std::make_unique<Symbol>(param.second->name,
             Symbol::SymbolKind::VARIABLE, 
             std::move(param.first), 
             interner));
             if (!status){
                 throw SemanticError("Parameter " 
                    + param.second->name 
                    + " already declared", 
                    0, 
                    0, 
                    SemanticError::Action::ERROR);
            }
        }
    }

    void 
    ProgramChecker::visit(FunctionDefinition& node){

        symbolTable.addSymbol(std::make_unique<Symbol>(node.name->name,
             Symbol::SymbolKind::FUNCTION, 
             std::move(node.returnType), 
             interner));

        node.parameters->accept(*this);

        scopeManager.enterScope(_SymbolMap());

        for(auto& stmt : node.body){
            stmt->accept(*this);
        }

        scopeManager.exitScope();
    }

    void 
    ProgramChecker::visit(Statement& node){
        // Do nothing
    }

    void
    ProgramChecker::visit(VariableDeclaration& node){

        if(node.initializer != nullptr){
            node.initializer.;
        }

    }
}