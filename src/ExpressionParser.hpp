#pragma once

#include <string>
#include <vector>
#include <queue>

#include <boost/coroutine/all.hpp>

#include "ITokenStream.hpp"

namespace Compiler
{
    struct ExpressionToken
    {
        ETokenType type = TT_INVALID;
        std::string value = "";
        unsigned line = 0;
        unsigned column = 0;

        ExpressionToken(const ETokenType& type, const std::string& value,
                              const unsigned& line, const unsigned& column);

        ExpressionToken(const ETokenType &type);

        ExpressionToken() {}


        inline bool operator ==(const ETokenType& rhs) const
        {
            return type == rhs;
        }

        inline bool operator !=(const ETokenType& rhs) const
        {
            return !(*this == rhs);
        }
    };

    class ExpressionASTNode
    {
    public:
        typedef ExpressionToken Token;

        Token token;

        ExpressionASTNode(const ExpressionToken& token);
        ExpressionASTNode(const ExpressionToken& token, ExpressionASTNode* left, ExpressionASTNode* right);
        virtual ~ExpressionASTNode();

        ExpressionASTNode* GetLeft()
        {
            return left_;
        }

        ExpressionASTNode* GetRight()
        {
            return right_;
        }

        void SetLeft(ExpressionASTNode* left)
        {
            left_ = left;
        }

        void SetRight(ExpressionASTNode* right)
        {
            right_ = right;
        }


    private:
        ExpressionASTNode* left_{NULL};
        ExpressionASTNode* right_{NULL};
    };

    class ExpressionParser : public ITokenStream
    {
        typedef ExpressionToken Token;
        typedef boost::coroutines::coroutine<void(const Token&)> Coroutine;

    private:
        std::vector<Token> tokenStack_;
        std::vector<ExpressionASTNode*> nodeStack_;
        Coroutine parseCoroutine_;

        ExpressionASTNode* ParseTopLevelExpression_(Coroutine::caller_type& caller);
        ExpressionASTNode* ParsePrimaryExpression_(Coroutine::caller_type& caller);
        ExpressionASTNode* ParseBinaryOperator_(Coroutine::caller_type& caller, int priority);
        ExpressionASTNode* ParseExpression_(Coroutine::caller_type& caller);
        ExpressionASTNode* ParseAssignmentExpression_(Coroutine::caller_type& caller);
        ExpressionASTNode* ParseUnaryExpression_(Coroutine::caller_type& caller);
        ExpressionASTNode* ParseConditionalExpression_(Coroutine::caller_type& caller);
        ExpressionASTNode* ParsePostfixExpression_(Coroutine::caller_type& caller);

        void ThrowInvalidTokenError_(const Token& token, const std::string& descriptionText = "");
        void PrintAST_(ExpressionASTNode* root) const;
        void ResumeParse_(const Token& token);
        Token WaitForTokenReady_(Coroutine::caller_type& caller);

        void FlushOutput_();

    public:
        ExpressionParser();

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
