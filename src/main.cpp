//#include "ast/visitor/ASTVisitor.h"
#include "umbra/error/ErrorManager.h"
#include "umbra/lexer/Lexer.h"
#include "umbra/parser/Parser.h"
#include "umbra/preprocessor/Preprocessor.h"
#include "umbra/lexer/Tokens.h"
#include "umbra/semantic/SemanticAnalyzer.h"
#include "umbra/codegen/context/CodegenContext.h"
#include "umbra/codegen/visitors/CodegenVisitor.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <boost/program_options.hpp>
#include <optional>
#include <iostream>
#include <chrono>
#include "umbra/compiler/Compiler.h"

namespace po = boost::program_options;

int main(int argc, char *argv[]) {

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "Show this menu")
        ("input-file", po::value<std::string>(), "Input source file") // Opción para el archivo de entrada
        ("set-target-machine", "Set the target machine code")
        ("show-tokenizer", "Print all tokens")
        ("show-ast", "Print the AST")
        ("show-ir", "Print the LLVM IR")
        ("show-asm", "Print the assembly code")
        ("dump-ir", "Dump the LLVM IR to a file")
        ("dump-asm", "Dump the assembly code to a file")
        ("compile-to-executable", "Compile to an executable");


    po::positional_options_description p;

    p.add("input-file", 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
    po::notify(vm);

    if(vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    umbra::UmbraCompilerOptions options;

    if (vm.count("input-file")) {
        options.inputFilePath = vm["input-file"].as<std::string>();
    } else {
        std::cerr << "Error: No input file specified." << std::endl;
        std::cout << desc << std::endl;
        return 1;
    }

    if(vm.count("show-tokenizer")){
        options.printTokens = true;
    }

    if(vm.count("show-ast")){
        options.printAST = true;
    }

    umbra::ErrorManager errorManager;
    umbra::Compiler compiler(options, errorManager);
    compiler.compile();
    if(errorManager.hasErrors()){
        std::cout << errorManager.getErrorReport();
    }

    return 0;
}
