#include "Compiler.h"
#include "../preprocessor/Preprocessor.h"
#include "../error/CompilerError.h"
#include "../semantic/SemanticVisitor.h"
#include "../semantic/ScopeManager.h"
#include "../semantic/Symbol.h"
#include "../semantic/TypeCompatibility.h"
#include "../semantic/StringInterner.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include "../codegen/visitors/CodegenVisitor.h"
#include "../codegen/context/CodegenContext.h"
#include "../utils/utils.h"

#include <memory>

namespace umbra {

    Compiler::Compiler(UmbraCompilerOptions opt) : options(opt){}

    Compiler::Compiler(UmbraCompilerOptions opt, ErrorManager& errorManager) : options(opt), errorManager(errorManager) {}

    bool Compiler::preprocess(std::string& src) {

        try{
            Preprocessor preprocessor(options.inputFilePath);
            src = preprocessor.getProcessedContent();
            return true;
        } catch (const std::exception& e) {
            errorManager.addError(std::make_unique<CompilerError>(
                ErrorType::PREPROCESSOR,
                "Error during preprocessing: " + std::string(e.what()),
                0,
                0
            ));
            return false;
        }
    }

    std::vector<Lexer::Token> Compiler::lex(std::string& src){
        std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(src, errorManager);
        auto tokens = lexer->tokenize();
        if (errorManager.hasErrors()) {
            return {};
        }
        return tokens;
    }

    std::unique_ptr<ProgramNode> Compiler::parse(std::vector<Lexer::Token>& tokens){
        std::unique_ptr<Parser> parser = std::make_unique<Parser>(tokens, errorManager);
        auto programNode = parser->parseProgram();
        if (errorManager.hasErrors()) {
            return nullptr;
        }
        return programNode;
    }

    bool Compiler::semanticAnalyze(ProgramNode& programNode){
        StringInterner interner;
        ScopeManager scopeManager;
        ProgramChecker programChecker(interner, scopeManager, errorManager);
        programNode.accept(programChecker);
        if (errorManager.hasErrors()) {
            return false;
        }
        return true;
    }

    bool Compiler::generateCode(ProgramNode& programNode, std::string& moduleName){
        umbra::code_gen::CodegenContext codegenContext(moduleName);
        codegenContext.getPrintfFunction();
        umbra::code_gen::CodegenVisitor codegenVisitor(codegenContext);
        programNode.accept(codegenVisitor);
        if (errorManager.hasErrors()) {
            return false;
        }

        llvm::Function* entryPointFunction = codegenContext.llvmModule.getFunction("start");

        if(entryPointFunction == nullptr){
            llvm::errs() << "Error: Entry point function 'start' not found in module.\n";
            return false;
        }

        llvm::FunctionType* cMainFuncType = llvm::FunctionType::get(
            llvm::Type::getInt32Ty(codegenContext.llvmContext),
            false
        );

        llvm::Function* cMainFunction = llvm::Function::Create(
            cMainFuncType,
            llvm::Function::ExternalLinkage,
            "main",
            codegenContext.llvmModule
        );

        llvm::BasicBlock* cMainEntryBlock = llvm::BasicBlock::Create(codegenContext.llvmContext, "entry", cMainFunction);

        codegenContext.llvmBuilder.SetInsertPoint(cMainEntryBlock);

        llvm::CallInst* callToStart = codegenContext.llvmBuilder.CreateCall(entryPointFunction);

        if (entryPointFunction->getReturnType()->isVoidTy()) {
            codegenContext.llvmBuilder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(codegenContext.llvmContext), 0, true));
        } else if (entryPointFunction->getReturnType()->isIntegerTy(32)) {

            codegenContext.llvmBuilder.CreateRet(callToStart);
        } else {

            llvm::errs() << "Warning: Entry point function 'start' returns a non-integer/non-void type. 'main' will return 0.\n";
            codegenContext.llvmBuilder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(codegenContext.llvmContext), 0, true));
        }
        return true;
    }

    
    void Compiler::generateIRFile(llvm::Module& module, const std::string& filename){
        std::error_code errorCode;
        llvm::raw_fd_ostream outputStream(filename, errorCode);
        if (errorCode) {
            std::cerr << "Error opening file for writing: " << errorCode.message() << std::endl;
            return;
        }
        module.print(outputStream, nullptr);
        outputStream.close();
    }

    bool Compiler::compile(){
        std::string src;
        if (!preprocess(src)){
            return false;
        }
        auto tokens = lex(src);
        if (tokens.empty()) {
            return false;
        }
        auto root = parse(tokens);
        if (!root) {
            return false;
        }
        if (!semanticAnalyze(*root)) {
            return false;
        }
        if (!generateCode(*root, options.outputIRFile)) {
            return false;
        }
        std::cout << "Compilation successful!" << std::endl;
    }
}