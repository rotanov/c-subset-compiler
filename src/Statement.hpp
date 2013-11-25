#pragma once

#include <vector>
#include <cassert>

#include "ASTNode.hpp"

namespace Compiler
{
    enum class EStatementType
    {
        COMPOUND,
        SELECTION,
        ITERATION_FOR,
        ITERATION_DO,
        ITERATION_WHILE,
        JUMP,
        EXPRESSION,
    };

    class Statement : public ASTNode
    {
    public:
        Statement(const Token& token);

        virtual bool IsStatement() const { return true; }
        virtual EStatementType GetStatementType() const = 0;

    private:

    };

    class SymbolTable;

    class CompoundStatement : public Statement
    {
    public:
        CompoundStatement(const Token& token, SymbolTable* symbols);

        void AddStatement(Statement* statement);
        virtual EStatementType GetStatementType() const { return EStatementType::COMPOUND; }
        SymbolTable* GetSymbolTable() const;

    private:
        SymbolTable* symbols_{NULL};

    };

    class ExpressionStatement : public Statement
    {
    public:
        ExpressionStatement();

        void SetExpression(ASTNode* expression);
        ASTNode* GetExpression() const;
        virtual EStatementType GetStatementType() const { return EStatementType::EXPRESSION; }

    private:

    };

    class SelectionStatement : public Statement
    {
    public:
        SelectionStatement();

        void SetConditionExpression(ASTNode* conditionExpression);
        void SetStatementForIf(Statement* statement);
        void SetStatementForElse(Statement* statement);
        virtual EStatementType GetStatementType() const { return EStatementType::SELECTION; }

    private:

    };

    class IterationStatement : public Statement
    {
    public:
        virtual void SetControllingExpression(ASTNode* controllingExpression) = 0;
        virtual void SetLoopStatement(Statement* loopStatement) = 0;

    protected:
        IterationStatement(const Token& token);

    };

    class ForStatement : public IterationStatement
    {
    public:
        ForStatement();
        virtual EStatementType GetStatementType() const { return EStatementType::ITERATION_FOR; }
        SymbolTable* GetSymbolTable() const;
        void SetSymbolTable(SymbolTable* symbols);
        void SetInitializingExpression(ASTNode* initializingExpression);
        virtual void SetControllingExpression(ASTNode* controllingExpression);
        void SetIterationExpression(ASTNode* iterationExpression);
        virtual void SetLoopStatement(Statement* loopStatement);

    private:
        SymbolTable* symbols_{NULL};

    };

    class DoStatement : public IterationStatement
    {
    public:
        DoStatement();
        virtual EStatementType GetStatementType() const { return EStatementType::ITERATION_DO; }
        virtual void SetControllingExpression(ASTNode* controllingExpression);
        virtual void SetLoopStatement(Statement* loopStatement);

    };

    class WhileStatement : public IterationStatement
    {
    public:
        WhileStatement();
        virtual EStatementType GetStatementType() const { return EStatementType::ITERATION_WHILE; }
        virtual void SetControllingExpression(ASTNode* controllingExpression);
        virtual void SetLoopStatement(Statement* loopStatement);

    };

    class JumpStatement : public Statement
    {
    public:
        JumpStatement(const Token& token);
        void SetReturnExpression(ASTNode* expression);
        ASTNode* GetReturnExpression() const;
        virtual EStatementType GetStatementType() const { return EStatementType::JUMP; }
        void SetRefLoopStatement(IterationStatement* iterationStatement);

    private:
        IterationStatement* refLoop_{NULL};
    };

} // namespace Compiler
