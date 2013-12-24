#pragma once

#include "Token.hpp"

#include <vector>
#include <cassert>

#include "utils.hpp"

namespace Compiler
{
    class SymbolType;
////////////////////////////////////////////////////////////////////////////////
    class ASTNode
    {
    public:
        Token token;

        ASTNode(const Token& token);
        ASTNode(const Token& token, shared_ptr<SymbolType> type);
        virtual ~ASTNode();

        int GetChildCount() const;
        shared_ptr<ASTNode> GetChild(const int index);
        void SetTypeSym(shared_ptr<SymbolType> type);
        shared_ptr<SymbolType> GetTypeSym() const;

    protected:
        std::vector<shared_ptr<ASTNode>> children_;
        shared_ptr<SymbolType> typeSym_{NULL};
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

    private:
        void CheckTypes_();

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
        ASTNodeFunctionCall(const Token& token, shared_ptr<ASTNode> caller, vector<shared_ptr<ASTNode>>& parameters);

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeStructureAccess : public ASTNode
    {
    public:
        ASTNodeStructureAccess(const Token& token, shared_ptr<ASTNode> lhs, shared_ptr<ASTNode> rhs);

    };

////////////////////////////////////////////////////////////////////////////////
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

    bool IfLValue(shared_ptr<ASTNode> node);
    bool IfModifiableLValue(shared_ptr<ASTNode> node);

} // namespace Compiler
