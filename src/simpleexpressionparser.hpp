#pragma once

#include <string>
#include <vector>
#include <queue>

#include "ITokenStream.hpp"

namespace Compiler
{
struct SimpleExpressionToken
{
  ETokenType type;
  std::string value;
  unsigned line;
  unsigned column;

  SimpleExpressionToken(const ETokenType& type, const std::string& value,
                        const unsigned& line, const unsigned& column);

  inline bool operator ==(const ETokenType& rhs) const
  {
    return type == rhs;
  }

  inline bool operator !=(const ETokenType& rhs) const
  {
    return !(*this == rhs);
  }
};

class SimpleASTNode
{
public:
  typedef SimpleExpressionToken Token;

  Token token;

  SimpleASTNode(const SimpleExpressionToken& token);
  SimpleASTNode(const SimpleExpressionToken& token, SimpleASTNode* left, SimpleASTNode* right);
  virtual ~SimpleASTNode();

  SimpleASTNode* GetLeft()
  {
    return left_;
  }

  SimpleASTNode* GetRight()
  {
    return right_;
  }

private:
  SimpleASTNode* left_{NULL};
  SimpleASTNode* right_{NULL};
};

class SimpleExpressionParser : public ITokenStream
{
  typedef SimpleExpressionToken Token;

private:
  enum EParsingState
  {
    PS_OPERAND,
    PS_OPERATOR,
  };
  std::vector<EParsingState> stateStack_;
  std::vector<Token> tokenStack_;
  std::vector<SimpleASTNode*> nodeStack_;

  void ThrowInvalidTokenError_(const Token& token);
  void PrintAST_(SimpleASTNode* root) const;

  // Shunting Yard algorithm
  void PushToken_(const Token& token);
  void FlushOutput_();
  // TODO: rename to verbal
  void StackTopToNode_();

public:
  SimpleExpressionParser();

  virtual void EmitInvalid(const string& source, const int line,
                           const int column);

  virtual void EmitKeyword(const string& source, ETokenType token_type,
                           const int line, const int column);

  virtual void EmitPunctuation(const string& source, ETokenType token_type,
                               const int line, const int column);

  virtual void EmitIdentifier(const string& source, const int line,
                              const int column);

  virtual void EmitLiteral(const string& source, EFundamentalType type,
                           const void* data, size_t nbytes, const int line,
                           const int column);

  virtual void EmitLiteralArray(const string& source, size_t num_elements,
                                EFundamentalType type, const void* data,
                                size_t nbytes, const int line,
                                const int column);

  virtual void EmitEof(const int line, const int column);

};

} // namespace Compiler
