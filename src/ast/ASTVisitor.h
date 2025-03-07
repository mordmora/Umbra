#include "Nodes.h"
namespace umbra{
    class ASTVisitor{
        public: 
        virtual ~ASTVisitor() = default;
        virtual void visit(ProgramNode& node) = 0;
        virtual void visit(FunctionDefinition& node) = 0;
        virtual void visit(ParameterList& node) = 0;
        virtual void visit(Type& node) = 0;
        virtual void visit(Identifier& node) = 0;
        virtual void visit(Expression& node) = 0;
        virtual void visit(VariableDeclaration& node) = 0;
        virtual void visit(FunctionCall& node) = 0;
        virtual void visit(AssignmentStatement& node) = 0;
        virtual void visit(Literal& node) = 0;
        virtual void visit(ExpressionStatement& node) = 0;
        virtual void visit(BinaryExpression& node) = 0;
        virtual void visit(PrimaryExpression& node) = 0;
    };

    class PrintASTVisitor : public ASTVisitor {
        public:
        PrintASTVisitor();
        
        void visit(ProgramNode &node) override;
        void visit(FunctionDefinition& node) override;
        void visit(ParameterList& node) override;
        void visit(Type& node) override;
        void visit(Identifier& node) override;
        void visit(Expression& node) override;
        void visit(VariableDeclaration& node) override;
        void visit(AssignmentStatement& node) override;
        void visit(FunctionCall& node) override;
        void visit(Literal& node) override;  
        void visit(ExpressionStatement& node) override; 
        void visit(BinaryExpression& node) override;
        void visit(PrimaryExpression& node) override;
        private:
        void indent();
        int deep = 0;
    };
}