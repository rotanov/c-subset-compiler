#pragma once

#include <string>
#include <vector>
#include <queue>

#include "ITokenStream.hpp"

namespace Compiler
{
    struct ExpressionToken
    {
        ETokenType type;
        std::string value;
        unsigned line;
        unsigned column;

        ExpressionToken(const ETokenType& type, const std::string& value,
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

    private:
        enum EParsingState
        {
            PS_UNARY_EXPRESSION,
            PS_BINARY_EXPRESSION,
        };
        std::vector<EParsingState> stateStack_;
        std::vector<Token> tokenStack_;
        std::vector<ASTNode*> nodeStack_;

        void ThrowInvalidTokenError_(const Token& token);
        void PrintAST_(ASTNode* root) const;

        // Shunting Yard algorithm
        void PushToken_(const Token& token);
        void FlushOutput_();
        // TODO: rename to verbal
        void StackTopToNode_();

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
