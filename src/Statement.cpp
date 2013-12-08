#include "Statement.hpp"

namespace Compiler
{

    Statement::Statement(const Token& token)
        : ASTNode(token)
    {

    }

    CompoundStatement::CompoundStatement(const Token& token, shared_ptr<SymbolTable> symbols)
        : Statement(Token(OP_LBRACE, "{"))
        , symbols_(symbols)
    {

    }

    void CompoundStatement::AddStatement(shared_ptr<Statement> statement)
    {
        assert(statement != NULL);
        children_.push_back(statement);
    }

    shared_ptr<SymbolTable> CompoundStatement::GetSymbolTable() const
    {
        assert(symbols_ != NULL);
        return symbols_;
    }

    ExpressionStatement::ExpressionStatement()
    // (expression-statement) - is too much
        : Statement(Token(OP_SEMICOLON, ";"))
    {

    }

    void ExpressionStatement::SetExpression(shared_ptr<ASTNode> expression)
    {
        assert(expression != NULL);
        assert(children_.size() == 0);
        children_.push_back(expression);
    }

    shared_ptr<ASTNode> ExpressionStatement::GetExpression() const
    {
        return children_.size() == 0 ? NULL : children_[0];
    }

    JumpStatement::JumpStatement(const Token& token)
        : Statement(token)
    {

    }

    void JumpStatement::SetReturnExpression(shared_ptr<ASTNode> expression)
    {
        assert(expression != NULL);
        assert(children_.size() == 0);
        assert(token == KW_RETURN);
        children_.push_back(expression);
    }

    shared_ptr<ASTNode> JumpStatement::GetReturnExpression() const
    {
        return children_.size() == 0 ? NULL : children_[0];
    }

    void JumpStatement::SetRefLoopStatement(shared_ptr<IterationStatement> iterationStatement)
    {
        assert(iterationStatement != NULL);
        assert(token == KW_CONTINUE
               || token == KW_BREAK);
        refLoop_ = iterationStatement;
    }

    SelectionStatement::SelectionStatement()
        : Statement(Token(KW_IF, "if"))
    {

    }

    void SelectionStatement::SetConditionExpression(shared_ptr<ASTNode> conditionExpression)
    {
        assert(conditionExpression != NULL);
        assert(children_.size() == 0);
        children_.push_back(conditionExpression);
    }

    void SelectionStatement::SetStatementForIf(shared_ptr<Statement> statement)
    {
        assert(statement != NULL);
        assert(children_.size() == 1);
        children_.push_back(statement);
    }

    void SelectionStatement::SetStatementForElse(shared_ptr<Statement> statement)
    {
        assert(statement != NULL);
        assert(children_.size() == 2);
        children_.push_back(statement);
    }

    IterationStatement::IterationStatement(const Token& token)
        : Statement(token)
    {

    }

    ForStatement::ForStatement()
        : IterationStatement(Token(KW_FOR, "for"))
    {

    }

    shared_ptr<SymbolTable> ForStatement::GetSymbolTable() const
    {
        return symbols_;
    }

    void ForStatement::SetSymbolTable(shared_ptr<SymbolTable> symbols)
    {
        assert(symbols != NULL);
        symbols_ = symbols;
    }

    void ForStatement::SetInitializingExpression(shared_ptr<ASTNode> initializingExpression)
    {
        assert(initializingExpression != NULL);
        assert(children_.size() == 0);
        children_.push_back(initializingExpression);
    }

    void ForStatement::SetControllingExpression(shared_ptr<ASTNode> controllingExpression)
    {
        assert(controllingExpression != NULL);
        assert(children_.size() == 1
               || children_.size() == 0);

        // no initializing expression
        if (children_.size() == 0)
        {
            children_.push_back(make_shared<ASTNode>(Token(TT_INVALID, "stub")));
        }
        children_.push_back(controllingExpression);
    }

    void ForStatement::SetIterationExpression(shared_ptr<ASTNode> iterationExpression)
    {
        assert(children_.size() == 2);
        assert(iterationExpression != NULL);
        children_.push_back(iterationExpression);
    }

    void ForStatement::SetLoopStatement(shared_ptr<Statement> loopStatement)
    {
        assert(loopStatement != NULL);
        assert(children_.size() == 3);
        children_.push_back(loopStatement);
    }

    DoStatement::DoStatement()
        : IterationStatement(Token(KW_DO, "do"))
    {

    }

    void DoStatement::SetControllingExpression(shared_ptr<ASTNode> controllingExpression)
    {
        assert(controllingExpression != NULL);
        assert(children_.size() == 0);
        children_.push_back(controllingExpression);
    }

    void DoStatement::SetLoopStatement(shared_ptr<Statement> loopStatement)
    {
        assert(loopStatement != NULL);
        assert(children_.size() == 1);
        children_.push_back(loopStatement);
    }

    WhileStatement::WhileStatement()
        : IterationStatement(Token(KW_WHILE, "while"))
    {

    }

    void WhileStatement::SetControllingExpression(shared_ptr<ASTNode> controllingExpression)
    {
        assert(controllingExpression != NULL);
        assert(children_.size() == 0);
        children_.push_back(controllingExpression);
    }

    void WhileStatement::SetLoopStatement(shared_ptr<Statement> loopStatement)
    {
        assert(loopStatement != NULL);
        assert(children_.size() == 1);
        children_.push_back(loopStatement);
    }



} // namespace Compiler
