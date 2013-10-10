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

    class ASTNode
    {
    public:
        typedef ExpressionToken Token;

        Token token;

        ASTNode(const ExpressionToken& token);
        ASTNode(const ExpressionToken& token, ASTNode* left, ASTNode* right);
        virtual ~ASTNode();

        ASTNode* GetLeft()
        {
            return left_;
        }

        ASTNode* GetRight()
        {
            return right_;
        }

    private:
        ASTNode* left_ = NULL;
        ASTNode* right_ = NULL;
    };

    class ExpressionParser : public ITokenStream
    {
        typedef ExpressionToken Token;
        typedef boost::coroutines::coroutine<void(const Token&)> Coroutine;

    private:
        std::vector<Token> tokenStack_;
        std::vector<ASTNode*> nodeStack_;
        std::vector<ASTNode*> returnValues_;
        std::vector<ASTNode*> nodes_;
        Coroutine parseCoroutine_;
        int parseExpressionCallDepth_ = 0;

        ASTNode* ParseExpression_(Coroutine::caller_type& caller);
        ASTNode* ParseTerm_(Coroutine::caller_type& caller);
        ASTNode* ParseFactor_(Coroutine::caller_type& caller);

        void ThrowInvalidTokenError_(const Token& token);
        void PrintAST_(ASTNode* root) const;
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
