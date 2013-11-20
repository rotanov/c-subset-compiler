#include "Statement.hpp"

namespace Compiler
{

    Statement::Statement(const Token& token)
        : ASTNode(token)
    {

    }

    CompoundStatement::CompoundStatement(const Token& token, SymbolTable* symbols)
        : Statement(token)
        , symbols_(symbols)
    {

    }

    void CompoundStatement::AddStatement(Statement* statement)
    {
        assert(statement != NULL);
        children_.push_back(statement);
    }

    ExpressionStatement::ExpressionStatement()
        : Statement(Token(TT_EXPRESSION_STATEMENT, "(expression-statement)"))
    {

    }

    void ExpressionStatement::SetExpression(ASTNode* expression)
    {
        assert(expression != NULL);
        assert(children_.size() == 0);
        children_.push_back(expression);
    }

    ASTNode*ExpressionStatement::GetExpression() const
    {
        return children_.size() == 0 ? NULL : children_[0];
    }

    JumpStatement::JumpStatement(const Token& token)
        : Statement(token)
    {

    }

    void JumpStatement::SetReturnExpression(ASTNode* expression)
    {
        assert(expression != NULL);
        assert(children_.size() == 0);
        assert(token == KW_RETURN);
        children_.push_back(expression);
    }

    ASTNode*JumpStatement::GetReturnExpression() const
    {
        return children_.size() == 0 ? NULL : children_[0];
    }

} // namespace Compiler
