#include "ASTVisitor.h"
#include "../../lexer/Tokens.h"
#include "../statementnodes/StatementNode.h"
#include <iostream>

namespace umbra {

PrintVisitor::PrintVisitor() : indentLevel(0) {}

PrintVisitor::~PrintVisitor() = default;

void PrintVisitor::visit(ProgramNode &node) {
    printIndent();
    std::cout << "ProgramNode:" << std::endl;
    increaseIndent();

    for (const auto &statement : node.getStatements()) {
        if (statement) {
            statement->accept(*this); // Invoca el visitor sobre cada tipo de nodo
        }
    }

    decreaseIndent();
}

// statements
void PrintVisitor::visit(st_VariableDeclNode &node) {
    printIndent();
    std::cout << "VariableDeclaration:" << std::endl;
    increaseIndent();

    printIndent();
    std::cout << "Type: " << TokenManager::tokenTypeToString(node.getType()) << std::endl;

    printIndent();
    std::cout << "Name: " << node.getName() << std::endl;

    if (node.getArraySize()) {
        printIndent();
        std::cout << "Array Size: " << std::endl;
        increaseIndent();
        node.getArraySize()->accept(*this);
        decreaseIndent();
    }

    if (node.getInitializer()) {
        printIndent();
        std::cout << "Initializer: " << std::endl;
        increaseIndent();
        node.getInitializer()->accept(*this);
        decreaseIndent();
    }

    decreaseIndent();
}

// expressions

void PrintVisitor::visit(ex_IntegerLiteralNode &node) {
    printIndent();
    std::cout << "IntegerLiteral: " << node.value << std::endl;
}

void PrintVisitor::visit(ex_FloatLiteralNode &node) {
    printIndent();
    std::cout << "FloatLiteral: " << node.value << std::endl;
}

void PrintVisitor::visit(ex_StringLiteralNode &node) {
    printIndent();
    std::cout << "StringLiteral: \"" << node.value << "\"" << std::endl;
}

void PrintVisitor::visit(ex_CharLiteralNode &node) {
    printIndent();
    std::cout << "CharLiteral: \"" << node.value << "\"" << std::endl;
}

void PrintVisitor::visit(ex_BoolLiteralNode &node) {
    printIndent();
    std::cout << "BooleanLiteral: " << (node.value ? "true" : "false") << std::endl;
}

void PrintVisitor::visit(ex_IdentifierNode &node) {
    printIndent();
    std::cout << "IdentifierNode: " << node.name << std::endl;
}

void PrintVisitor::increaseIndent() { ++indentLevel; }

void PrintVisitor::decreaseIndent() { --indentLevel; }

void PrintVisitor::printIndent() {
    for (int i = 0; i < indentLevel; ++i) {
        std::cout << "    ";
    }
}

} // namespace umbra