#include "Compiler.h"
#include "../preprocessor/Preprocessor.h"

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



}