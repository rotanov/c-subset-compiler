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
        CompoundStatement(shared_ptr<SymbolTable> symbols);

        void AddStatement(shared_ptr<Statement> statement);
        virtual EStatementType GetStatementType() const { return EStatementType::COMPOUND; }
        shared_ptr<SymbolTable> GetSymbolTable() const;

    private:
        shared_ptr<SymbolTable> symbols_{NULL};

    };

    class ExpressionStatement : public Statement
    {
    public:
        ExpressionStatement();

        void SetExpression(shared_ptr<ASTNode> expression);
        shared_ptr<ASTNode> GetExpression() const;
        virtual EStatementType GetStatementType() const { return EStatementType::EXPRESSION; }

    private:

    };

    class SelectionStatement : public Statement
    {
    public:
        SelectionStatement();

        void SetConditionExpression(shared_ptr<ASTNode> conditionExpression);
        void SetStatementForIf(shared_ptr<Statement> statement);
        void SetStatementForElse(shared_ptr<Statement> statement);
        virtual EStatementType GetStatementType() const { return EStatementType::SELECTION; }

    private:

    };

    class IterationStatement : public Statement
    {
    public:
        virtual void SetControllingExpression(shared_ptr<ASTNode> controllingExpression) = 0;
        virtual void SetLoopStatement(shared_ptr<Statement> loopStatement) = 0;

    protected:
        IterationStatement(const Token& token);

    };

    class ForStatement : public IterationStatement
    {
    public:
        ForStatement();
        virtual EStatementType GetStatementType() const { return EStatementType::ITERATION_FOR; }
        shared_ptr<SymbolTable> GetSymbolTable() const;
        void SetSymbolTable(shared_ptr<SymbolTable> symbols);
        void SetInitializingExpression(shared_ptr<ASTNode> initializingExpression);
        virtual void SetControllingExpression(shared_ptr<ASTNode> controllingExpression);
        void SetIterationExpression(shared_ptr<ASTNode> iterationExpression);
        virtual void SetLoopStatement(shared_ptr<Statement> loopStatement);

    private:
        shared_ptr<SymbolTable> symbols_{NULL};

    };

    class DoStatement : public IterationStatement
    {
    public:
        DoStatement();
        virtual EStatementType GetStatementType() const { return EStatementType::ITERATION_DO; }
        virtual void SetControllingExpression(shared_ptr<ASTNode> controllingExpression);
        virtual void SetLoopStatement(shared_ptr<Statement> loopStatement);

    };

    class WhileStatement : public IterationStatement
    {
    public:
        WhileStatement();
        virtual EStatementType GetStatementType() const { return EStatementType::ITERATION_WHILE; }
        virtual void SetControllingExpression(shared_ptr<ASTNode> controllingExpression);
        virtual void SetLoopStatement(shared_ptr<Statement> loopStatement);

    };

    class JumpStatement : public Statement
    {
    public:
        JumpStatement(const Token& token);
        void SetReturnExpression(shared_ptr<ASTNode> expression);
        shared_ptr<ASTNode> GetReturnExpression() const;
        virtual EStatementType GetStatementType() const { return EStatementType::JUMP; }
        void SetRefLoopStatement(shared_ptr<IterationStatement> iterationStatement);

    private:
        shared_ptr<IterationStatement> refLoop_{NULL};
    };

} // namespace Compiler
