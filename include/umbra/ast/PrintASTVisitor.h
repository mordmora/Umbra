

#include "umbra/ast/Visitor.h"
#include "umbra/ast/Nodes.h"
#include <memory>
namespace umbra {

    class Printer : BaseV<std::unique_ptr, Printer, void> {
        public:

            void visitProgramNode(ProgramNode& node){
                std::cout << static_cast<int>(node.getKind()) << std::endl;
            }
    };



}
