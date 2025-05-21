#pragma once 

#include <string>
#include <memory>
#include "../error/ErrorManager.h"
#include "../lexer/Lexer.h"
#include "../parser/Parser.h"
#include "../ast/ASTNode.h"
#include "../ast/Nodes.h"
#include <llvm/IR/Module.h>
namespace umbra {

    typedef struct UmbraCompilerOptions {
        std::string inputFilePath;
        std::string outputIRFile = "umbra_ir.ll";
        std::string outputExecName = "umbra_output";
        bool compileToExecutable = true;
        bool showASMCode = false;
        bool showIRCode = false;
        bool printAST = false;
        bool traceParse = false;
        bool traceLex = false;
        bool printTokens = false;
        bool printGrammarTrace = false;
    } UmbraCompilerOptions;

    class Compiler {
        public:
            explicit Compiler(UmbraCompilerOptions opt); // Usará ErrorManager interno
            Compiler(UmbraCompilerOptions opt, ErrorManager& externalErrorManager); // Usará ErrorManager externo
            bool compile();

        private:
            UmbraCompilerOptions options;
            std::unique_ptr<ErrorManager> internalErrorManager_; // Solo se usa si no se proporciona uno externo
            ErrorManager& errorManagerRef_; // Siempre referencia a un ErrorManager válido

            void printTokens(const std::vector<Lexer::Token>& tokens);
            bool preprocess(std::string& src);
            std::vector<Lexer::Token> lex(std::string& src);
            std::unique_ptr<ProgramNode> parse(std::vector<Lexer::Token>& tokens);
            bool semanticAnalyze(ProgramNode& programNode);
            bool generateCode(ProgramNode& programNode, std::string& moduleName);
            void generateIRFile(llvm::Module& module, const std::string& filename);
            bool generateExecutable(const std::string& irFilename, const std::string& outputName);

    };
}