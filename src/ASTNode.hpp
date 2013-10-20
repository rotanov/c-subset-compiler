#pragma once

#include "Token.hpp"

namespace Compiler
{
    class ASTNode
    {
    public:
        Token token;

        ASTNode(const Token& token);
        virtual ~ASTNode();

    };

    class ASTNodeBinaryOperator : public ASTNode
    {

    };

    class ASTNodeUnaryOperator : public ASTNode
    {

    };

    class ASTNodeConditionalOperator : public ASTNode
    {

    };

    class ASTNodeFunctionCall : public ASTNode
    {

    };

} // namespace Compiler
