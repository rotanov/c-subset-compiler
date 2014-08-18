#include "ASTNode.hpp"

#include <iostream>
#include <tuple>

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

//==============================================================================
int ASTNode::GetChildCount() const
{
  return children_.size();
}

//==============================================================================
shared_ptr<ASTNode> ASTNode::GetChild(const int index)
{
  assert(index >= 0 && static_cast<size_t>(index) < children_.size());
  return children_[index];
}

//==============================================================================
void ASTNode::SetTypeSym(shared_ptr<SymbolType> type)
{
  assert(type != NULL);
  typeSym_ = type;
}

//==============================================================================
shared_ptr<SymbolType> ASTNode::GetTypeSym() const
{
  assert(typeSym_ != NULL);
  return typeSym_;
}

//==============================================================================
bool ASTNode::IsConstExpr() const
{
  return token == TT_LITERAL_CHAR
      || token == TT_LITERAL_INT
      || token == TT_LITERAL_FLOAT;
}

//==============================================================================
int ASTNode::EvalToInt() const
{
  switch (token.type)
  {
  case TT_LITERAL_CHAR:
  case TT_LITERAL_INT:
    return token.intValue;

  case TT_LITERAL_FLOAT:
    return token.floatValue;

  default:
    ThrowInvalidTokenError(token, "can't be evaluated as integer constant expression");
  }

  return 0;
}

//==============================================================================
ASTNode::ASTNode(const Token &token)
  : token(token)
{

}

//==============================================================================
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
  assert(IsBinaryOperator(token));
  children_.push_back(left);
  children_.push_back(right);

  shared_ptr<SymbolType> symLeft = left->GetTypeSym();
  shared_ptr<SymbolType> symRight = right->GetTypeSym();

  switch (token.type)
  {
  case OP_STAR:
  case OP_DIV:
    if (!(IfArithmetic(symLeft)
          && IfArithmetic(symRight)))
    {
      ThrowInvalidTokenError(token, "both operands must be of an arithmetic type");
    }
    SetTypeSym(CalcCommonArithmeticType(symLeft, symRight));
    break;

  case OP_MOD:
    if (!(IfInteger(symLeft)
          && IfInteger(symRight)))
    {
      ThrowInvalidTokenError(token, "both operands must be of an integer type");
    }
    SetTypeSym(CalcCommonArithmeticType(symLeft, symRight));
    break;

    // rather complicated checks. see 6.5.6 of n1570
  case OP_PLUS:
  case OP_MINUS:
  {
    // correct if both operands are arithmetic or one is pointer to a
    // complete type and other is integer

    shared_ptr<SymbolType> symPtr = NULL;
    shared_ptr<SymbolType> symInt = NULL;

    std::tie(symInt, symPtr) = IfOfType(symLeft, ESymbolType::TYPE_POINTER) ? std::make_tuple(symRight, symLeft) :
                                                                              IfOfType(symRight, ESymbolType::TYPE_POINTER) ? std::make_tuple(symLeft, symRight) :
                                                                                                                              std::make_tuple(nullptr, nullptr);

    bool wrongTypes = false;

    if (symInt == nullptr)
    {
      if (IfArithmetic(symLeft)
          && IfArithmetic(symRight))
      {
        SetTypeSym(CalcCommonArithmeticType(symLeft, symRight));
      }
      else
      {
        if (token == OP_MINUS)
        {
          if (IfOfType(symLeft, ESymbolType::TYPE_POINTER)
              && IfOfType(symRight, ESymbolType::TYPE_POINTER)
              && symLeft->IfTypeFits(symRight))
            // TODO: look into case when one pointer is const and other is not
            // TODO: completness
          {
            SetTypeSym(make_shared<SymbolInt>());
          }
          else
          {
            ThrowInvalidTokenError(token, "left and right operands of `-` should be both arithmetic, or both pointers to compatible complete types or left is pointer to complete type and right is of integer type");
          }
        }
        else
        {
          wrongTypes = true;
        }
      }
    }
    else if (!IfOfType(symPtr, ESymbolType::TYPE_POINTER))
      // TODO: check completness
    {
      wrongTypes = true;
    }
    else
    {
      if (token == OP_MINUS
          && symInt != symRight)
      {
        ThrowInvalidTokenError(token, "left operand of `-` should be of pointer type and right one should be of integer type");
      }
      SetTypeSym(symPtr);
    }

    if (wrongTypes)
    {
      ThrowInvalidTokenError(token, "both operands of `+` should be arithmetic or one sholud be pointer to complete object type and other should be integer");
    }

    break;
  }

  case OP_LSHIFT:
  case OP_RSHIFT:
  {
    if (!(IfInteger(symLeft)
          && IfInteger(symRight)))
    {
      ThrowInvalidTokenError(token, "both operands of either `>>` or `<<` should be of integer type");
    }
    SetTypeSym(CalcCommonArithmeticType(symLeft, symRight));
    // note:  If the value of the right operand is negative or is
    // greater than or equal to the width of the promoted left operand, the behavior is undefined.
    break;
  }

  case OP_GE:
  case OP_LE:
  case OP_GT:
  case OP_LT:
    break;

  case OP_EQ:
  case OP_NE:
    break;

  case OP_AMP:
  case OP_XOR:
  case OP_BOR:
  {
    if (!(IfInteger(symLeft)
          && IfInteger(symRight)))
    {
      ThrowInvalidTokenError(token, "both operand should be of integer type");
    }
    SetTypeSym(make_shared<SymbolInt>());
    break;
  }

  case OP_LAND:
  case OP_LOR:
  {
    if (!(IfScalar(symLeft)
          && IfScalar(symRight)))
    {
      ThrowInvalidTokenError(token, "both operands should be of scalar type");
    }
    SetTypeSym(make_shared<SymbolInt>());
    break;
  }

  default:
    assert(false);
  }
}

////////////////////////////////////////////////////////////////////////////////
bool ASTNodeBinaryOperator::IsConstExpr() const
{
  return token != OP_COMMA
      && children_[0]->IsConstExpr()
      && children_[1]->IsConstExpr();
}

//==============================================================================
int ASTNodeBinaryOperator::EvalToInt() const
{
  int lhs  = children_[0]->EvalToInt();
  int rhs = children_[1]->EvalToInt();

  switch (token.type)
  {
  case OP_PLUS:
    return lhs + rhs;
  case OP_MINUS:
    return lhs - rhs;
  case OP_STAR:
    return lhs * rhs;
  case OP_DIV:
    return lhs / rhs;
  case OP_MOD:
    return lhs % rhs;
  case OP_LSHIFT:
    return lhs << rhs;
  case OP_RSHIFT:
    return lhs >> rhs;
  case OP_LT:
    return lhs < rhs;
  case OP_GT:
    return lhs > rhs;
  case OP_LE:
    return lhs <= rhs;
  case OP_GE:
    return lhs >= rhs;
  case OP_EQ:
    return lhs == rhs;
  case OP_NE:
    return lhs != rhs;
  case OP_AMP:
    return lhs & rhs;
  case OP_XOR:
    return lhs ^ rhs;
  case OP_BOR:
    return lhs | rhs;
  case OP_LAND:
    return lhs && rhs;
  case OP_LOR:
    return lhs || rhs;
  default:
    return ASTNode::EvalToInt();
  }
}

////////////////////////////////////////////////////////////////////////////////
ASTNodeAssignment::ASTNodeAssignment(const Token& token, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right)
  : ASTNode(token)
{
  assert(left != NULL && right != NULL);

  children_.push_back(left);
  children_.push_back(right);

  if (!IfModifiableLValue(left))
  {
    ThrowInvalidTokenError(token, "left operand of assignment must be modifiable lvalue");
  }

  switch (token.type)
  {
  case OP_ASS:
  {
    // simple assignment

    break;
  }
  }

  SetTypeSym(left->GetTypeSym());
}

//==============================================================================
bool ASTNodeAssignment::IsConstExpr() const
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////
ASTNodeUnaryOperator::ASTNodeUnaryOperator(const Token& token)
  : ASTNode(token)
{

}

//==============================================================================
ASTNodeUnaryOperator::ASTNodeUnaryOperator(const Token& token, shared_ptr<ASTNode> node)
  : ASTNode(token)
{
  assert(node != NULL);
  children_.push_back(node);
  CheckTypes_();
}

//==============================================================================
void ASTNodeUnaryOperator::SetOperand(shared_ptr<ASTNode> node)
{
  assert(children_.size() == 0);
  assert(node != NULL);
  children_.push_back(node);
  CheckTypes_();
}

//==============================================================================
shared_ptr<ASTNode> ASTNodeUnaryOperator::GetOperand()
{
  assert(children_.size() == 1);
  return children_[0];
}

//==============================================================================
bool ASTNodeUnaryOperator::IsConstExpr() const
{
  return token != OP_INC
      && token != OP_DEC
      && children_[0]->IsConstExpr();
}

//==============================================================================
int ASTNodeUnaryOperator::EvalToInt() const
{
  int rhs = children_[0]->EvalToInt();
  switch (token.type)
  {
  case OP_MINUS:
    return -rhs;
  case OP_PLUS:
    return rhs;
  default:
    return ASTNode::EvalToInt();
  }
}

//==============================================================================
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
    if (!(IfOfType(sym, ESymbolType::TYPE_POINTER)
          || IfOfType(sym, ESymbolType::TYPE_ARRAY)))
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

  // TODO: check for constraints, apply type, see 6.5.15
}

//==============================================================================
bool ASTNodeConditionalOperator::IsConstExpr() const
{
  if (!children_[0]->IsConstExpr())
  {
    return false;
  }
  int condition = children_[0]->EvalToInt();
  if (condition == 1)
  {
    return children_[1]->IsConstExpr();
  }
  else
  {
    return children_[2]->IsConstExpr();
  }
}

//==============================================================================
int ASTNodeConditionalOperator::EvalToInt() const
{
  int condition = children_[0]->EvalToInt();
  return condition ? children_[1]->EvalToInt() : children_[2]->EvalToInt();
  }

  ////////////////////////////////////////////////////////////////////////////////
  ASTNodeArraySubscript::ASTNodeArraySubscript(const Token& token, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right)
  : ASTNode(token)
  {
  assert(left != NULL && right != NULL);
  assert(token == OP_LSQUARE);

  children_.push_back(left);
  children_.push_back(right);

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
  if (!IfOfType(leftTypeSym, ESymbolType::TYPE_ARRAY)
      && !IfOfType(leftTypeSym, ESymbolType::TYPE_POINTER))
  {
    ThrowInvalidTokenError(token, "one of `[]` operands must be of either pointer or array type");
  }
  leftTypeSym = GetRefSymbol(leftTypeSym);
  if (constant)
  {
    leftTypeSym = make_shared<SymbolConst>(leftTypeSym);
  }
  SetTypeSym(leftTypeSym);
  // TODO: completness check
}

//==============================================================================
bool ASTNodeArraySubscript::IsConstExpr() const
{
  return false;
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
        && !GetRefSymbol(argsSymbols->orderedVariables[i])->IfTypeFits(parameters[i]->GetTypeSym()))
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

//==============================================================================
bool ASTNodeFunctionCall::IsConstExpr() const
{
  return false;
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
      if (typeRef->GetType() == ESymbolType::TYPE_CONST)
      {
        constant = true;
        typeRef = GetRefSymbol(typeRef);
      }
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

//==============================================================================
bool ASTNodeStructureAccess::IsConstExpr() const
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////
ASTNodeTypeName::ASTNodeTypeName(shared_ptr<SymbolType> typeNameSymbol)
  : ASTNode(Token(TT_TYPE_NAME, typeNameSymbol->GetQualifiedName()))
  , typeSymbol_(typeNameSymbol)
{

}

//==============================================================================
bool ASTNodeTypeName::IsConstExpr() const
{
  return false;
}

////////////////////////////////////////////////////////////////////////////////
ASTNodeCast::ASTNodeCast(shared_ptr<ASTNodeTypeName> left, shared_ptr<ASTNode> right)
  : ASTNode(Token(TT_CAST, "cast"))
{
  assert(left != NULL && right != NULL);
  children_.push_back(left);
  children_.push_back(right);
  SetTypeSym(left->GetTypeSym());
}

//==============================================================================
bool ASTNodeCast::IsConstExpr() const
{
  return children_[1]->IsConstExpr()
      && IfArithmetic(children_[0]->GetTypeSym());
}

////////////////////////////////////////////////////////////////////////////////
ASTNodeCommaOperator::ASTNodeCommaOperator(const Token& token)
  : ASTNode(token)
{
  assert(token == OP_COMMA);
}

//==============================================================================
bool ASTNodeCommaOperator::IsConstExpr() const
{
  assert(children_.size() > 0);
  return children_.back()->IsConstExpr();
}

//==============================================================================
int ASTNodeCommaOperator::EvalToInt() const
{
  assert(children_.size() > 0);
  return children_.back()->EvalToInt();
}

//==============================================================================
void ASTNodeCommaOperator::PushOperand(const shared_ptr<ASTNode> node)
{
  assert(node != NULL);
  children_.push_back(node);
  SetTypeSym(node->GetTypeSym());
}

//==============================================================================
////////////////////////////////////////////////////////////////////////////////
bool IfModifiableLValue(shared_ptr<ASTNode> node)
{
  assert(node != NULL);
  Token token = node->token;

  auto type = node->GetTypeSym()->GetType();

  if (IfConst(node->GetTypeSym()))
  {
    return false;
  }

  switch (token)
  {
  case TT_IDENTIFIER:
  case OP_LSQUARE:
  case OP_DOT:
  case OP_ARROW:
    return type != ESymbolType::TYPE_FUNCTION
        && type != ESymbolType::TYPE_ARRAY;

  case OP_STAR:
    return node->GetChildCount() == 1
        && type != ESymbolType::TYPE_ARRAY;

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

  auto type = sym->GetType();

  switch (token)
  {
  case OP_LSQUARE:
  case TT_IDENTIFIER:
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
