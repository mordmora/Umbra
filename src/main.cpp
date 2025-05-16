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

        umbra::Preprocessor preprocessor(argv[1]);
        std::string sourceCode = preprocessor.getProcessedContent();
        umbra::ErrorManager errorManager;
        umbra::Lexer lexer(sourceCode, errorManager);

        auto tokens = lexer.tokenize();

        if(errorManager.hasErrors()) {
            std::cerr << "\nCompilation failed with errors:\n";
            std::cerr << errorManager.getErrorReport();
            return 1;
        }

        umbra::Parser parser(tokens, errorManager);

        try {
            auto ast = parser.parseProgram();
            

            umbra::printAST(ast.get());
            umbra::semanticAnalize(ast.get(), errorManager);

            umbra::code_gen::CodegenContext context("umbra_main_module");
            context.getPrintfFunction();
            umbra::code_gen::CodegenVisitor codegenVisitor(context);
            ast->accept(codegenVisitor);

            llvm::Function* entryPointFunction = context.llvmModule.getFunction("start");
            if(!entryPointFunction){
                llvm::errs() << "Error: Entry point function 'start' not found in module.\n";
                return 1;
            }

            llvm::FunctionType* cMainFuncType = llvm::FunctionType::get(
                llvm::Type::getInt32Ty(context.llvmContext),
                false
            );

            llvm::Function* cMainFunc = llvm::Function::Create(
                cMainFuncType,
                llvm::Function::ExternalLinkage,
                "main",
                &context.llvmModule
            );

            llvm::BasicBlock* cMainEntryBlock = llvm::BasicBlock::Create(
                context.llvmContext,
                "entry",
                cMainFunc
            );

            context.llvmBuilder.SetInsertPoint(cMainEntryBlock);

            llvm::CallInst* callToStart = context.llvmBuilder.CreateCall(
                entryPointFunction
            );

            if (entryPointFunction->getReturnType()->isVoidTy()) {
                // Si 'start' es void, 'main' debe retornar i32 0
                context.llvmBuilder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), 0, true));
            } else if (entryPointFunction->getReturnType()->isIntegerTy(32)) {
                // Si 'start' devuelve i32, 'main' puede retornar ese valor
                context.llvmBuilder.CreateRet(callToStart);
            } else {
                // Si 'start' devuelve otro tipo (ej. float), 'main' aún debe retornar i32.

                llvm::errs() << "Warning: Entry point function 'start' returns a non-integer/non-void type. 'main' will return 0.\n";
                context.llvmBuilder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), 0, true));
            }

            std::string outputFilename = "output.ll";
            std::error_code errorCode;
            llvm::raw_fd_ostream outputStream(outputFilename, errorCode);
            if (errorCode) {
                std::cerr << "Error opening file for writing: " << errorCode.message() << std::endl;
                return 1;
            }
            context.llvmModule.print(outputStream, nullptr);
            outputStream.close();
            std::cout << "Compilation successful!" << std::endl;

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
