#include "Parser.hpp"

#include <sstream>
#include <unordered_set>
#include <functional>
#include <queue>
#include <iostream>
#include <algorithm>
#include <cassert>

#include <boost/bind.hpp>

#include "utils.hpp"

namespace Compiler
{
//------------------------------------------------------------------------------
    Parser::Parser()
    {
        parseCoroutine_ = boost::move(Coroutine(boost::bind(&Parser::ParseTopLevelExpression_, this, _1), Token()));
    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParseTopLevelExpression_(Coroutine::caller_type& caller)
    {
        while (true)
        {
            PrintAST_(ParseExpression_(caller));
            std::cout << std::endl << std::endl;

            if (tokenStack_.empty())
            {
                throw std::runtime_error("token stack empty");
            }
            else if (tokenStack_.back() == TT_EOF)
            {
                break;
            }

        };

        FlushOutput_();
    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParsePrimaryExpression_(Coroutine::caller_type& caller)
    {
        Token token = WaitForTokenReady_(caller);
        switch(token.type)
        {
            case TT_IDENTIFIER:
            case TT_LITERAL:
            case TT_LITERAL_ARRAY:
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
                    ThrowInvalidTokenError_(token, "')' expected");
                }
                return r;
                break;
            }

            default:
            {
                ThrowInvalidTokenError_(token);
                tokenStack_.push_back(token);
                return NULL;
                break;
            }
        }
    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParseBinaryOperator_(Coroutine::caller_type& caller, int priority)
    {
        ASTNode* left = NULL;

        if (nodeStack_.empty())
        {
            left = ParseUnaryExpression_(caller);
        }
        else
        {
            left = nodeStack_.back();
            nodeStack_.pop_back();
        }

        Token token = WaitForTokenReady_(caller);

        while (binaryOperatorTypeToPrecedence.find(token.type) != binaryOperatorTypeToPrecedence.end()
               && binaryOperatorTypeToPrecedence.at(token.type) >= priority)
        {
            left = new ASTNode(token, left, ParseBinaryOperator_(caller, binaryOperatorTypeToPrecedence.at(token.type) + 1));
            token = WaitForTokenReady_(caller);
        }

        tokenStack_.push_back(token);
        return left;
    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParseExpression_(Coroutine::caller_type& caller)
    {
        ASTNode* node = ParseAssignmentExpression_(caller);
        Token token = WaitForTokenReady_(caller);
        while (token == OP_COMMA)
        {
            node = new ASTNode(token, node, ParseAssignmentExpression_(caller));
            token = WaitForTokenReady_(caller);
        }

        tokenStack_.push_back(token);
        return node;
    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParseAssignmentExpression_(Coroutine::caller_type& caller)
    {
        ASTNode* node = ParseUnaryExpression_(caller);
        Token token = WaitForTokenReady_(caller);
        if (IsAssignmentOperator(token.type))
        {
            return new ASTNode(token, node, ParseAssignmentExpression_(caller));
        }
        else
        {
            // return node, so ParseBinOp will get it
            // return token(binop) as well
            tokenStack_.push_back(token);
            nodeStack_.push_back(node);
            return ParseConditionalExpression_(caller);
        }

    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParseUnaryExpression_(Coroutine::caller_type& caller)
    {
        Token token = WaitForTokenReady_(caller);

        if (IsUnaryOperator(token.type)
            || token == OP_INC
            || token == OP_DEC)
        {
            ASTNode* node = new ASTNode(token);
            ASTNode* root = node;
            token = WaitForTokenReady_(caller);
            while (IsUnaryOperator(token.type)
                   || token == OP_INC
                   || token == OP_DEC)
            {
                node->SetLeft(new ASTNode(token));
                node = node->GetLeft();
                token = WaitForTokenReady_(caller);
            }
            tokenStack_.push_back(token);
            node->SetLeft(ParsePostfixExpression_(caller));
            return root;
        }
        else
        {
            tokenStack_.push_back(token);
            return ParsePostfixExpression_(caller);
        }
    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParseConditionalExpression_(Coroutine::caller_type& caller)
    {
        ASTNode* node = ParseBinaryOperator_(caller, 0);
        Token token = WaitForTokenReady_(caller);
        if (token == OP_QMARK)
        {
            ASTNode* leftSubNode = ParseExpression_(caller);
            Token colonToken = WaitForTokenReady_(caller);
            if (colonToken == OP_COLON)
            {
                return new ASTNode(token, node, new ASTNode(colonToken, leftSubNode,
                    ParseConditionalExpression_(caller)));
            }
            else
            {
                ThrowInvalidTokenError_(colonToken, "':' expected");
            }
        }
        else
        {
            tokenStack_.push_back(token);
            return node;
        }
    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParsePostfixExpression_(Coroutine::caller_type& caller)
    {
        ASTNode* node = ParsePrimaryExpression_(caller);
        Token token = WaitForTokenReady_(caller);

        while (true)
        {
            switch (token.type)
            {
                case OP_LSQUARE:
                {
                    // postfix-expression '[' expression ']'
                    node = new ASTNode(token, node, ParseExpression_(caller));
                    token = WaitForTokenReady_(caller);
                    if (token != OP_RSQUARE)
                    {
                        ThrowInvalidTokenError_(token, "']' expected");
                    }
                    token = WaitForTokenReady_(caller);
                    break;
                }

                case OP_LPAREN:
                {
                    // postfix-expression '(' {expression} ')' // argument-expression-list
                    Token prevToken = token;
                    token = WaitForTokenReady_(caller);
                    if (token != OP_RPAREN)
                    {
                        tokenStack_.push_back(token);
                        node = new ASTNode(prevToken, node, ParseExpression_(caller));
                        token = WaitForTokenReady_(caller);
                        if (token != OP_RPAREN)
                        {
                            ThrowInvalidTokenError_(token, "')' expected");
                        }
                    }
                    else
                    {
                        node = new ASTNode(token, node, NULL);
                    }
                    token = WaitForTokenReady_(caller);
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
                    else
                    {
                        ThrowInvalidTokenError_(identifierToken, "identifier expected");
                    }
                    token = WaitForTokenReady_(caller);
                    break;
                }

                case OP_INC:
                case OP_DEC:
                {
                    // postfix-expression '++'
                    // postfix-expression '--'
                    while (token == OP_INC
                           || token == OP_DEC)
                    {
                        node = new ASTNode(token, node, NULL);
                        token = WaitForTokenReady_(caller);
                    }
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
    void Parser::ThrowInvalidTokenError_(const Token &token, const std::string& descriptionText)
    {
        FlushOutput_();
        std::stringstream ss;
        ss << "unexpected token " << TokenTypeToString(token.type) << " : \""
           << token.value << "\" at " << token.line << "-" << token.column;
        if (!descriptionText.empty())
        {
            ss << ", " << descriptionText;
        }
        throw std::logic_error(ss.str());
    }

//------------------------------------------------------------------------------
    void Parser::PrintAST_(ASTNode *root) const
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

        std::function<void(ASTNode*, PrintTreeNode*, int)> f =
                [&](ASTNode* next, PrintTreeNode* print, int estimate)
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
                }
                else if ((next->GetLeft() != NULL && next->GetRight() == NULL)
                         || (next->GetLeft() == NULL && next->GetRight() != NULL))
                {
                    assert(print != NULL);
                    ASTNode* validNode = next->GetLeft() != NULL ? next->GetLeft() : next->GetRight();
                    print->left = new PrintTreeNode;

                    depth++;
                    f(next->GetLeft(), print->left, estimate);

                    int maxLeft = *std::max_element(offsetByDepth.begin() + depth, offsetByDepth.end());

                    depth--;


                    print->text = std::string(spaceCount, ' ')
                                  + next->token.value;
                    print->depth = depth;
                    offsetByDepth[depth] += next->token.value.size() + spaceCount;
                }
                else
                {
                    print->left = new PrintTreeNode;
                    print->right = new PrintTreeNode;

                    depth++;
                    f(next->GetLeft(), print->left, estimate);
                    int maxLeft = *std::max_element(offsetByDepth.begin() + depth, offsetByDepth.end());
                    f(next->GetRight(), print->right, maxLeft + spacing);
                    depth--;
                    assert(print != NULL);
                    print->text = std::string(spaceCount, ' ')
                                  + next->token.value
                                  + std::string(maxLeft + spacing - estimate
                                                - (next->token.value.size() - 1), '-');
                    print->depth = depth;
                    offsetByDepth[depth] += next->token.value.size() + maxLeft
                                            + spacing - estimate + spaceCount - (next->token.value.size() - 1);
                }
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
                        && node->right == NULL)
                    {
                        std::cout << std::string(node->text.size(), ' ');
                        continue;
                    }

                    bool singleLeaf = node->left == NULL || node->right == NULL;
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
                                if (!singleLeaf)
                                {
                                    std::cout << '|';
                                }
                                else
                                {
                                    std::cout << ' ';
                                }
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
    void Parser::ResumeParse_(const Token& token)
    {
        parseCoroutine_(token);
    }

//------------------------------------------------------------------------------
    Token Parser::WaitForTokenReady_(Coroutine::caller_type& caller)
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
    void Parser::FlushOutput_()
    {
        for (auto& node : nodeStack_)
        {
            PrintAST_(node);
            std::cout << std::endl;
        }
    }

//------------------------------------------------------------------------------
    void Parser::EmitInvalid(const string &source, const int line, const int column)
    {
        Token token(TT_INVALID, source, line, column);
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void Parser::EmitKeyword(const string &source, ETokenType token_type, const int line, const int column)
    {
        Token token(token_type, source, line, column);
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void Parser::EmitPunctuation(const string &source, ETokenType token_type, const int line, const int column)
    {
        const unordered_set<ETokenType> disallowedPunctuation =
        {
            OP_DOTS,
            OP_LBRACE,
            OP_RBRACE,
            OP_SEMICOLON,
        };

        Token token(token_type, source, line, column);

        if (disallowedPunctuation.find(token_type) == disallowedPunctuation.end())
        {
            ResumeParse_(token);
        }
        else
        {
            ThrowInvalidTokenError_(token);
        }
    }

//------------------------------------------------------------------------------
    void Parser::EmitIdentifier(const string &source, const int line, const int column)
    {
        Token token(TT_IDENTIFIER, source, line, column);
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void Parser::EmitLiteral(const string &source,
                                             EFundamentalType type,
                                             const void *data, size_t nbytes,
                                             const int line, const int column)
    {
        Token token(TT_LITERAL, source, line, column);
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void Parser::EmitLiteralArray(const string &source,
                                                  size_t num_elements,
                                                  EFundamentalType type,
                                                  const void *data, size_t nbytes,
                                                  const int line, const int column)
    {
        Token token(TT_LITERAL_ARRAY, "\"" + source + "\"", line, column);
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void Parser::EmitEof(const int line, const int column)
    {
        Token token(TT_EOF, "", line, column);
        ResumeParse_(token);
    }

} // namespace Compiler
