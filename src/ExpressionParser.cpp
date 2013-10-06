#include "ExpressionParser.hpp"

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
        stateStack_.push_back(PS_UNARY_EXPRESSION);
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
    void ExpressionParser::StackTopToNode_()
    {
        if (nodeStack_.size() < 2)
        {
            if (nodeStack_.size() == 1
                && tokenStack_.back() == OP_LBRACE)
            {
                throw std::logic_error("parentheses mismatch");
            }
            throw std::logic_error("operator or operand expected");
        }
        Token topToken = tokenStack_.back();
        tokenStack_.pop_back();
        ASTNode* right = nodeStack_.back();
        nodeStack_.pop_back();
        ASTNode* left = nodeStack_.back();
        nodeStack_.pop_back();
        ASTNode* node = new ASTNode(topToken, left, right);
        nodeStack_.push_back(node);
    }

//------------------------------------------------------------------------------
    void ExpressionParser::PushToken_(const Token& token)
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

        EParsingState state = stateStack_.back();
        stateStack_.pop_back();

        switch (state)
        {
        case PS_UNARY_EXPRESSION:
        {
            if (token == TT_LITERAL
                || token == TT_IDENTIFIER
                || token == TT_LITERAL_ARRAY)
            {
                nodeStack_.push_back(new ASTNode(token));
                stateStack_.push_back(PS_BINARY_EXPRESSION);
            }
            else if (token == OP_LPAREN)
            {
                tokenStack_.push_back(token);
                stateStack_.push_back(PS_UNARY_EXPRESSION);
            }
            else if (UnaryOperators.count(token.type) != 0)
            {
                tokenStack_.push_back(token);
                stateStack_.push_back(PS_UNARY_EXPRESSION);
            }
            else if (token == TT_EOF)
            {
                FlushOutput_();
            }
            else
            {
                ThrowInvalidTokenError_(token);
            }
            break;
        }

        case PS_BINARY_EXPRESSION:
        {
            if (token == TT_LITERAL
                || token == TT_IDENTIFIER
                || token == OP_LPAREN)
            {
                FlushOutput_();
                stateStack_.push_back(PS_UNARY_EXPRESSION);
                PushToken_(token);
            }
            else if (TokenTypeToPrecedence.find(token.type) != TokenTypeToPrecedence.end())
            {
                while (!tokenStack_.empty()
                       && (TokenTypeToPrecedence.find(token.type) != TokenTypeToPrecedence.end()
                           && ((TokenTypeToRightAssociativity.count(token.type) == 0
                                && GetPrecedence(token) == GetPrecedence(tokenStack_.back()))
                               || (GetPrecedence(token) > GetPrecedence(tokenStack_.back())))))
                {
                    StackTopToNode_();
                }
                tokenStack_.push_back(token);
                stateStack_.push_back(PS_UNARY_EXPRESSION);
            }
            else if (token == OP_RPAREN)
            {
                while (!tokenStack_.empty()
                       && tokenStack_.back().type != OP_LPAREN)
                {
                    Token top = tokenStack_.back();
                    StackTopToNode_();
                }

                if (tokenStack_.empty())
                {
                    FlushOutput_();
                    std::stringstream ss;
                    ss << "unexpected parenthesis ')' at" << token.line << "-" << token.column;
                    throw std::logic_error(ss.str());
                }

                tokenStack_.pop_back();
                stateStack_.push_back(PS_BINARY_EXPRESSION);
            }
            else if (token == TT_EOF)
            {
                FlushOutput_();
                stateStack_.push_back(PS_UNARY_EXPRESSION);
            }
            else
            {
                ThrowInvalidTokenError_(token);
            }
            break;
        }

        }
    }

//------------------------------------------------------------------------------
    void ExpressionParser::FlushOutput_()
    {
        while (!tokenStack_.empty())
        {
            StackTopToNode_();
        }

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
            PushToken_(token);
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
        PushToken_(token);
    }

//------------------------------------------------------------------------------
    void ExpressionParser::EmitLiteral(const string &source,
                                             EFundamentalType type,
                                             const void *data, size_t nbytes,
                                             const int line, const int column)
    {
        Token token(TT_LITERAL, source, line, column);
        PushToken_(token);
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
        PushToken_(token);
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

} // namespace Compiler
