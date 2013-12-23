#include "ASTNode.hpp"

#include <iostream>

#include "SymbolTable.hpp"

namespace Compiler
{
////////////////////////////////////////////////////////////////////////////////
    ASTNode::~ASTNode()
    {

    }

//------------------------------------------------------------------------------
    int ASTNode::GetChildCount() const
    {
        return children_.size();
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> ASTNode::GetChild(const int index)
    {
        assert(index >= 0 && static_cast<size_t>(index) < children_.size());
        return children_[index];
    }

    //------------------------------------------------------------------------------
    void ASTNode::SetType(shared_ptr<SymbolType> type)
    {
        assert(type != NULL);
        type_ = type;
    }

    //------------------------------------------------------------------------------
    shared_ptr<SymbolType> ASTNode::GetType() const
    {
        assert(type_ != NULL);
        return type_;
    }

    //------------------------------------------------------------------------------
    ASTNode::ASTNode(const Token &token)
        : token(token)
    {

    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeBinaryOperator::ASTNodeBinaryOperator(const Token& token, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right)
        : ASTNode(token)
    {
        assert(left != NULL && right != NULL);
//        assert(IsBinaryOperator(token));
        children_.push_back(left);
        children_.push_back(right);
    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeAssignment::ASTNodeAssignment(const Token& token, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right)
        : ASTNodeBinaryOperator(token, left, right)
    {
        if (!IsLValue(left))
        {
            ThrowInvalidTokenError(token, "left operand of assignment must be modifiable lvalue");
        }
    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeUnaryOperator::ASTNodeUnaryOperator(const Token& token)
        : ASTNode(token)
    {

    }

//------------------------------------------------------------------------------
    ASTNodeUnaryOperator::ASTNodeUnaryOperator(const Token& token, shared_ptr<ASTNode> node)
        : ASTNode(token)
    {
        assert(node != NULL);
        children_.push_back(node);
    }

//------------------------------------------------------------------------------
    void ASTNodeUnaryOperator::SetOperand(shared_ptr<ASTNode> node)
    {
        assert(children_.size() == 0);
        assert(node != NULL);
        children_.push_back(node);
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> ASTNodeUnaryOperator::GetOperand()
    {
        assert(children_.size() == 1);
        return children_[0];
    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeConditionalOperator::ASTNodeConditionalOperator(const Token& token,
        shared_ptr<ASTNode> condition, shared_ptr<ASTNode> thenExpression, shared_ptr<ASTNode> elseExpression)
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
    ASTNodeArraySubscript::ASTNodeArraySubscript(const Token& token, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right)
        : ASTNodeBinaryOperator(token, left, right)
    {
        assert(token == OP_LSQUARE);

    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeFunctionCall::ASTNodeFunctionCall(const Token& token, shared_ptr<ASTNode> caller)
        : ASTNode(token)
    {
        assert(caller != NULL);
        children_.push_back(caller);
    }

//------------------------------------------------------------------------------
    void ASTNodeFunctionCall::AddArgumentExpressionNode(shared_ptr<ASTNode> node)
    {
        assert(children_.size() > 0);
        assert(node != NULL);
        children_.push_back(node);
    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeStructureAccess::ASTNodeStructureAccess(const Token& token, shared_ptr<ASTNode> lhs, shared_ptr<ASTNode> rhs)
        : ASTNode(token)
    {
        assert(lhs != NULL);
        assert(rhs != NULL);
        children_.push_back(lhs);
        children_.push_back(rhs);
    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeTypeName::ASTNodeTypeName(shared_ptr<SymbolType> typeNameSymbol)
        : ASTNode(Token(TT_TYPE_NAME, typeNameSymbol->GetQualifiedName()))
        , typeSymbol_(typeNameSymbol)
    {

    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeCast::ASTNodeCast(shared_ptr<ASTNodeTypeName> left, shared_ptr<ASTNode> right)
        : ASTNodeBinaryOperator(Token(TT_CAST, "cast"), left, right)
    {

    }

//------------------------------------------------------------------------------
    bool IsLValue(shared_ptr<ASTNode> node)
    {
        assert(node != NULL);
        Token token = node->token;
        switch (token)
        {
            case TT_IDENTIFIER:
                return true;

            case OP_LSQUARE:
                return IsLValue(node->GetChild(0));

            default:
                return false;
        }
    }


} // namespace
