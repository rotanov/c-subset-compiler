#include "ASTNode.hpp"

#include <iostream>

#include "SymbolTable.hpp"
#include "Parser.hpp"

namespace Compiler
{
    // not sure if really need it
    extern Parser* theParser;
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
    void ASTNode::SetTypeSym(shared_ptr<SymbolType> type)
    {
        assert(type != NULL);
        typeSym_ = type;
    }

//------------------------------------------------------------------------------
    shared_ptr<SymbolType> ASTNode::GetTypeSym() const
    {
        assert(typeSym_ != NULL);
        return typeSym_;
    }

//------------------------------------------------------------------------------
    ASTNode::ASTNode(const Token &token)
        : token(token)
    {

    }

//------------------------------------------------------------------------------
    ASTNode::ASTNode(const Token& token, shared_ptr<SymbolType> type)
        : token(token)
        , typeSym_(type)
    {
        assert(type != NULL);
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
        if (!(IfArithmetic(left->GetTypeSym())
              || IfArithmetic(right->GetTypeSym())))
        {
            ThrowInvalidTokenError(token, "one of `[]` operands must be of arithmetic type");
        }
        if (!IfArithmetic(right->GetTypeSym()))
        {
            left = children_[1];
            right = children_[0];
            children_[0] = left;
            children_[1] = right;
        }
        bool constant = false;
        shared_ptr<SymbolType> leftTypeSym = left->GetTypeSym();
        if (leftTypeSym->GetType() == ESymbolType::TYPE_CONST)
        {
            constant = true;
            leftTypeSym = static_pointer_cast<SymbolConst>(leftTypeSym)->GetRefSymbol();
        }
        if (leftTypeSym->GetType() != ESymbolType::TYPE_ARRAY
            && leftTypeSym->GetType() != ESymbolType::TYPE_POINTER)
        {
            ThrowInvalidTokenError(token, "on of `[]` operands must be of either pointer or array type");
        }
        leftTypeSym = GetRefSymbol(leftTypeSym);
        if (constant)
        {
            leftTypeSym = make_shared<SymbolConst>(leftTypeSym);
        }
        SetTypeSym(leftTypeSym);
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

        assert(lhs->GetTypeSym() != NULL);
        shared_ptr<SymbolStruct> symStruct = NULL;
        // rhs is always identifier due to parser check above
        switch (token.type)
        {
            case OP_DOT:
                if (lhs->GetTypeSym()->GetType() != ESymbolType::TYPE_STRUCT)
                {
                    ThrowInvalidTokenError(lhs->token, "left operand of `.` operator should have structure type");
                }
                symStruct = static_pointer_cast<SymbolStruct>(lhs->GetTypeSym());
                break;

            case OP_ARROW:
                if (lhs->GetTypeSym()->GetType() != ESymbolType::TYPE_POINTER)
                {
                    ThrowInvalidTokenError(lhs->token, "left operand of `->` operator should have pointer to structure type");
                }
                else
                {
                    shared_ptr<SymbolType> typeRef = static_pointer_cast<SymbolTypeRef>(lhs->GetTypeSym())->GetRefSymbol();
                    if (typeRef->GetType() != ESymbolType::TYPE_STRUCT)
                    {
                        ThrowInvalidTokenError(lhs->token, "left operand of `->` operator should have pointer to structure type");
                    }
                    symStruct = static_pointer_cast<SymbolStruct>(typeRef);
                }
                break;

            default:
                assert(false);
                break;
        }
        shared_ptr<SymbolVariable> field = symStruct->GetSymbolTable()->LookupVariable(rhs->token.text);
        if (field == NULL)
        {
            ThrowInvalidTokenError(rhs->token, "field doesn't exist");
        }
        typeSym_ = field->GetRefSymbol();
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

        if (node->GetTypeSym()->GetType() == ESymbolType::TYPE_CONST)
        {
            return false;
        }

        switch (token)
        {
            case TT_IDENTIFIER:
                return node->GetTypeSym()->GetType() != ESymbolType::TYPE_FUNCTION;

            case OP_LSQUARE:
                return node->GetTypeSym()->GetType() != ESymbolType::TYPE_ARRAY;

            case OP_ASS:
            case OP_STARASS:
            case OP_DIVASS:
            case OP_MODASS:
            case OP_PLUSASS:
            case OP_MINUSASS:
            case OP_LSHIFTASS:
            case OP_RSHIFTASS:
            case OP_BANDASS:
            case OP_XORASS:
            case OP_BORASS:
                //--
            case OP_INC:
            case OP_DEC:
                //--
            case OP_DOT:
            case OP_ARROW:
                return true;

            case OP_STAR:
                return  node->GetChildCount() == 1;

            default:
                return false;
        }
    }


} // namespace
