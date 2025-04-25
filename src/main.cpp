//#include "ast/visitor/ASTVisitor.h"
#include "error/ErrorManager.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "ast/ASTVisitor.h"
#include "preprocessor/Preprocessor.h"
#include "lexer/Tokens.h"
#include "semantic/SemanticVisitor.h"
#include "semantic/SemanticVisitor.h"
#include<optional>
#include <iostream>
#include <chrono>

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


        auto start = std::chrono::high_resolution_clock::now();
        auto tokens = lexer.tokenize();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "Tokenization took: " << elapsed.count() << " seconds" << std::endl;
        if(errorManager.hasErrors()) {
            std::cerr << "\nCompilation failed with errors:\n";
            std::cerr << errorManager.getErrorReport();
            return 1;
        }


      //  for(auto tok : tokens){
      //      std::cout << "Token: " << static_cast<int>(tok.type) << " - " << tok.lexeme() << std::endl;
       // }

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
