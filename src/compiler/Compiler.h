#pragma once 

#include <string>
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
            Compiler(UmbraCompilerOptions opt);
            Compiler(UmbraCompilerOptions opt, ErrorManager& errorManager);
            bool compile();

        private:
            ErrorManager errorManager;
            UmbraCompilerOptions options;

            bool preprocess(std::string& src);
            std::vector<Lexer::Token> lex(std::string& src);
            std::unique_ptr<ProgramNode> parse(std::vector<Lexer::Token>& tokens);
            bool semanticAnalyze(ProgramNode& programNode);
            bool generateCode(ProgramNode& programNode, std::string& moduleName);
            void generateIRFile(llvm::Module& module, const std::string& filename);
            //bool generateExecutable();

    };
}