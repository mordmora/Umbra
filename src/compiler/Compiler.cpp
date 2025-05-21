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

    Compiler::Compiler(UmbraCompilerOptions opt)
        : options(std::move(opt)), // Mueve opt si es apropiado
          internalErrorManager_(std::make_unique<ErrorManager>()),
          errorManagerRef_(*internalErrorManager_) {
        // Ahora errorManagerRef_ apunta al ErrorManager interno recién creado
    }

    Compiler::Compiler(UmbraCompilerOptions opt, ErrorManager& externalErrorManager)
        : options(std::move(opt)), // Mueve opt si es apropiado
          // internalErrorManager_ permanece nullptr o sin inicializar (no se usa)
          errorManagerRef_(externalErrorManager) {
        // Ahora errorManagerRef_ apunta al ErrorManager externo
    }

    bool Compiler::preprocess(std::string& src) {

        try{
            Preprocessor preprocessor(options.inputFilePath);
            src = preprocessor.getProcessedContent();
            return true;
        } catch (const std::exception& e) {
            errorManagerRef_.addError(std::make_unique<CompilerError>( // Usa la referencia
                ErrorType::PREPROCESSOR,
                "Error during preprocessing: " + std::string(e.what()),
                0,
                0
            ));
            return false;
        }
    }

    void Compiler::printTokens(const std::vector<Lexer::Token>& tokens) {
        for (const auto& token : tokens) {
            std::cout << "Token << " << token.lexeme << " >> "
                      << "Type: " << static_cast<int>(token.type) << " "
                      << "Line: " << token.line << " "
                      << "Column: " << token.column << std::endl;
        }
    }

    std::vector<Lexer::Token> Compiler::lex(std::string& src){
        std::unique_ptr<Lexer> lexer = std::make_unique<Lexer>(src, errorManagerRef_); // Usa la referencia
        auto tokens = lexer->tokenize();

        if(options.printTokens) {
            printTokens(tokens);
        }

        if (errorManagerRef_.hasErrors()) { // Usa la referencia
            return {};
        }
        return tokens;
    }

    std::unique_ptr<ProgramNode> Compiler::parse(std::vector<Lexer::Token>& tokens){
        std::unique_ptr<Parser> parser = std::make_unique<Parser>(tokens, errorManagerRef_); // Usa la referencia
        auto programNode = parser->parseProgram();
        if (errorManagerRef_.hasErrors()) { // Usa la referencia
            return nullptr;
        }
        return programNode;
    }

    bool Compiler::semanticAnalyze(ProgramNode& programNode){
        StringInterner interner;
        ScopeManager scopeManager;
        ProgramChecker programChecker(interner, scopeManager, errorManagerRef_); // Usa la referencia
        programNode.accept(programChecker);
        if (errorManagerRef_.hasErrors()) { // Usa la referencia
            return false;
        }
        return true;
    }

    bool Compiler::generateCode(ProgramNode& programNode, std::string& moduleName){
        umbra::code_gen::CodegenContext codegenContext(moduleName);
        codegenContext.getPrintfFunction();
        umbra::code_gen::CodegenVisitor codegenVisitor(codegenContext); // Pasa la referencia
        programNode.accept(codegenVisitor);
        if (errorManagerRef_.hasErrors()) { // Usa la referencia
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
        generateIRFile(codegenContext.llvmModule, options.outputIRFile);
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

    bool Compiler::generateExecutable(const std::string& irFilename, const std::string& outputName){
        std::string command = "llc -filetype=obj " + irFilename + " -o " + outputName + ".o";
        int result = system(command.c_str());
        if (result != 0) {
            std::cerr << "Error generating object file." << std::endl;
            return false;
        }

        command = "gcc " + outputName + ".o -no-pie -o " + outputName;
        result = system(command.c_str());
        if (result != 0) {
            std::cerr << "Error generating executable." << std::endl;
            return false;
        }
        return true;
    }

    bool Compiler::compile(){
        std::string src;
        if (!preprocess(src)){
            // errorManagerRef_ ya tiene el error
            return false;
        }
        auto tokens = lex(src);
        if (errorManagerRef_.hasErrors()) {
            return false;
        }

        if (tokens.empty() || (tokens.back().type != TokenType::TOK_EOF && !tokens.empty()) ) {
            // errorManagerRef_.addError(std::make_unique<CompilerError>(ErrorType::LEXER, "Lexical analysis produced no tokens or missing EOF.",0,0));
            return false;
        }

        auto root = parse(tokens);
        if (errorManagerRef_.hasErrors() || !root) {
            return false;
        }

        if (!semanticAnalyze(*root)) {
            return false;
        }

        std::string moduleName = "umbra_module";
        if (!generateCode(*root, moduleName)) {
            return false;
        }

        generateExecutable(options.outputIRFile, options.outputExecName);

        std::cout << "Compilation successful!" << std::endl;
        return true;
    }

} // namespace umbra