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
        if (!IfModifiableLValue(left))
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
        CheckTypes_();
    }

//------------------------------------------------------------------------------
    void ASTNodeUnaryOperator::SetOperand(shared_ptr<ASTNode> node)
    {
        assert(children_.size() == 0);
        assert(node != NULL);
        children_.push_back(node);
        CheckTypes_();
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> ASTNodeUnaryOperator::GetOperand()
    {
        assert(children_.size() == 1);
        return children_[0];
    }

//------------------------------------------------------------------------------
    void ASTNodeUnaryOperator::CheckTypes_()
    {
        assert(children_.size() == 1);

        shared_ptr<ASTNode> node = children_[0];
        shared_ptr<SymbolType> sym = node->GetTypeSym();

        switch (token.type)
        {
            case OP_DEC:
            case OP_INC:
                if (!(IfModifiableLValue(node)
                      && IfScalar(sym)))
                {
                    ThrowInvalidTokenError(token, "operand must be scalar modifiable lvalue");
                }
                SetTypeSym(sym);
                break;

            case OP_STAR:
                if (!(sym->GetType() == ESymbolType::TYPE_POINTER
                      && sym->GetType() == ESymbolType::TYPE_ARRAY))
                {
                    ThrowInvalidTokenError(token, "operand must have pointer or array type");
                }
                SetTypeSym(GetRefSymbol(sym));
                break;

            case OP_AMP:
                if (!(IfLValue(node)))
                {
                    ThrowInvalidTokenError(token, "operand must be lvalue");
                }
                SetTypeSym(make_shared<SymbolPointer>(sym));
                break;

            case OP_PLUS:
            case OP_MINUS:
                if (!IfArithmetic(sym))
                {
                    ThrowInvalidTokenError(token, "operand must have arithmetic type");
                }
                // TODO: look more into type here
                SetTypeSym(sym);
                break;

            case OP_COMPL:
                if (!IfInteger(sym))
                {
                    ThrowInvalidTokenError(token, "operand must have integer type");
                }
                SetTypeSym(sym);
                break;

            case OP_LNOT:
                if (!IfScalar(sym))
                {
                    ThrowInvalidTokenError(token, "operand must have scalar type");
                }
                SetTypeSym(sym);
                break;

            case KW_SIZEOF:
                // TODO: complete checks
                SetTypeSym(theParser->LookupType("int"));
                break;

            default:
                assert(false);
        }
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
        // TODO: completness check
    }

////////////////////////////////////////////////////////////////////////////////
    ASTNodeFunctionCall::ASTNodeFunctionCall(const Token& token, shared_ptr<ASTNode> caller, vector<shared_ptr<ASTNode> >& parameters)
        : ASTNode(token)
    {
        assert(caller != NULL);
        children_.push_back(caller);

        shared_ptr<SymbolType> symType = caller->GetTypeSym();

        if (symType->GetType() == ESymbolType::TYPE_POINTER)
        {
            symType = GetRefSymbol(symType);
        }

        if (symType->GetType() != ESymbolType::TYPE_FUNCTION)
        {
            ThrowInvalidTokenError(caller->token, "left of `(...)` operator must have designate a function or have a function pointer type");
        }

        shared_ptr<SymbolFunctionType> symFun = static_pointer_cast<SymbolFunctionType>(symType);
        shared_ptr<SymbolTableWithOrder> argsSymbols = symFun->GetSymbolTable();

        typeSym_ = symFun->GetRefSymbol();

        bool internal = caller->token.text == "print";

        if (argsSymbols->orderedVariables.size() != parameters.size()
            && !internal)
        {
            ThrowInvalidTokenError(caller->token, "invalid parameter count");
        }

        for (size_t i = 0; i < parameters.size(); i++)
        {
            if (!internal
                && !argsSymbols->orderedVariables[i]->IfTypeFits(parameters[i]->GetTypeSym()))
            {
                ThrowInvalidTokenError(caller->token, "actual parameter "
                  + std::to_string(i)
                  + ": " + parameters[i]->GetTypeSym()->GetQualifiedName()
                  + "doesn't fit formal parameter: "
                  + argsSymbols->orderedVariables[i]->GetQualifiedName());
            }
            children_.push_back(parameters[i]);
        }

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
        shared_ptr<SymbolType> typeSym = lhs->GetTypeSym();
        bool constant = false;

        if (typeSym->GetType() == ESymbolType::TYPE_CONST)
        {
            constant = true;
            typeSym = GetRefSymbol(typeSym);
        }

        switch (token.type)
        {
            case OP_DOT:
                if (typeSym->GetType() != ESymbolType::TYPE_STRUCT)
                {
                    ThrowInvalidTokenError(lhs->token, "left operand of `.` operator should have structure type");
                }
                symStruct = static_pointer_cast<SymbolStruct>(typeSym);
                break;

            case OP_ARROW:
                if (typeSym->GetType() != ESymbolType::TYPE_POINTER)
                {
                    ThrowInvalidTokenError(lhs->token, "left operand of `->` operator should have pointer to structure type");
                }
                else
                {
                    shared_ptr<SymbolType> typeRef = GetRefSymbol(typeSym);
                    // TODO: could it be const again?
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
        if (constant
            && typeSym_->GetType() != ESymbolType::TYPE_CONST)
        {
            typeSym_ = make_shared<SymbolConst>(typeSym);
        }
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
    bool IfModifiableLValue(shared_ptr<ASTNode> node)
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
                return node->GetChildCount() == 1;

            default:
                return false;
        }
    }

    bool IfLValue(shared_ptr<ASTNode> node)
    {
        assert(node != NULL);
        Token token = node->token;

        shared_ptr<SymbolType> sym = node->GetTypeSym();

        if (sym->GetType() == ESymbolType::TYPE_CONST)
        {
            sym = GetRefSymbol(sym);
            assert(sym->GetType() != ESymbolType::TYPE_CONST);
        }

        switch (token)
        {
            case OP_LSQUARE:
                //--
            case TT_IDENTIFIER:
                //--
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
                return node->GetChildCount() == 1;

            default:
                return false;
        }
    }


} // namespace
