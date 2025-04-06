/**
 * @file ASTVisitor.h
 * @brief Declaration of the AST visitor pattern interface and concrete implementations
 *
 * This file defines the visitor pattern interface for traversing the Umbra Abstract Syntax Tree (AST).
 * It provides both the abstract base visitor class and concrete implementations like PrintASTVisitor.
 */

#ifndef UMBRA_AST_VISITOR_H
#define UMBRA_AST_VISITOR_H

#include "Nodes.h"

namespace umbra {

/**
 * @class ASTVisitor
 * @brief Abstract visitor interface for AST traversal
 *
 * Defines the interface for all concrete visitors that need to traverse the AST.
 * The visitor pattern allows adding new operations to the AST nodes without modifying their classes.
 */
class ASTVisitor {
public:
    /**
     * @brief Virtual destructor for proper cleanup in derived classes
     */
    virtual ~ASTVisitor() = default;
    
    /**
     * @brief Visit method for ProgramNode
     * @param node The program node to visit
     */
    virtual void visit(ProgramNode& node) = 0;
    
    /**
     * @brief Visit method for FunctionDefinition
     * @param node The function definition node to visit
     */
    virtual void visit(FunctionDefinition& node) = 0;
    
    /**
     * @brief Visit method for ParameterList
     * @param node The parameter list node to visit
     */
    virtual void visit(ParameterList& node) = 0;
    
    /**
     * @brief Visit method for Type
     * @param node The type node to visit
     */
    virtual void visit(Type& node) = 0;
    
    /**
     * @brief Visit method for Identifier
     * @param node The identifier node to visit
     */
    virtual void visit(Identifier& node) = 0;
    
    /**
     * @brief Visit method for VariableDeclaration
     * @param node The variable declaration node to visit
     */
    virtual void visit(VariableDeclaration& node) = 0;
    
    /**
     * @brief Visit method for AssignmentStatement
     * @param node The assignment statement node to visit
     */
    virtual void visit(AssignmentStatement& node) = 0;
    
    /**
     * @brief Visit method for FunctionCall
     * @param node The function call node to visit
     */
    virtual void visit(FunctionCall& node) = 0;
    
    /**
     * @brief Visit method for Expression
     * @param node The expression node to visit
     */
    virtual void visit(Expression& node) = 0;
    
    /**
     * @brief Visit method for Literal
     * @param node The literal node to visit
     */
    virtual void visit(Literal& node) = 0;
    
    /**
     * @brief Visit method for ExpressionStatement
     * @param node The expression statement node to visit
     */
    virtual void visit(ExpressionStatement& node) = 0;
    
    /**
     * @brief Visit method for BinaryExpression
     * @param node The binary expression node to visit
     */
    virtual void visit(BinaryExpression& node) = 0;
    
    /**
     * @brief Visit method for PrimaryExpression
     * @param node The primary expression node to visit
     */
    virtual void visit(PrimaryExpression& node) = 0;
    
    /**
     * @brief Visit method for StringLiteral
     * @param node The string literal node to visit
     */
    virtual void visit(StringLiteral& node) = 0;

    /**
     * @brief Visit method for CharLiteral
     * @param node The char literal node to visit
     */
    virtual void visit(CharLiteral& node) = 0;

    /**
     * @brief Visit method for BooleanLiteral
     * @param node The boolean literal node to visit
     */
    virtual void visit(BooleanLiteral& node) = 0;

    /**
     * @brief Visit method for NumericLiteral
     * @param node The numeric literal node to visit
     */
    virtual void visit(NumericLiteral& node) = 0;

    /**
     * @brief Visit method for ReturnExpression
     * @param node The return expression node to visit
     */
    virtual void visit(ReturnExpression& node) = 0;
};

/**
 * @class BaseVisitor
 * @brief Base class for AST visitors with default implementations
 *
 * This class provides default implementations for all visit methods in the ASTVisitor interface.
 * Derived classes can override only the methods they are interested in.
 */

class BaseVisitor : public ASTVisitor {
    public:
        virtual ~BaseVisitor() = default;
        
        // Default implementations for all methods
        void visit(ProgramNode& node) override {}
        void visit(FunctionDefinition& node) override {}
        void visit(ParameterList& node) override {}
        void visit(Type& node) override {}
        void visit(Identifier& node) override {}
        void visit(VariableDeclaration& node) override {}
        void visit(AssignmentStatement& node) override {}
        void visit(FunctionCall& node) override {}
        void visit(Expression& node) override {}
        void visit(Literal& node) override {}
        void visit(ExpressionStatement& node) override {}
        void visit(BinaryExpression& node) override {}
        void visit(PrimaryExpression& node) override {}
        void visit(StringLiteral& node) override {}
        void visit(CharLiteral &node) override {}
        void visit(BooleanLiteral &node) override {}
        void visit(NumericLiteral &node) override {}
        void visit(ReturnExpression& node) override {}
        // Add any other visit methods required by ASTVisitor
};


/**
 * @class PrintASTVisitor
 * @brief Concrete visitor implementation that prints the AST structure
 *
 * This visitor traverses the AST and produces a hierarchical text representation
 * with proper indentation to visualize the tree structure.
 */
class PrintASTVisitor : public BaseVisitor {
private:
    /**
     * @brief Current indentation depth in the tree
     */
    int deep;
    
    /**
     * @brief Helper method to print the correct indentation for the current depth
     */
    void indent();

public:
    /**
     * @brief Constructor initializing the indentation depth to zero
     */
    PrintASTVisitor();
    
    /**
     * @brief Visit implementation for ProgramNode
     * @param node The program node to process
     */
    void visit(ProgramNode& node) override;
    
    /**
     * @brief Visit implementation for FunctionDefinition
     * @param node The function definition node to process
     */
    void visit(FunctionDefinition& node) override;
    
    /**
     * @brief Visit implementation for ParameterList
     * @param node The parameter list node to process
     */
    void visit(ParameterList& node) override;
    
    /**
     * @brief Visit implementation for Type
     * @param node The type node to process
     */
    void visit(Type& node) override;
    
    /**
     * @brief Visit implementation for Identifier
     * @param node The identifier node to process
     */
    void visit(Identifier& node) override;
    
    /**
     * @brief Visit implementation for VariableDeclaration
     * @param node The variable declaration node to process
     */
    void visit(VariableDeclaration& node) override;
    
    /**
     * @brief Visit implementation for AssignmentStatement
     * @param node The assignment statement node to process
     */
    void visit(AssignmentStatement& node) override;
    
    /**
     * @brief Visit implementation for FunctionCall
     * @param node The function call node to process
     */
    void visit(FunctionCall& node) override;
    
    /**
     * @brief Visit implementation for Expression
     * @param node The expression node to process
     */
    void visit(Expression& node) override;
    
    /**
     * @brief Visit implementation for Literal
     * @param node The literal node to process
     */
    void visit(Literal& node) override;
    
    /**
     * @brief Visit implementation for ExpressionStatement
     * @param node The expression statement node to process
     */
    void visit(ExpressionStatement& node) override;
    
    /**
     * @brief Visit implementation for BinaryExpression 
     * @param node The binary expression node to process
     */
    void visit(BinaryExpression& node) override;
    
    /**
     * @brief Visit implementation for PrimaryExpression
     * @param node The primary expression node to process
     */
    void visit(PrimaryExpression& node) override;
    
    /**
     * @brief Visit implementation for StringLiteral
     * @param node The string literal node to process
     */
    void visit(StringLiteral& node) override;

    /**
     * @brief Visit implementation for ReturnExpression
     * @param node The return expression node to process
     */
    void visit(ReturnExpression& node) override;
};

} // namespace umbra

#endif // UMBRA_AST_VISITOR_H