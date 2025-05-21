//#include "ast/visitor/ASTVisitor.h"
#include "error/ErrorManager.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "ast/ASTVisitor.h"
#include "preprocessor/Preprocessor.h"
#include "lexer/Tokens.h"
#include "semantic/SemanticVisitor.h"
#include "codegen/context/CodegenContext.h"
#include "codegen/visitors/CodegenVisitor.h"
#include "semantic/SemanticVisitor.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <boost/program_options.hpp>
#include<optional>
#include <iostream>
#include <chrono>
#include "compiler/Compiler.h"

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

    // ... procesa otras opciones de manera similar ...
    
    umbra::ErrorManager errorManager;
    umbra::Compiler compiler(options, errorManager);
    compiler.compile();

    return 0;
}
