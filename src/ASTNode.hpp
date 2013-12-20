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
        shared_ptr<ASTNode> GetChild(const int index);

    protected:
        std::vector<shared_ptr<ASTNode>> children_;

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeBinaryOperator : public ASTNode
    {
    public:
        ASTNodeBinaryOperator(const Token& token, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right);

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeAssignment : public ASTNodeBinaryOperator
    {
    public:
        ASTNodeAssignment(const Token& token, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right);

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeUnaryOperator : public ASTNode
    {
    public:
        ASTNodeUnaryOperator(const Token& token);
        ASTNodeUnaryOperator(const Token& token, shared_ptr<ASTNode> node);

        void SetOperand(shared_ptr<ASTNode> node);
        shared_ptr<ASTNode> GetOperand();

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeConditionalOperator : public ASTNode
    {
    public:
        ASTNodeConditionalOperator(const Token& token, shared_ptr<ASTNode> condition,
                                   shared_ptr<ASTNode> thenExpression, shared_ptr<ASTNode> elseExpression);

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeArraySubscript : public ASTNodeBinaryOperator
    {
    public:
        ASTNodeArraySubscript(const Token& token, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right);

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeFunctionCall : public ASTNode
    {
    public:
        ASTNodeFunctionCall(const Token& token, shared_ptr<ASTNode> caller);

        void AddArgumentExpressionNode(shared_ptr<ASTNode> node);

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeStructureAccess : public ASTNode
    {
    public:
        ASTNodeStructureAccess(const Token& token, shared_ptr<ASTNode> lhs, shared_ptr<ASTNode> rhs);

    };

////////////////////////////////////////////////////////////////////////////////
    class SymbolType;
    class ASTNodeTypeName : public ASTNode
    {
    public:
        ASTNodeTypeName(shared_ptr<SymbolType> typeNameSymbol);

    private:
        shared_ptr<SymbolType> typeSymbol_{NULL};
    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeCast : public ASTNodeBinaryOperator
    {
    public:
        ASTNodeCast(shared_ptr<ASTNodeTypeName> left, shared_ptr<ASTNode> right);
    };

} // namespace Compiler
