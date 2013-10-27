#pragma once

#include "Token.hpp"

#include <vector>
#include <cassert>

#include "utils.hpp"

namespace Compiler
{
////////////////////////////////////////////////////////////////////////////////
    class ASTNode
    {
    public:
        Token token;

        ASTNode(const Token& token);
        virtual ~ASTNode();

        int GetChildCount() const;
        ASTNode* GetChild(const int index);

    protected:
        std::vector<ASTNode*> children_;

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeBinaryOperator : public ASTNode
    {
    public:
        ASTNodeBinaryOperator(const Token& token, ASTNode* left, ASTNode* right);

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeAssignment : public ASTNodeBinaryOperator
    {
    public:
        ASTNodeAssignment(const Token& token, ASTNode* left, ASTNode* right);

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeUnaryOperator : public ASTNode
    {
    public:
        ASTNodeUnaryOperator(const Token& token);
        ASTNodeUnaryOperator(const Token& token, ASTNode* node);

        void SetOperand(ASTNode* node);
        ASTNode* GetOperand();

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeConditionalOperator : public ASTNode
    {
    public:
        ASTNodeConditionalOperator(const Token& token, ASTNode* condition,
                                   ASTNode* thenExpression, ASTNode* elseExpression);

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeArraySubscript : public ASTNodeBinaryOperator
    {
    public:
        ASTNodeArraySubscript(const Token& token, ASTNode* left, ASTNode* right);

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeFunctionCall : public ASTNode
    {
    public:
        ASTNodeFunctionCall(const Token& token, ASTNode* caller);

        void AddArgumentExpressionNode(ASTNode* node);

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeStructureAccess : public ASTNode
    {
    public:
        ASTNodeStructureAccess(const Token& token, ASTNode* lhs, ASTNode* rhs);

    };

} // namespace Compiler
