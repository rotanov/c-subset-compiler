#pragma once

#include <string>
#include <vector>
#include <queue>

#include <boost/coroutine/all.hpp>

#include "ITokenStream.hpp"
#include "Token.hpp"
#include "ASTNode.hpp"
#include "SymbolTable.hpp"

namespace Compiler
{
    class Parser : public ITokenStream
    {
        typedef boost::coroutines::coroutine<void(const Token&)> Coroutine;

    private:
        std::vector<Token> tokenStack_;
        std::vector<ASTNode*> nodeStack_;
        Coroutine parseCoroutine_;

        ASTNode* ParseTopLevelExpression_(Coroutine::caller_type& caller);
        ASTNode* ParsePrimaryExpression_(Coroutine::caller_type& caller);
        ASTNode* ParseBinaryOperator_(Coroutine::caller_type& caller, int priority);
        ASTNode* ParseExpression_(Coroutine::caller_type& caller);
        ASTNode* ParseAssignmentExpression_(Coroutine::caller_type& caller);
        ASTNode* ParseUnaryExpression_(Coroutine::caller_type& caller);
        ASTNode* ParseConditionalExpression_(Coroutine::caller_type& caller);
        ASTNode* ParsePostfixExpression_(Coroutine::caller_type& caller);

        void ThrowInvalidTokenError_(const Token& token, const std::string& descriptionText = "");
        void ResumeParse_(const Token& token);
        Token WaitForTokenReady_(Coroutine::caller_type& caller);

        void FlushOutput_();

    public:
        Parser();

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
