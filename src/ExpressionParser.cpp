#include "ExpressionParser.hpp"

#include <sstream>
#include <unordered_set>
#include <functional>
#include <queue>
#include <iostream>
#include <algorithm>
#include <cassert>

#include <boost/bind.hpp>

namespace Compiler
{
//------------------------------------------------------------------------------
    ASTNode::~ASTNode()
    {
        delete left_;
        delete right_;
    }

//------------------------------------------------------------------------------
    ASTNode::ASTNode(const ExpressionToken &token)
        : token(token)
    {

    }

//------------------------------------------------------------------------------
    ASTNode::ASTNode(const ExpressionToken &token, ASTNode *left, ASTNode *right)
        : left_(left)
        , right_(right)
        , token(token)
    {

    }

//------------------------------------------------------------------------------
    ExpressionParser::ExpressionParser()
    {
        parseCoroutine_ = boost::move(Coroutine(boost::bind(&ExpressionParser::ParseExpression_, this, _1), Token(TT_START_PARSE)));
    }

//------------------------------------------------------------------------------
    ASTNode* ExpressionParser::ParsePrimaryExpression_(Coroutine::caller_type& caller)
    {
        Token token = WaitForTokenReady_(caller);
        switch(token.type)
        {
            case TT_IDENTIFIER:
            case TT_LITERAL:
            {
                return new ASTNode(token);
                break;
            }

            case OP_LPAREN:
            {
                ASTNode* r = ParseExpression_(caller);

                token = WaitForTokenReady_(caller);
                if (token != OP_RPAREN)
                {
                    delete r;
                    std::stringstream ss;
                    ss << "closing parenthesis ')' expected at" << token.line << "-" << token.column;
                    throw std::logic_error(ss.str());
                }
                return r;
                break;
            }

            case TT_EOF:
            {
                FlushOutput_();
                break;
            }

            default:
            {
                FlushOutput_();
                ThrowInvalidTokenError_(token);
                break;
            }
        }
    }

//------------------------------------------------------------------------------
    ASTNode* ExpressionParser::ParseBinaryOperator_(Coroutine::caller_type& caller)
    {
        parseExpressionCallDepth_++;
        ASTNode* left = NULL;

        if (!nodeStack_.empty())
        {
            left = nodeStack_.back();
            nodeStack_.pop_back();
        }
        else
        {
            left = ParseUnaryExpression_(caller);
        }

        Token token = WaitForTokenReady_(caller);

        if (token == OP_PLUS
                || token == OP_MINUS)
        {
            while (token == OP_PLUS
                    || token == OP_MINUS)
            {
                left = new ASTNode(token, left, ParseBinaryOperator_(caller));
                token = WaitForTokenReady_(caller);
            }
        }

        tokenStack_.push_back(token);
        if (parseExpressionCallDepth_ > 1)
        {
            parseExpressionCallDepth_--;
            return left;
        }
        else
        {
            nodeStack_.push_back(left);
            if (token == TT_EOF)
            {
                FlushOutput_();
            }
            else
            {
                parseExpressionCallDepth_--;
                ParseBinaryOperator_(caller);
            }
        }

        return left;
    }

//    ASTNode* ExpressionParser::ParseTerm_(Coroutine::caller_type& caller)
//    {
//        ASTNode* left = ParseFactor_(caller);
//        Token token = WaitForTokenReady_(caller);

//        while (token == OP_STAR
//                || token == OP_DIV)
//        {
//            left = new ASTNode(token, left, ParseFactor_(caller));
//            token = WaitForTokenReady_(caller);
//        }
//        tokenStack_.push_back(token);
//        return left;
//    }

//------------------------------------------------------------------------------
    ASTNode*ExpressionParser::ParseExpression_(Coroutine::caller_type& caller)
    {
        ASTNode* node = ParseAssignmentExpression_(caller);
        Token token = WaitForTokenReady_(caller);
        while (token == OP_COMMA)
        {
            node = new ASTNode(token, node, ParseAssignmentExpression_(caller));
        }
        if (token == TT_EOF)
        {
            FlushOutput_();
        }
        else
        {
            nodeStack_.push_back(token);
            ThrowInvalidTokenError_(token);
        }
    }

//------------------------------------------------------------------------------
    ASTNode* ExpressionParser::ParseAssignmentExpression_(Coroutine::caller_type& caller)
    {
        ASTNode* node = ParseUnaryExpression_(caller);
        Token token = WaitForTokenReady_(caller);
        if (IsBinaryOperator_(token))
        {
            // return node, so ParseBinOp will get it
            // return token(binop) as well
            tokenStack_.push_back(token);
            nodeStack_.push_back(node);
            return ParseConditionalExpression_(caller);
        }
        else if (IsAssignmentOperator_(token))
        {
            return new ASTNode(token, node, ParseAssignmentExpression_(caller));
        }
        else if (token == TT_EOF)
        {
            FlushOutput_();
        }
        else
        {
            tokenStack_.push_back(token);
//            ThrowInvalidTokenError_(token);
        }
    }

//------------------------------------------------------------------------------
    ASTNode* ExpressionParser::ParseUnaryExpression_(Coroutine::caller_type& caller)
    {
        Token token = WaitForTokenReady_(caller);

        if (IsUnaryOperator_(token)
            || token == OP_INC
            || token == OP_DEC)
        {
            return new ASTNode(token, ParsePostfixExpression_(caller), NULL);
        }
        else
        {
            tokenStack_.push_back(token);
            return ParsePostfixExpression_(caller);
        }
    }

//------------------------------------------------------------------------------
    ASTNode* ExpressionParser::ParseConditionalExpression_(Coroutine::caller_type& caller)
    {
        ASTNode* node = ParseBinaryOperator_(caller);
        Token token = WaitForTokenReady_(caller);
        if (token == OP_QMARK)
        {
            node = new ASTNode(token, node, ParseExpression_(caller));
            token = WaitForTokenReady_(caller);
            if (token == OP_COLON)
            {
                return new ASTNode(token, node, ParseConditionalExpression_(caller));
            }
            else
            {
                // colon missing
                ThrowInvalidTokenError_(token);
            }
        }
        else
        {
            tokenStack_.push_back(token);
            return node;
        }
    }

//------------------------------------------------------------------------------
    ASTNode* ExpressionParser::ParsePostfixExpression_(Coroutine::caller_type& caller)
    {
        ASTNode* node = ParsePrimaryExpression_(caller);
        Token token = WaitForTokenReady_(caller);

        while (true)
        {
//            bool flag = true;
            switch (token.type)
            {
                case OP_LSQUARE:
                {
                    // postfix-expression '[' expression ']'
                    node = new ASTNode(token, node, ParseExpression_(caller));
                    token = WaitForTokenReady_(caller);
                    if (token != OP_RSQUARE)
                    {
                        // expecting ']'
                        ThrowInvalidTokenError_(token);
                    }
                    break;
                }

                case OP_LPAREN:
                {
                    // postfix-expression '(' {expression} ')' // argument-expression-list
                    token = WaitForTokenReady_(caller);
                    if (token != OP_RPAREN)
                    {
                        node = new ASTNode(token, node, ParseExpression_(caller));
                        token = WaitForTokenReady_(caller);
                        if (token != OP_RPAREN)
                        {
                            // expecting ')'
                            ThrowInvalidTokenError_(token);
                        }
                    }
                    break;
                }

                case OP_DOT:
                case OP_ARROW:
                {
                    // postfix-expression '.' identifier
                    // postfix-expression '->' identifier
                    Token identifierToken = WaitForTokenReady_(caller);
                    if (identifierToken == TT_IDENTIFIER)
                    {
                        node = new ASTNode(token, node, new ASTNode(identifierToken));
                    }
                    break;
                }

                case OP_INC:
                case OP_DEC:
                {
                    // postfix-expression '++'
                    // postfix-expression '--'
                    node = new ASTNode(token, node, ParsePostfixExpression_(caller));
                    break;
                }

                default:
                {
                    // primary-expression
                    tokenStack_.push_back(token);
                    return node;
                    break;
                }
            }
        }
    }

//------------------------------------------------------------------------------
    bool ExpressionParser::IsBinaryOperator_(const ExpressionParser::Token& token)
    {
        return token == OP_PLUS
               || token == OP_MINUS
               || token == OP_STAR
               || token == OP_DIV;
    }

//------------------------------------------------------------------------------
    bool ExpressionParser::IsAssignmentOperator_(const ExpressionParser::Token& token)
    {
        return token == OP_ASS
               || token == OP_STARASS
               || token == OP_BANDASS
               || token == OP_DIVASS;
    }

//------------------------------------------------------------------------------
    bool ExpressionParser::IsUnaryOperator_(const ExpressionParser::Token& token)
    {
        return token == OP_AMP
               || token == OP_STAR
               || token == OP_PLUS
               || token == OP_MINUS
               || token == OP_NE
               || token == OP_LNOT;
    }

//------------------------------------------------------------------------------
    void ExpressionParser::ThrowInvalidTokenError_(const ExpressionParser::Token &token)
    {
        std::stringstream ss;
        ss << "unexpected token at " << token.line << "-" << token.column;
        throw std::logic_error(ss.str());
    }

//------------------------------------------------------------------------------
    void ExpressionParser::PrintAST_(ASTNode *root) const
    {
        struct PrintTreeNode
        {
            std::string text;
            PrintTreeNode* left = NULL;
            PrintTreeNode* right = NULL;
            unsigned depth = 0;
        };

        std::vector<int> offsetByDepth;
        offsetByDepth.resize(1);
        offsetByDepth[0] = 0;

        unsigned depth = 0;
        const int spacing = 2;

        std::function<unsigned(ASTNode*, PrintTreeNode*, int)> f =
                [&](ASTNode* next, PrintTreeNode* print, int estimate) -> unsigned
        {
            if (next != NULL)
            {
                if (offsetByDepth.size() < depth + 1)
                {
                    offsetByDepth.resize(depth + 1, 0);
                }
                int spaceCount = estimate - offsetByDepth[depth];
                if (spaceCount == 1)
                {
                    spaceCount = 2;
                }

                if (next->GetLeft() == NULL
                        && next->GetRight() == NULL)
                {
                    assert(print !=NULL);
                    print->text = std::string(spaceCount, ' ') + next->token.value;
                    print->depth = depth;
                    offsetByDepth[depth] += next->token.value.size() + spaceCount;
                    return next->token.value.size();
                }
                else
                {
                    print->left = new PrintTreeNode;
                    print->right = new PrintTreeNode;

                    depth++;
                    unsigned leftLength = f(next->GetLeft(), print->left, estimate);

                    int maxLeft = *std::max_element(offsetByDepth.begin() + depth, offsetByDepth.end());

                    unsigned rightLength = f(next->GetRight(), print->right, maxLeft + spacing);
                    depth--;

                    assert(print != NULL);

                    print->text = std::string(spaceCount, ' ')
                                  + next->token.value
                                  + std::string(maxLeft + spacing - estimate - (next->token.value.size() - 1), '-');
                    print->depth = depth;
                    offsetByDepth[depth] += next->token.value.size() + maxLeft + spacing - estimate + spaceCount - (next->token.value.size() - 1);

                    return maxLeft;
                }
            }
            else
            {
                return 0u;
            }
        };

        PrintTreeNode printRoot;
        f(root, &printRoot, 0);

        std::queue<PrintTreeNode*> queue;
        queue.push(&printRoot);
        vector<PrintTreeNode*> line;
        depth= 0;
        while (!queue.empty())
        {
            PrintTreeNode* next = queue.front();
            queue.pop();

            if (next->left != NULL)
            {
                queue.push(next->left);
            }

            if (next->right != NULL)
            {
                queue.push(next->right);
            }

            std::cout << next->text;
            line.push_back(next);
            if (!queue.empty() &&
                    depth != queue.front()->depth)
            {
                std::cout << std::endl;
                depth = queue.front()->depth;
                for (auto node : line)
                {
                    if (node->left == NULL
                            || node->right == NULL)
                    {
                        std::cout << std::string(node->text.size(), ' ');
                        continue;
                    }
                    bool flag = false;
                    for (int i = 0; i < node->text.size(); i++)
                    {
                        if (!flag)
                        {
                            if (node->text[i] != ' ')
                            {
                                std::cout << '|';
                                flag = true;
                            }
                            else
                            {
                                std::cout << ' ';
                            }
                        }
                        else
                        {
                            if (i == node->text.size() - 1)
                            {
                                std::cout << '|';
                            }
                            else
                            {
                                std::cout << ' ';
                            }
                        }

                    }
                }
                std::cout << std::endl;
                line.clear();
            }
        }
    }

//------------------------------------------------------------------------------
    void ExpressionParser::ResumeParse_(const ExpressionParser::Token& token)
    {
        parseCoroutine_(token);
    }

//------------------------------------------------------------------------------
    ExpressionParser::Token ExpressionParser::WaitForTokenReady_(Coroutine::caller_type& caller)
    {
        Token token;
        if (tokenStack_.empty())
        {
            caller();
            token = caller.get();
        }
        else
        {
            token = tokenStack_.back();
            tokenStack_.pop_back();
        }
        return token;
    }

//------------------------------------------------------------------------------
    void ExpressionParser::FlushOutput_()
    {
        while (!nodeStack_.empty())
        {
            PrintAST_(nodeStack_.back());
            nodeStack_.pop_back();
            std::cout << std::endl << std::endl;
        }
    }

//------------------------------------------------------------------------------
    void ExpressionParser::EmitInvalid(const string &source, const int line, const int column)
    {
        ThrowInvalidTokenError_(Token(TT_INVALID, source, line, column));
    }

//------------------------------------------------------------------------------
    void ExpressionParser::EmitKeyword(const string &source, ETokenType token_type, const int line, const int column)
    {
        ThrowInvalidTokenError_(Token(token_type, source, line, column));
    }

//------------------------------------------------------------------------------
    void ExpressionParser::EmitPunctuation(const string &source, ETokenType token_type, const int line, const int column)
    {
        Token token(token_type, source, line, column);

        if (TokenTypeToPrecedence.find(token_type) != TokenTypeToPrecedence.end())
        {
            ResumeParse_(token);
        }
        else
        {
            ThrowInvalidTokenError_(token);
        }
    }

//------------------------------------------------------------------------------
    void ExpressionParser::EmitIdentifier(const string &source, const int line, const int column)
    {
        Token token(TT_IDENTIFIER, source, line, column);
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void ExpressionParser::EmitLiteral(const string &source,
                                             EFundamentalType type,
                                             const void *data, size_t nbytes,
                                             const int line, const int column)
    {
        Token token(TT_LITERAL, source, line, column);
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void ExpressionParser::EmitLiteralArray(const string &source,
                                                  size_t num_elements,
                                                  EFundamentalType type,
                                                  const void *data, size_t nbytes,
                                                  const int line, const int column)
    {
        Token token(TT_LITERAL_ARRAY, source, line, column);
        ThrowInvalidTokenError_(token);
    }

//------------------------------------------------------------------------------
    void ExpressionParser::EmitEof(const int line, const int column)
    {
        Token token(TT_EOF, "", line, column);
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    ExpressionToken::ExpressionToken(const ETokenType &type,
                                                 const std::string &value,
                                                 const unsigned &line,
                                                 const unsigned &column)
        : type(type)
        , value(value)
        , line(line)
        , column(column)
    {

    }

    ExpressionToken::ExpressionToken(const ETokenType& type)
        :type(type)
    {

    }

} // namespace Compiler
