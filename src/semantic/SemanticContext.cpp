#include "umbra/semantic/SemanticContext.h"

namespace umbra {

    void SemanticContext::enterScope(){
        symbolTable.enterScope();
    }

    void SemanticContext::exitScope(){
        symbolTable.exitScope();
    }

    int SemanticContext::getCurrentScopeLevel() const {
        return symbolTable.getCurrentScopeLevel();
    }


} //namespace umbra
