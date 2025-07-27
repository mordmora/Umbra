#include "umbra/semantic/SemanticAnalyzer.h"

#include <utility>
#include <unordered_set>
#include <iostream>

namespace umbra {

    void SemanticAnalyzer::execAnalysisPipeline(){
        collector.visitProgramNode(rootASTNode);
    }



}; // namespace umbra
