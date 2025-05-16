#pragma once 

#include <string>
#include "../error/ErrorManager.h"

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
            bool lex();
            bool parse();
            bool semanticAnalyze();
            bool generateCode();
            bool generateExecutable();

    };
}