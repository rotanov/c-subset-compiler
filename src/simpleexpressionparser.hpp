#pragma once

#include <string>
#include <vector>

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

    class ASTNode
    {
    public:
        typedef SimpleExpressionToken Token;

        Token token;

        ASTNode(const SimpleExpressionToken& token);
        ASTNode(const SimpleExpressionToken& token, ASTNode* left, ASTNode* right);
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
        ASTNode* left_;
        ASTNode* right_;
    };

    class SimpleExpressionParser : public ITokenStream
    {
        typedef SimpleExpressionToken Token;
    private:
        std::vector<Token> tokens_;
        size_t pos_;

        ASTNode* ParseTerm_();
        ASTNode* ParseFactor_();
        ASTNode* ParseExpression_();
        const Token& GetNextToken_();
        void ThrowInvalidTokenError_(const Token& token);
        void PrintAST_(ASTNode* root) const;

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

        virtual void EmitEof();

    };

} // namespace Compiler
