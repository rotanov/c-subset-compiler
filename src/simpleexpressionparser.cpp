#include "SimpleExpressionParser.hpp"

#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <functional>
#include <queue>
#include <iostream>
#include <algorithm>
#include <cassert>

#include "utils.hpp"

namespace Compiler
{
//------------------------------------------------------------------------------
    SimpleASTNode::~SimpleASTNode()
    {
        delete left_;
        delete right_;
    }

//------------------------------------------------------------------------------
    SimpleASTNode::SimpleASTNode(const SimpleExpressionToken &token)
        : token(token)
    {

    }

//------------------------------------------------------------------------------
    SimpleASTNode::SimpleASTNode(const SimpleExpressionToken &token, SimpleASTNode *left, SimpleASTNode *right)
        : left_(left)
        , right_(right)
        , token(token)
    {

    }

//------------------------------------------------------------------------------
    SimpleExpressionParser::SimpleExpressionParser()
    {
        stateStack_.push_back(PS_OPERAND);
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::ThrowInvalidTokenError_(const SimpleExpressionParser::Token &token)
    {
        std::stringstream ss;
        ss << "unexpected token at " << token.line << "-" << token.column;
        throw std::logic_error(ss.str());
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::PrintAST_(SimpleASTNode *root) const
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

        std::function<unsigned(SimpleASTNode*, PrintTreeNode*, int)> f =
                [&](SimpleASTNode* next, PrintTreeNode* print, int estimate) -> unsigned
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
                    int maxLeft = *std::max_element(offsetByDepth.begin() + depth, offsetByDepth.end());
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
                    for (size_t i = 0; i < node->text.size(); i++)
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
        SimpleASTNode* right = nodeStack_.back();
        nodeStack_.pop_back();
        SimpleASTNode* left = nodeStack_.back();
        nodeStack_.pop_back();
        SimpleASTNode* node = new SimpleASTNode(topToken, left, right);
        nodeStack_.push_back(node);
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::PushToken_(const Token& token)
    {
        auto GetPrecedence = [](const Token tt) -> int
        {
            if (binaryOperatorTypeToPrecedence.find(tt.type) != binaryOperatorTypeToPrecedence.end())
            {
                return binaryOperatorTypeToPrecedence.at(tt.type);
            }
            else
            {
                throw "ploho";
            }
        };

        EParsingState state = stateStack_.back();
        stateStack_.pop_back();

        switch (state)
        {
        case PS_OPERAND:
        {
            if (IsLiteral(token.type)
                || token == TT_IDENTIFIER)
            {
                nodeStack_.push_back(new SimpleASTNode(token));
                stateStack_.push_back(PS_OPERATOR);
            }
            else if (token == OP_LPAREN)
            {
                tokenStack_.push_back(token);
                stateStack_.push_back(PS_OPERAND);
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

        case PS_OPERATOR:
        {
            if (IsLiteral(token.type)
                || token == TT_IDENTIFIER
                || token == OP_LPAREN)
            {
                FlushOutput_();
                stateStack_.push_back(PS_OPERAND);
                PushToken_(token);
            }
            else if (token == OP_STAR
                     || token == OP_DIV
                     || token == OP_PLUS
                     || token == OP_MINUS)
            {
                while (!tokenStack_.empty()
                       && ((tokenStack_.back().type == OP_STAR
                            || tokenStack_.back().type == OP_DIV
                            || tokenStack_.back().type == OP_PLUS
                            || tokenStack_.back().type == OP_MINUS)
                           && ((tokenTypeToRightAssociativity.count(token.type) == 0
                            && GetPrecedence(token) == GetPrecedence(tokenStack_.back()))
                           || (GetPrecedence(token) < GetPrecedence(tokenStack_.back())))))
                {
                    StackTopToNode_();
                }
                tokenStack_.push_back(token);
                stateStack_.push_back(PS_OPERAND);
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
                stateStack_.push_back(PS_OPERATOR);
            }
            else if (token == TT_EOF)
            {
                FlushOutput_();
                stateStack_.push_back(PS_OPERAND);
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
    void SimpleExpressionParser::FlushOutput_()
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
        UNUSED(data);
        UNUSED(nbytes);

        std::unordered_map<EFundamentalType, ETokenType> ftToTtMap =
        {
            {FT_INT, TT_LITERAL_INT},
            {FT_FLOAT, TT_LITERAL_FLOAT},
            {FT_CHAR, TT_LITERAL_CHAR},
        };

        assert(ftToTtMap.find(type) != ftToTtMap.end());

        Token token(ftToTtMap[type], source, line, column);
        PushToken_(token);
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::EmitLiteralArray(const string &source,
                                                  size_t num_elements,
                                                  EFundamentalType type,
                                                  const void *data, size_t nbytes,
                                                  const int line, const int column)
    {
        UNUSED(data);
        UNUSED(nbytes);
        UNUSED(num_elements);

        assert(type == FT_CHAR);
        Token token(TT_LITERAL_CHAR_ARRAY, source, line, column);
        ThrowInvalidTokenError_(token);
    }

//------------------------------------------------------------------------------
    void SimpleExpressionParser::EmitEof(const int line, const int column)
    {
        Token token(TT_EOF, "", line, column);
        PushToken_(token);
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
