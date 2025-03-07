#include"ASTVisitor.h"
#include <iostream>
#include <algorithm>

namespace umbra {
    
    PrintASTVisitor::PrintASTVisitor() : deep(0) {}
    
    // Función auxiliar para manejar la indentación
    void PrintASTVisitor::indent() {
        for (int i = 0; i < deep; ++i) {
            std::cout << "  ";
        }
    }

    void PrintASTVisitor::visit(ProgramNode& node) {
        indent();
        std::cout << "Program:" << std::endl;
        
        deep++;
        for (const auto& function : node.functions) {
            function->accept(*this);
        }
        deep--;
    }

    void PrintASTVisitor::visit(FunctionDefinition& node) {
        indent();
        std::cout << "Function Definition:" << std::endl;
        
        deep++;
        
        indent();
        std::cout << "Name: ";
        node.name->accept(*this);
        
        indent();
        std::cout << "Parameters:" << std::endl;
        deep++;
        node.parameters->accept(*this);
        deep--;
        
        indent();
        std::cout << "Return Type: ";
        node.returnType->accept(*this);
        
        indent();
        std::cout << "Body:" << std::endl;
        deep++;
        for (const auto& statement : node.body) {
            statement->accept(*this);
        }
        deep--;
        
        deep--;
    }

    void PrintASTVisitor::visit(ParameterList& node) {
        if (node.parameters.empty()) {
            indent();
            std::cout << "(no parameters)" << std::endl;
            return;
        }
        
        for (const auto& param : node.parameters) {
            indent();
            std::cout << "Parameter:" << std::endl;
            deep++;
            indent();
            std::cout << "Type: ";
            param.first->accept(*this);
            indent();
            std::cout << "Name: ";
            param.second->accept(*this);
            deep--;
        }
    }

    void PrintASTVisitor::visit(Type& node) {
        std::cout << node.baseType;
        for (int i = 0; i < node.arrayDimensions; ++i) {
            std::cout << "[]";
        }
        std::cout << std::endl;
    }

    void PrintASTVisitor::visit(Identifier& node) {
        std::cout << node.name << std::endl;
    }

    void PrintASTVisitor::visit(VariableDeclaration& node) {
        indent();
        std::cout << "Variable Declaration:" << std::endl;
        
        deep++;
        
        indent();
        std::cout << "Type: ";
        node.type->accept(*this);
        
        indent();
        std::cout << "Name: ";
        node.name->accept(*this);
        
        if (node.initializer) {
            indent();
            std::cout << "Initializer:" << std::endl;
            deep++;
            node.initializer->accept(*this);
            deep--;
        }
        
        deep--;
    }

    void PrintASTVisitor::visit(AssignmentStatement& node) {
        indent();
        std::cout << "Assignment:" << std::endl;
        
        deep++;
        
        indent();
        std::cout << "Target:" << std::endl;
        deep++;
        node.target->accept(*this);
        deep--;
        
        indent();
        std::cout << "Value:" << std::endl;
        deep++;
        node.value->accept(*this);
        deep--;
        
        deep--;
    }

    void PrintASTVisitor::visit(FunctionCall& node){
        indent();
        std::cout << "Function Call:" << std::endl;
        deep++;
        
        indent();
        std::cout << "Function Name: ";
        node.functionName->accept(*this);
        
        if (!node.arguments.empty()) {
            indent();
            std::cout << "Arguments:" << std::endl;
            deep++;
            for (const auto& arg : node.arguments) {
                indent();
                std::cout << "Argument:" << std::endl;
                deep++;
                arg->accept(*this);
                deep--;
            }
            deep--;
        } else {
            indent();
            std::cout << "No Arguments" << std::endl;
        }
        
        deep--;
    }

    void PrintASTVisitor::visit(Expression& node) {
        indent();
        std::cout << "Expression (base)" << std::endl;
    }

    void PrintASTVisitor::visit(Literal& node) {
        indent();
        std::cout << "Literal: " << node.value << std::endl;
    }

    void PrintASTVisitor::visit(ExpressionStatement& node) {
        indent();
        std::cout << "Expression Statement:" << std::endl;
        deep++;
        node.exp->accept(*this);
        deep--;
    }

    void PrintASTVisitor::visit(BinaryExpression& node) {
        indent();
        std::cout << "Binary Expression: operator '" << node.op << "'" << std::endl;
        
        deep++;
        indent();
        std::cout << "Left:" << std::endl;
        deep++;
        node.left->accept(*this);
        deep--;
        
        indent();
        std::cout << "Right:" << std::endl;
        deep++;
        node.right->accept(*this);
        deep--;
        deep--;
    }

    void PrintASTVisitor::visit(PrimaryExpression& node){
        indent();
        std::cout << "Primary Expression:" << std::endl;
        deep++;
        
        switch (node.exprType) {
            case PrimaryExpression::Type::IDENTIFIER:
                indent();
                std::cout << "Identifier:" << std::endl;
                deep++;
                node.identifier->accept(*this);
                deep--;
                break;
                
            case PrimaryExpression::Type::LITERAL:
                indent();
                std::cout << "Literal:" << std::endl;
                deep++;
                node.literal->accept(*this);
                deep--;
                break;
                
            case PrimaryExpression::Type::EXPRESSION_CALL:
                node.functionCall->accept(*this);
                break;
                
            case PrimaryExpression::Type::PARENTHESIZED:
                indent();
                std::cout << "Parenthesized Expression:" << std::endl;
                deep++;
                node.parenthesized->accept(*this);
                deep--;
                break;
                
            case PrimaryExpression::Type::ARRAY_ACCESS:
                indent();
                std::cout << "Array Access:" << std::endl;
                // Handle array access if implemented
                break;
                
            case PrimaryExpression::Type::MEMBER_ACCESS:
                indent();
                std::cout << "Member Access:" << std::endl;
                // Handle member access if implemented
                break;
                
            case PrimaryExpression::Type::CAST_EXPRESSION:
                indent();
                std::cout << "Cast Expression:" << std::endl;
                // Handle cast if implemented
                break;
                
            case PrimaryExpression::Type::TERNARY_EXPRESSION:
                indent();
                std::cout << "Ternary Expression:" << std::endl;
                // Handle ternary if implemented
                break;
        }
        
        deep--;
    }
}