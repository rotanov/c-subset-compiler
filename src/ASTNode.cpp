#include "ASTNode.hpp"

namespace Compiler
{
////////////////////////////////////////////////////////////////////////////////
    ASTNode::~ASTNode()
    {
        while(!children_.empty())
        {
            delete children_.back();
            children_.pop_back();
        }
    }

//------------------------------------------------------------------------------
    int ASTNode::GetChildCount() const
    {
        return children_.size();
    }

//------------------------------------------------------------------------------
    ASTNode* ASTNode::GetChild(const int index)
    {
        assert(index >= 0 && index < children_.size());
        return children_[index];
    }

//------------------------------------------------------------------------------
    ASTNode::ASTNode(const Token &token)
        : token(token)
    {

    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeBinaryOperator::ASTNodeBinaryOperator(const Token& token, ASTNode* left, ASTNode* right)
        : ASTNode(token)
    {
        assert(left != NULL && right != NULL);
//        assert(IsBinaryOperator(token));
        children_.push_back(left);
        children_.push_back(right);
    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeAssignment::ASTNodeAssignment(const Token& token, ASTNode* left, ASTNode* right)
        : ASTNodeBinaryOperator(token, left, right)
    {

    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeUnaryOperator::ASTNodeUnaryOperator(const Token& token)
        : ASTNode(token)
    {

    }

//------------------------------------------------------------------------------
    ASTNodeUnaryOperator::ASTNodeUnaryOperator(const Token& token, ASTNode* node)
        : ASTNode(token)
    {
        assert(node != NULL);
        children_.push_back(node);
    }

//------------------------------------------------------------------------------
    void ASTNodeUnaryOperator::SetOperand(ASTNode* node)
    {
        assert(children_.size() == 0);
        assert(node != NULL);
        children_.push_back(node);
    }

//------------------------------------------------------------------------------
    ASTNode*ASTNodeUnaryOperator::GetOperand()
    {
        assert(children_.size() == 1);
        return children_[0];
    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeConditionalOperator::ASTNodeConditionalOperator(const Token& token,
        ASTNode* condition, ASTNode* thenExpression, ASTNode* elseExpression)
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

////////////////////////////////////////////////////////////////////////////////
    ASTNodeArraySubscript::ASTNodeArraySubscript(const Token& token, ASTNode* left, ASTNode* right)
        : ASTNodeBinaryOperator(token, left, right)
    {

    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeFunctionCall::ASTNodeFunctionCall(const Token& token, ASTNode* caller)
        : ASTNode(token)
    {
        assert(caller != NULL);
        children_.push_back(caller);
    }

//------------------------------------------------------------------------------
    void ASTNodeFunctionCall::AddArgumentExpressionNode(ASTNode* node)
    {
        assert(children_.size() > 0);
        assert(node != NULL);
        children_.push_back(node);
    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeStructureAccess::ASTNodeStructureAccess(const Token& token, ASTNode* lhs, ASTNode* rhs)
        : ASTNode(token)
    {
        assert(lhs != NULL);
        assert(rhs != NULL);
        children_.push_back(lhs);
        children_.push_back(rhs);
    }


} // namespace
