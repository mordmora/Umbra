#include "ast/visitor/ASTVisitor.h"
#include "error/ErrorManager.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "preprocessor/Preprocessor.h"
#include <fstream>
#include<optional>
#include <iostream>
#include <memory>
#include <sstream>


namespace umbra {


void printAST(ASTNode *node) {
    PrintVisitor visitor;
    node->accept(visitor);
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

        // Perform lexical analysis
        auto tokens = lexer.tokenize();

        // Print tokens (for debugging)
        std::cout << "Tokens:" << std::endl;
        #define RAW_DEBUG
        #if defined (RAW_DEBUG)
            for(const auto &token : tokens){
                std::cout << token.lexeme;
            }
        #else
        for (const auto &token : tokens) {
            std::cout << "Token: Type=" << lexer.tokenManager.tokenTypeToString(token.type)
                      << ", Lexeme='";
            if (token.type == umbra::TokenType::TOK_NEWLINE) {
                std::cout << "\\n";
            } else {
                std::cout << token.lexeme;
            }
            std::cout << "', Line=" << token.line << ", Column=" << token.column << std::endl;
        }
        #endif

        std::cout << "Lexical analysis completed" << std::endl;

        // Perform parsing
        umbra::Parser parser(tokens, errorManager);
        std::unique_ptr<umbra::ASTNode> ast;

        try {
            ast = parser.parse();
            std::cout << "Parsing completed." << std::endl;

            if (errorManager.hasErrors()) {
                std::cerr << "Compilation failed. Errors:\n";
                std::cerr << errorManager.getErrorReport();
            } else {
                std::cerr << "Compilation successfully.\n";
            }

            // Print AST using our new PrintVisitor
            std::cout << "Abstract Syntax Tree:" << std::endl;
            umbra::printAST(ast.get());
        } catch (const std::exception &e) {
            std::cerr << "Parsing failed. Error: " << e.what() << std::endl;
            return 1;
        }

        // TODO: Add further stages of compilation here

    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
