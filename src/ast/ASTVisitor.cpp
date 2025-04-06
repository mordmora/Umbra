/**
 * @file ASTVisitor.cpp
 * @brief Implementation of the PrintASTVisitor class for AST visualization
 * @author Equipo Umbra
 * @date 7 de marzo de 2025
 *
 * This file contains the implementation of the PrintASTVisitor class, which is a concrete
 * visitor pattern implementation for traversing and printing an Abstract Syntax Tree (AST).
 * The visitor provides a hierarchical, indented text representation of the AST structure.
 *
 * Key features:
 * - Implements visit methods for all AST node types
 * - Uses indentation to show nesting levels
 * - Prints detailed information about each node
 * - Handles various expression types (binary, primary, function calls, etc.)
 * - Supports program structure elements (functions, parameters, variables)
 *
 * The printing format uses:
 * - Two spaces for each level of indentation
 * - Descriptive labels for each node type
 * - Hierarchical representation of nested structures
 * 
 * @see ASTVisitor.h
 * @see AST.h
 * @namespace umbra
 */
#include"ASTVisitor.h"
#include <iostream>

namespace umbra {

    /**
     * @brief Constructor for the PrintASTVisitor class
     * @details Initializes the indentation depth counter to zero. This counter
     * is used to track the hierarchical level in the AST during traversal.
     */
    PrintASTVisitor::PrintASTVisitor() : deep(0) {}

    /**
     * @brief Helper method to print the correct indentation
     * @details Outputs two spaces for each level of depth to visually represent
     * the hierarchical structure of the AST. The depth increases as the visitor
     * descends into nested nodes and decreases when it returns to higher levels.
     */
    void PrintASTVisitor::indent() {
        for (int i = 0; i < deep; ++i) {
            std::cout << "  ";
        }
    }

    /**
     * @brief Process a program node and all its contained functions
     * @param node The program node to visit
     * @details Prints the program node header and recursively visits all function
     * definitions contained within the program. Represents the entry point for
     * the entire AST traversal process.
     */
    void PrintASTVisitor::visit(ProgramNode& node) {
        indent();
        std::cout << "Program:" << std::endl;
        
        deep++;
        for (const auto& function : node.functions) {
            function->accept(*this);
        }
        deep--;
    }

    /**
     * @brief Process a function definition node and all its components
     * @param node The function definition node to visit
     * @details Prints the function's name, parameter list, return type, and
     * visits all statements in the function body. The function structure is
     * displayed with proper indentation showing the hierarchical relationship.
     */
    void PrintASTVisitor::visit(FunctionDefinition& node) {
        indent();
        std::cout << "Function Definition:" << std::endl;
        
        deep++;
        
        indent();
        std::cout << "Name: " << node.name->name << std::endl;
        
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
        if (node.returnValue) {
            indent();
            std::cout << "Return Value:" << std::endl;
            deep++;
            node.returnValue->accept(*this);
            deep--;
        }
        deep--;
        
        deep--;
    }

    /**
     * @brief Process a parameter list node
     * @param node The parameter list node to visit
     * @details Handles both empty parameter lists and those with parameters.
     * For each parameter, prints its type and name with proper indentation.
     * If there are no parameters, indicates this with a special message.
     */
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
            std::cout << "Name: " << param.second->name << std::endl;
            deep--;
        }
    }

    /**
     * @brief Process a type node
     * @param node The type node to visit
     * @details Prints the base type name and appends array brackets for 
     * array types according to the specified dimensions. No indentation
     * is performed as this is typically used inline within other node displays.
     */
    void PrintASTVisitor::visit(Type& node) {
        std::cout << node.baseType;
        for (int i = 0; i < node.arrayDimensions; ++i) {
            std::cout << "[]";
        }
        std::cout << std::endl;
    }

    /**
     * @brief Process an identifier node
     * @param node The identifier node to visit
     * @details Prints the name of the identifier with proper indentation.
     * Identifiers are fundamental elements that can appear in various contexts
     * such as variable references, function names, etc.
     */
    void PrintASTVisitor::visit(Identifier& node) {
        indent();
        std::cout << "Identifier: " << node.name << std::endl;
    }

    /**
     * @brief Process a variable declaration node
     * @param node The variable declaration node to visit
     * @details Prints the variable's type and name, and if an initializer is present,
     * recursively visits the initializer expression. The structure is properly
     * indented to show the relationship between components.
     */
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

    /**
     * @brief Process an assignment statement node
     * @param node The assignment statement node to visit
     * @details Prints the assignment operation structure, showing both the
     * target (left-hand side) and the value expression (right-hand side)
     * with proper indentation to represent their relationship.
     */
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

    /**
     * @brief Process a function call node
     * @param node The function call node to visit
     * @details Prints the function name being called and visits all argument
     * expressions if any exist. Handles the special case of a function call with
     * no arguments. The structure is properly indented to show the relationship.
     */
    void PrintASTVisitor::visit(FunctionCall& node){
        indent();
        std::cout << "Function Call:" << std::endl;
        deep++;
        
        indent();
        std::cout << "Function Name: " << node.functionName->name<<std::endl;
        
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

    /**
     * @brief Process a base expression node
     * @param node The expression node to visit
     * @details This is typically only called for the base Expression class,
     * which serves as the parent for more specific expression types. Prints
     * a generic label indicating a base expression node.
     */
    void PrintASTVisitor::visit(Expression& node) {
        indent();
        std::cout << "Expression (base)" << std::endl;
    }

    /**
     * @brief Process a literal node
     * @param node The literal node to visit
     * @details Prints the literal's value with proper indentation. Literals
     * represent constant values in the source code such as numbers.
     */
    void PrintASTVisitor::visit(Literal& node) {
        indent();
        std::cout << "Literal: " << node.value << "\n       Type: " << node.literalType << std::endl;
    }

    /**
     * @brief Process an expression statement node
     * @param node The expression statement node to visit
     * @details An expression statement is an expression used as a statement.
     * This method prints a header and then visits the contained expression.
     */
    void PrintASTVisitor::visit(ExpressionStatement& node) {
        indent();
        std::cout << "Expression Statement:" << std::endl;
        deep++;
        node.exp->accept(*this);
        deep--;
    }

    /**
     * @brief Process a binary expression node
     * @param node The binary expression node to visit
     * @details Prints the binary operator and recursively visits the left and
     * right operand expressions. The structure is properly indented to show
     * the relationship between the operator and its operands.
     */
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

    /**
     * @brief Process a primary expression node
     * @param node The primary expression node to visit
     * @details Handles various types of primary expressions based on the expression type.
     * This includes identifiers, literals, function calls, parenthesized expressions,
     * and other special types like array access or ternary expressions. The appropriate
     * processing is selected using a switch statement based on the expression type.
     */
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

    /**
     * @brief Process a string literal node
     * @param node The string literal node to visit
     * @details Prints the string literal value with surrounding quotes and
     * proper indentation. String literals represent text constants in the source code.
     */
    void PrintASTVisitor::visit(StringLiteral& node) {
        indent();
        std::cout << "String Literal: \"" << node.value << "\"" << std::endl;
    }

    /**
     * @brief Process a return expression node
     * @param node The return expression node to visit
     * @details Prints the return keyword and recursively visits the contained
     * expression if present. The structure is properly indented to show the
     * relationship between the return statement and its value.
     */
    void PrintASTVisitor::visit(ReturnExpression& node) {
        indent();
        std::cout << "Return Expression:" << std::endl;
        
        deep++;
        if (node.returnValue) {
            node.returnValue->accept(*this);
        } else {
            indent();
            std::cout << "(no return value)" << std::endl;
        }
        deep--;
    }
}