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
    ("help", "Show this menu")
    ("set-target-machine", "Set the target machine code")
    ("show-tokenizer", "Print all tokens")
    ("show-ast", "Print the AST")
    ("show-ir", "Print the LLVM IR")
    ("show-asm", "Print the assembly code")
    ("dump-ir", "Dump the LLVM IR to a file")
    ("dump-asm", "Dump the assembly code to a file")
    ("compile-to-executable", "Compile to an executable");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    umbra::UmbraCompilerOptions options;
    options.inputFilePath = argv[1];
    
    umbra::ErrorManager errorManager;
    umbra::Compiler compiler(options, errorManager);
    compiler.compile();

    return 0;
}
