#pragma once

#include "Token.hpp"

#include <vector>
#include <cassert>

#include "utils.hpp"

namespace Compiler
{
    class ASTNode
    {
    public:
        class Iterator
        {

        };

        Token token;

        ASTNode(const Token& token);
        virtual ~ASTNode();

        int GetChildCount() const
        {
            return children_.size();
        }

        ASTNode* GetChild(const int index)
        {
            assert(index >= 0 && index < children_.size());
            return children_[index];
        }

    protected:
        std::vector<ASTNode*> children_;
    };

    class ASTNodeBinaryOperator : public ASTNode
    {
    public:
        ASTNodeBinaryOperator(const Token& token, ASTNode* left, ASTNode* right)
            : ASTNode(token)
        {
            assert(left != NULL && right != NULL);
//            assert(IsBinaryOperator(token));
            children_.push_back(left);
            children_.push_back(right);
        }
    };

    class ASTNodeAssignment : public ASTNodeBinaryOperator
    {
    public:
        ASTNodeAssignment(const Token& token, ASTNode* left, ASTNode* right)
            : ASTNodeBinaryOperator(token, left, right)
        {

        }
    };

    class ASTNodeUnaryOperator : public ASTNode
    {
    public:
        ASTNodeUnaryOperator(const Token& token)
            : ASTNode(token)
        {

        }

        ASTNodeUnaryOperator(const Token& token, ASTNode* node)
            : ASTNode(token)
        {
            assert(node != NULL);
            children_.push_back(node);
        }

        void SetOperand(ASTNode* node)
        {
            assert(children_.size() == 0);
            assert(node != NULL);
            children_.push_back(node);
        }

        ASTNode* GetOperand()
        {
            assert(children_.size() == 1);
            return children_[0];
        }
    };


    class ASTNodeConditionalOperator : public ASTNode
    {
    public:
        ASTNodeConditionalOperator(const Token& token, ASTNode* condition,
                                   ASTNode* thenExpression, ASTNode* elseExpression)
            // TOOD: dunno whatever the reason for token to be here
            // should it be `?` or `:`
            : ASTNode(token)
        {
            assert(condition != NULL);
            assert(thenExpression != NULL);
            assert(elseExpression != NULL);
            children_.push_back(condition);
            children_.push_back(thenExpression);
            children_.push_back(elseExpression);
        }

    };

    class ASTNodeArraySubscript : public ASTNodeBinaryOperator
    {
    public:
        ASTNodeArraySubscript(const Token& token, ASTNode* left, ASTNode* right)
            : ASTNodeBinaryOperator(token, left, right)
        {

        }

    };

    class ASTNodeFunctionCall : public ASTNode
    {
    public:
        ASTNodeFunctionCall(const Token& token, ASTNode* caller)
            : ASTNode(token)
        {
            assert(caller != NULL);
            children_.push_back(caller);
        }

        void AddArgumentExpressionNode(ASTNode* node)
        {
            assert(children_.size() > 0);
            assert(node != NULL);
            children_.push_back(node);
        }

    };

    class ASTNodeStructureAccess : public ASTNode
    {
    public:
        ASTNodeStructureAccess(const Token& token, ASTNode* lhs, ASTNode* rhs)
            : ASTNode(token)
        {
            assert(lhs != NULL);
            assert(rhs != NULL);
            children_.push_back(lhs);
            children_.push_back(rhs);
        }

    };

} // namespace Compiler
