//#include "ast/visitor/ASTVisitor.h"
#include "error/ErrorManager.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "preprocessor/Preprocessor.h"
#include "lexer/Tokens.h"
#include<optional>
#include <iostream>

namespace umbra {


//void printAST(ASTNode *node) {
 //   PrintVisitor visitor;
//    node->accept(visitor);
//}

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
        std::cout << sourceCode;
        umbra::ErrorManager errorManager;
        umbra::Lexer lexer(sourceCode, errorManager);

        auto tokens = lexer.tokenize();
        #define DEBUG
        #ifdef DEBUG
        std::cout << "Tokens:" << std::endl;
        for (const auto &token : tokens) {
            std::cout << "Token: "<< lexer.tokenManager.tokenTypeToString(token.type) << " " << (token.lexeme == "\n" ? "New Line" : token.lexeme) << std::endl;
        }
        #endif

        umbra::Parser parser(tokens, errorManager);

        try {
            auto ast = parser.parseProgram();
            
            if (errorManager.hasErrors()) {
                std::cerr << "\nCompilation failed with errors:\n";
                std::cerr << errorManager.getErrorReport();
                return 1;
            }
            
            std::cout << "\nParsing completed successfully.\n";
            
            // Descomenta esto cuando implementes el PrintVisitor
            // std::cout << "Abstract Syntax Tree:" << std::endl;
            // umbra::printAST(ast.get());
            
        } catch (const std::exception& e) {
            std::cerr << "Parser error: " << e.what() << std::endl;
            return 1;
        }

        // TODO: Add further stages of compilation here

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
