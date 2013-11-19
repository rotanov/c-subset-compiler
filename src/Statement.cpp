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

} // namespace Compiler
