#include "ast/visitor/ASTVisitor.h"
#include "error/ErrorManager.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

namespace umbra {

std::string readFile(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

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
        std::string sourceCode = umbra::readFile(argv[1]);
        umbra::ErrorManager errorManager;
        umbra::Lexer lexer(sourceCode, errorManager);

        // Perform lexical analysis
        auto tokens = lexer.tokenize();

        // Print tokens (for debugging)
        std::cout << "Tokens:" << std::endl;
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