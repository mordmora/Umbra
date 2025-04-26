
#include "CodegenContext.h"

namespace umbra { 
    namespace code_gen {
    CodegenContext::CodegenContext(const std::string& moduleName)
        : llvmContext(),
        llvmModule(moduleName, llvmContext),
        llvmBuilder(llvmContext) {}
    }
} //namespace umbra::code_gen