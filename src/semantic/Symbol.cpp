
#include "Symbol.h"
#include <llvm/IR/LLVMContext.h>

namespace umbra {
    Symbol::Symbol(std::string name, Symbol::SymbolKind kind,
        std::unique_ptr<Type> type, 
        StringInterner &interner) 
        : 
        name(interner.intern(name)),
        type(std::move(type)), 
        kind(kind) 
    {
        paramTypes = std::vector<std::unique_ptr<Type>>();
    }

    
}