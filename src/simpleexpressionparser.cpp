#include "SimpleExpressionParser.hpp"

#include <sstream>
#include <unordered_set>
#include <functional>
#include <queue>
#include <iostream>

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
            return new ASTNode(token, left, ParseExpression_());
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
                : length(0)
                , left(NULL)
                , right(NULL)
                , depth(0)
            {

            }

            std::string text;
            unsigned length;
            PrintTreeNode* left;
            PrintTreeNode* right;
            unsigned depth;
        };

        auto depth = 0;
        std::function<unsigned(ASTNode*, PrintTreeNode*)> f = [&](ASTNode* next, PrintTreeNode* print) -> unsigned
        {
            if (next != NULL)
            {
                if (next->GetLeft() == NULL
                        && next->GetRight() == NULL)
                {
                    if (print != NULL)
                    {
                        print->text = next->token.value + ' ';
                        print->length = next->token.value.size() + 1;
                        print->depth = depth;
                    }
                    return print->length;
                }
                else
                {
                    print->left = new PrintTreeNode;
                    print->right = new PrintTreeNode;
                    depth++;
                    unsigned length = f(next->GetLeft(), print->left) + f(next->GetRight(), print->right);
                    depth--;
                    if (print != NULL)
                    {
                        print->text = next->token.value + std::string(print->left->length, '-') + std::string(print->right->length - 1, ' ');
                        print->depth = depth;
                        print->length = length;
                    }
                    return length;
                }
            }
            else
            {
                return 0u;
            }
        };

        PrintTreeNode printRoot;
        f(root, &printRoot);

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
            }
        }
    }

    ASTNode *SimpleExpressionParser::ParseTerm_()
    {
        ASTNode* left = ParseFactor_();
        Token token = GetNextToken_();

        if (token == OP_STAR
                || token == OP_DIV)
        {
            return new ASTNode(token, left, ParseTerm_());
        }
        else
        {
            pos_--;
            return left;
        }
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

    void SimpleExpressionParser::EmitInvalid(const string &source, const int line,
                                             const int column)
    {
        ThrowInvalidTokenError_(Token(TT_INVALID, source, line, column));
    }

    void SimpleExpressionParser::EmitKeyword(const string &source,
                                             ETokenType token_type,
                                             const int line, const int column)
    {
        ThrowInvalidTokenError_(Token(token_type, source, line, column));
    }

    void SimpleExpressionParser::EmitPunctuation(const string &source,
                                                 ETokenType token_type,
                                                 const int line, const int column)
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

    void SimpleExpressionParser::EmitIdentifier(const string &source,
                                                const int line, const int column)
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
