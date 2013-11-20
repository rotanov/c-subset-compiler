#pragma once

#include <vector>
#include <cassert>

#include "ASTNode.hpp"

namespace Compiler
{
    class Statement : public ASTNode
    {
    public:
        Statement(const Token& token);

    private:
    };

    class SymbolTable;

    class CompoundStatement : public Statement
    {
    public:
        CompoundStatement(const Token& token, SymbolTable* symbols);

        void AddStatement(Statement* statement);

    private:
        SymbolTable* symbols_{NULL};
    };

    class ExpressionStatement : public Statement
    {
    public:
        ExpressionStatement();

        void SetExpression(ASTNode* expression);
        ASTNode* GetExpression() const;

    private:

    };

    class SelectionStatement : public Statement
    {
    public:
    private:

    };

    class IterationStatement : public Statement
    {
    public:
    private:

    };

    class JumpStatement : public Statement
    {
    public:
        JumpStatement(const Token& token);
        void SetReturnExpression(ASTNode* expression);
        ASTNode* GetReturnExpression() const;

    private:

    };

} // namespace Compiler
