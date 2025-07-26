#include "umbra/semantic/SemanticContext.h"

namespace umbra {

    void SemanticContext::enterScope(){
        symbolTable.enterScrope();
        std::cout << "Enter in new scope :)" << std::endl;
    }


} //namespace umbra
