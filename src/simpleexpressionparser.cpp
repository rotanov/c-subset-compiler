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
    ASTNode::~ASTNode()
    {
        delete left_;
        delete right_;
    }

    ASTNode::ASTNode(const SimpleExpressionToken &token)
        : left_(NULL)
        , right_(NULL)
        , token(token)
    {

    }

    ASTNode::ASTNode(const SimpleExpressionToken &token, ASTNode *left, ASTNode *right)
        : left_(left)
        , right_(right)
        , token(token)
    {

    }

    SimpleExpressionParser::SimpleExpressionParser()
        : pos_(0)
    {

    }

    ASTNode *SimpleExpressionParser::ParseExpression_()
    {
        ASTNode* left = ParseTerm_();
        Token token = GetNextToken_();

        if (token == OP_PLUS
                || token == OP_MINUS)
        {
            while (token == OP_PLUS
                    || token == OP_MINUS)
            {
                left = new ASTNode(token, left, ParseTerm_());
                token = GetNextToken_();
            }
            pos_--;
            return left;
        }
        else
        {
            pos_--;
            return left;
        }
    }

    const SimpleExpressionParser::Token& SimpleExpressionParser::GetNextToken_()
    {
        const Token& r = tokens_[pos_];
        pos_++;
        return r;
    }

    void SimpleExpressionParser::ThrowInvalidTokenError_(const SimpleExpressionParser::Token &token)
    {
        std::stringstream ss;
        ss << "unexpected token at " << token.line << "-" << token.column;
        throw std::logic_error(ss.str());
    }

    void SimpleExpressionParser::PrintAST_(ASTNode *root) const
    {
        struct PrintTreeNode
        {
            PrintTreeNode()
//                : length(0)
                : left(NULL)
                , right(NULL)
                , depth(0)
            {

            }

            std::string text;
//            unsigned length;
            PrintTreeNode* left;
            PrintTreeNode* right;
            unsigned depth;
        };

        std::vector<int> offsetByDepth;
//        std::vector<std::string> outputLines;
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

    ASTNode *SimpleExpressionParser::ParseTerm_()
    {
        ASTNode* left = ParseFactor_();
        Token token = GetNextToken_();

        while (token == OP_STAR
                || token == OP_DIV)
        {
            left = new ASTNode(token, left, ParseFactor_());
            token = GetNextToken_();
        }
        pos_--;
        return left;
    }

    ASTNode *SimpleExpressionParser::ParseFactor_()
    {
        Token token = GetNextToken_();

        if (token == TT_LITERAL
                || token == TT_IDENTIFIER)
        {
            return new ASTNode(token);
        }
        else if (token == OP_LPAREN)
        {
            ASTNode* r = ParseExpression_();

            token = GetNextToken_();
            if (token != OP_RPAREN)
            {
                delete r;
                std::stringstream ss;
                ss << "right brace ')' expected at" << token.line << "-"
                   << token.column;
                throw std::logic_error(ss.str());
            }
            return r;
        }
        else if (token == TT_EOF)
        {
            throw std::logic_error("empty expression");
        }
        else
        {
            ThrowInvalidTokenError_(token);
        }
    }

    void SimpleExpressionParser::EmitInvalid(const string &source, const int line, const int column)
    {
        ThrowInvalidTokenError_(Token(TT_INVALID, source, line, column));
    }

    void SimpleExpressionParser::EmitKeyword(const string &source, ETokenType token_type, const int line, const int column)
    {
        ThrowInvalidTokenError_(Token(token_type, source, line, column));
    }

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
            tokens_.push_back(token);
        }
        else
        {
            ThrowInvalidTokenError_(token);
        }
    }

    void SimpleExpressionParser::EmitIdentifier(const string &source, const int line, const int column)
    {
        Token token(TT_IDENTIFIER, source, line, column);
        tokens_.push_back(token);
    }

    void SimpleExpressionParser::EmitLiteral(const string &source,
                                             EFundamentalType type,
                                             const void *data, size_t nbytes,
                                             const int line, const int column)
    {
        Token token(TT_LITERAL, source, line, column);
        tokens_.push_back(token);
    }

    void SimpleExpressionParser::EmitLiteralArray(const string &source,
                                                  size_t num_elements,
                                                  EFundamentalType type,
                                                  const void *data, size_t nbytes,
                                                  const int line, const int column)
    {
        Token token(TT_LITERAL_ARRAY, source, line, column);
        ThrowInvalidTokenError_(token);
    }

    void SimpleExpressionParser::EmitEof()
    {
        // TODO: 0, 0 -?
        Token token(TT_EOF, "", 0, 0);
        tokens_.push_back(token);
        while(tokens_[pos_] != TT_EOF)
        {
            ASTNode* root = ParseExpression_();
            PrintAST_(root);
            std::cout << std::endl;
        }
    }

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
