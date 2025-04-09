//#include "ast/visitor/ASTVisitor.h"
#include "error/ErrorManager.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "ast/ASTVisitor.h"
#include "preprocessor/Preprocessor.h"
#include "lexer/Tokens.h"
#include "semantic/Semantic.h"
#include "semantic/SemanticVisitor.h"
#include<optional>
#include <iostream>

namespace umbra {


void printAST(ASTNode* node) {
    PrintASTVisitor visitor;
    node->accept(visitor);
}

void semanticAnalize(ASTNode* node, ErrorManager& errorManager) {
    StringInterner interner;
    ScopeManager scopeManager;
    ProgramChecker programChecker(interner, scopeManager, errorManager);
    node->accept(programChecker);

}

} // namespace umbra

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <source_file>" << std::endl;
        return 1;
    }
    try {
        umbra::SourceLocation location = {argv[1], 0, 0};
        umbra::File file = {argv[1], location, false, std::nullopt};
        umbra::Preprocessor preprocessor(file);
        std::string sourceCode = preprocessor.out;
        umbra::ErrorManager errorManager;
        umbra::Lexer lexer(sourceCode, errorManager);

        auto tokens = lexer.tokenize();
        if(errorManager.hasErrors()) {
            std::cerr << "\nCompilation failed with errors:\n";
            std::cerr << errorManager.getErrorReport();
            return 1;
        }

        umbra::Parser parser(tokens, errorManager);

        try {
            auto ast = parser.parseProgram();
            

            umbra::printAST(ast.get());
            umbra::semanticAnalize(ast.get(), errorManager);
        
        } catch (const std::exception& e) {
            std::cerr << "Parser error: " << e.what() << std::endl;
            return 1;
        }

        if (errorManager.hasErrors()) {
            std::cerr << "\nCompilation failed with errors:\n";
            std::cerr << errorManager.getErrorReport();
            return 1;
        }
        

        // TODO: Add further stages of compilation here

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
