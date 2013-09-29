#pragma once

#include <chrono>
#include <string>
#include <thread>
#include <future>
#include <iostream>
#include <atomic>
#include <queue>

#include "ITokenStream.hpp"

namespace Compiler
{
    class ASTNode
    {
    public:
        ETokenType type;
        std::string value;

        ASTNode()
            : left_(NULL)
            , right_(NULL)
        {

        }

        ASTNode(ASTNode* left, ASTNode* right)
            : left_(left)
            , right_(right)
        {

        }

    private:
        ASTNode* left_;
        ASTNode* right_;
    };

    class SimpleExpressionParser : public ITokenStream
    {
    private:
        std::atomic_bool tokenReady_;
        std::queue<ETokenType> tokenTypeQueue_;
        std::queue<std::string> valueQueue_;

    public:
        SimpleExpressionParser()
            : tokenReady_(false)
        {

        }


        ASTNode* ParseExpression()
        {
            ASTNode* left = ParseTerm();
            WaitForTokenReady();
            ETokenType tokenType = tokenTypeQueue_.front();
            tokenTypeQueue_.pop();
            string value = valueQueue_.front();
            valueQueue_.pop();
            tokenReady_ = false;
            if (tokenType == OP_PLUS
                    || tokenType == OP_MINUS)
            {
                ASTNode* r = new ASTNode(left, ParseExpression());
                r->type = tokenType;
                r->value = value;
                return r;
            }
            else if (tokenType == TT_EOF)
            {
                return left;
            }
            else
            {
                throw std::logic_error("bad shit");
            }
        }

        ASTNode* ParseTerm()
        {
            ASTNode* left = ParseFactor();
            WaitForTokenReady();
            ETokenType tokenType = tokenTypeQueue_.front();
            tokenTypeQueue_.pop();
            string value = valueQueue_.front();
            valueQueue_.pop();
            tokenReady_ = false;
            if (tokenType == OP_STAR
                    || tokenType == OP_DIV)
            {
                ASTNode* r = new ASTNode(left, ParseTerm());
                r->type = tokenType;
                r->value = value;
                return r;
            }
            else if (tokenType == TT_EOF)
            {
                return left;
            }
            else
            {
                throw std::logic_error("bad shit");
            }
        }

        ASTNode* ParseFactor()
        {
            // dup code
            WaitForTokenReady();
            ETokenType tokenType = tokenTypeQueue_.front();
            tokenTypeQueue_.pop();
            string value = valueQueue_.front();
            valueQueue_.pop();
            tokenReady_ = false;

            if (tokenType == TT_LITERAL
                    || tokenType == TT_IDENTIFIER)
            {
                ASTNode* r = new ASTNode();
                r->type = tokenType;
                r->value = value;
                return r;
            }
            else if (tokenType == OP_LBRACE)
            {
                ASTNode* r = ParseExpression();

                WaitForTokenReady();
                tokenType = tokenTypeQueue_.front();
                tokenTypeQueue_.pop();
                value = valueQueue_.front();
                valueQueue_.pop();
                tokenReady_ = false;

                if (tokenType != OP_RBRACE)
                {
                    throw std::logic_error("missing )");
                }
                return r;
            }
            else if (tokenType == TT_EOF)
            {
                //return left;
                throw std::logic_error("empty expression");
            }
            else
            {
                throw std::logic_error("bad shit");
            }
        }

        void WaitForTokenReady()
        {
            while (!tokenReady_)
            {
                std::this_thread::yield();
//                 std::chrono::seconds one_second(1);
//                 std::this_thread::sleep_for (one_second);


            }
        }

        virtual void EmitInvalid(const string& source, const int line,
                                 const int column)
        {

        }

        virtual void EmitKeyword(const string& source, ETokenType token_type,
                                 const int line, const int column)
        {

        }

        virtual void EmitPunctuation(const string& source, ETokenType token_type,
                                     const int line, const int column)
        {
            if (token_type == OP_PLUS
                    || token_type == OP_MINUS
                    || token_type == OP_STAR
                    || token_type == OP_DIV)
            {
                tokenTypeQueue_.push(token_type);
                valueQueue_.push(source);
                tokenReady_ = true;
                std::this_thread::yield();
            }
        }

        virtual void EmitIdentifier(const string& source, const int line,
                                    const int column)
        {
            tokenTypeQueue_.push(TT_IDENTIFIER);
            valueQueue_.push(source);
            tokenReady_ = true;
            std::this_thread::yield();
        }

        virtual void EmitLiteral(const string& source, EFundamentalType type,
                                 const void* data, size_t nbytes, const int line,
                                 const int column)
        {
            tokenTypeQueue_.push(TT_LITERAL);
            valueQueue_.push(source);
            tokenReady_ = true;
            std::this_thread::yield();
        }

        virtual void EmitLiteralArray(const string& source, size_t num_elements,
                                      EFundamentalType type, const void* data,
                                      size_t nbytes, const int line,
                                      const int column)
        {

        }

        virtual void EmitEof()
        {
            tokenTypeQueue_.push(TT_EOF);
            valueQueue_.push("");
            tokenReady_ = true;
            std::this_thread::yield();
        }

    };

} // namespace Compiler
