#include "SimpleExpressionParser.hpp"

#include <sstream>
#include <unordered_set>
#include <functional>
#include <queue>
#include <iostream>
#include <algorithm>
#include <cassert>

namespace Compiler
{
//------------------------------------------------------------------------------
    ASTNode::~ASTNode()
    {
        delete left_;
        delete right_;
    }

//------------------------------------------------------------------------------
    ASTNode::ASTNode(const SimpleExpressionToken &token)
        : token(token)
    {

    }

//------------------------------------------------------------------------------
    ASTNode::ASTNode(const SimpleExpressionToken &token, ASTNode *left, ASTNode *right)
        : left_(left)
        , right_(right)
        , token(token)
    {

    }

//------------------------------------------------------------------------------
    SimpleExpressionParser::SimpleExpressionParser()
    {

    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::ThrowInvalidTokenError_(const SimpleExpressionParser::Token &token)
    {
        std::stringstream ss;
        ss << "unexpected token at " << token.line << "-" << token.column;
        throw std::logic_error(ss.str());
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::PrintAST_(ASTNode *root) const
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
                                  + std::string(maxLeft + spacing - estimate, '-');
                    print->depth = depth;
                    offsetByDepth[depth] += next->token.value.size() + maxLeft + spacing - estimate + spaceCount;

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
    void SimpleExpressionParser::StackTopToNode_()
    {
        Token topToken = tokenStack_.back();
        tokenStack_.pop_back();
        ASTNode* right = tokenQueue_.back();
        tokenQueue_.pop_back();
        ASTNode* left = tokenQueue_.back();
        tokenQueue_.pop_back();
        ASTNode* node = new ASTNode(topToken, left, right);
        tokenQueue_.push_back(node);
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::PushToken_(const Token& token)
    {
        auto GetPrecedence = [](const Token tt) -> int
        {
            if (TokenTypeToPrecedence.find(tt.type) != TokenTypeToPrecedence.end())
            {
                return TokenTypeToPrecedence.at(tt.type);
            }
            else
            {
                throw "pizdec";
            }
        };

        switch (token.type)
        {
        case TT_LITERAL:
        case TT_IDENTIFIER:
        {
            tokenQueue_.push_back(new ASTNode(token));
        }
            break;

        case OP_STAR:
        case OP_DIV:
        case OP_PLUS:
        case OP_MINUS:
        {
            while (!tokenStack_.empty()
                   && ((tokenStack_.back().type == OP_STAR
                        || tokenStack_.back().type == OP_DIV
                        || tokenStack_.back().type == OP_PLUS
                        || tokenStack_.back().type == OP_MINUS)
                   && ((TokenTypeToRightAssociativity.count(token.type) == 0
                        && GetPrecedence(token) == GetPrecedence(tokenStack_.back()))
                       || (GetPrecedence(token) > GetPrecedence(tokenStack_.back())))))
            {
                StackTopToNode_();
            }
            tokenStack_.push_back(token);
        }
            break;

        case OP_LPAREN:
        {
            tokenStack_.push_back(token);
        }
            break;

        case OP_RPAREN:
        {

            while (!tokenStack_.empty()
                   && tokenStack_.back().type != OP_LPAREN)
            {
                Token top = tokenStack_.back();

                StackTopToNode_();

                if (tokenStack_.empty())
                {
                    OutputQueue_();
                    std::stringstream ss;
                    ss << "closing parenthesis ')' expected at" << top.line << "-" << top.column;
                    throw std::logic_error(ss.str());
                }
            }
            tokenStack_.pop_back();

        }
            break;
        }
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::OutputQueue_()
    {
        while (!tokenStack_.empty())
        {
            StackTopToNode_();
        }

        while (!tokenQueue_.empty())
        {
            PrintAST_(tokenQueue_.back());
            tokenQueue_.pop_back();
            std::cout << std::endl << std::endl;
        }
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::EmitInvalid(const string &source, const int line, const int column)
    {
        ThrowInvalidTokenError_(Token(TT_INVALID, source, line, column));
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::EmitKeyword(const string &source, ETokenType token_type, const int line, const int column)
    {
        ThrowInvalidTokenError_(Token(token_type, source, line, column));
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::EmitPunctuation(const string &source, ETokenType token_type, const int line, const int column)
    {
        const std::unordered_set<int> validTokenTypes =
        {
            OP_PLUS,
            OP_MINUS,
            OP_STAR,
            OP_DIV,
            OP_LPAREN,
            OP_RPAREN,
        };

        Token token(token_type, source, line, column);

        if (validTokenTypes.find(token_type) != validTokenTypes.end())
        {
            PushToken_(token);
        }
        else
        {
            ThrowInvalidTokenError_(token);
        }
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::EmitIdentifier(const string &source, const int line, const int column)
    {
        Token token(TT_IDENTIFIER, source, line, column);
        PushToken_(token);
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::EmitLiteral(const string &source,
                                             EFundamentalType type,
                                             const void *data, size_t nbytes,
                                             const int line, const int column)
    {
        Token token(TT_LITERAL, source, line, column);
        PushToken_(token);
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::EmitLiteralArray(const string &source,
                                                  size_t num_elements,
                                                  EFundamentalType type,
                                                  const void *data, size_t nbytes,
                                                  const int line, const int column)
    {
        Token token(TT_LITERAL_ARRAY, source, line, column);
        ThrowInvalidTokenError_(token);
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::EmitEof(const int line, const int column)
    {
        Token token(TT_EOF, "", line, column);
        PushToken_(token);
        OutputQueue_();
    }

//------------------------------------------------------------------------------
    SimpleExpressionToken::SimpleExpressionToken(const ETokenType &type,
                                                 const std::string &value,
                                                 const unsigned &line,
                                                 const unsigned &column)
        : type(type)
        , value(value)
        , line(line)
        , column(column)
    {

    }

} // namespace Compiler
