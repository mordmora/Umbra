#include "umbra/semantic/SemanticContext.h"

namespace umbra {

    void SemanticContext::enterScope(){
        symbolTable.enterScope();
        std::cout << "Enter in new scope :) - Level " << symbolTable.getCurrentScopeLevel() << std::endl;
    }

    void SemanticContext::exitScope(){
        std::cout << "Exit scope - Level " << symbolTable.getCurrentScopeLevel() << std::endl;
        symbolTable.exitScope();
    }

    int SemanticContext::getCurrentScopeLevel() const {
        return symbolTable.getCurrentScopeLevel();
    }


} //namespace umbra
