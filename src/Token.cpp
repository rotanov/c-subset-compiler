#include "Token.hpp"

namespace Compiler
{
//------------------------------------------------------------------------------
    Token::Token(const ETokenType &type, const std::string &text,
                 const unsigned &line, const unsigned &column)
        : type(type)
        , text(text)
        , line(line)
        , column(column)
        , intValue(0) // FUCK
    {

    }

//------------------------------------------------------------------------------
    Token::Token(const ETokenType& type)
        : type(type)
        , intValue(0) // FUCK
    {

    }

//------------------------------------------------------------------------------
    Token::Token()
        : intValue(0) // FUCK
    {

    }

//------------------------------------------------------------------------------
    Token::Token(const ETokenType& type, const std::string& text)
        : type(type)
        , text(text)
        , intValue(0) // FUCK
    {

    }

//------------------------------------------------------------------------------
    Token& Token::operator =(Token&& token)
    {
        if (&token == this)
        {
            return *this;
        }
        else
        {
            if (type == TT_LITERAL_CHAR_ARRAY)
            {
                delete [] charValue;
            }

            line = token.line;
            column = token.column;
            type = token.type;
            text = token.text;

            intValue = 0;

            if (token == TT_LITERAL_CHAR_ARRAY)
            {
                charValue = token.charValue;
                token.charValue = NULL;
            }
            else
            {
                intValue = token.intValue;
            }
        }
    }

//------------------------------------------------------------------------------
    Token::Token(Token&& token)
        : intValue(0)
        , line(token.line)
        , column(token.column)
        , type(token.type)
        , text(token.text)
    {
        if (token == TT_LITERAL_CHAR_ARRAY)
        {
            charValue = token.charValue;
            token.charValue = NULL;
        }
        else
        {
            intValue = token.intValue;
        }
    }

//------------------------------------------------------------------------------
    Token::Token(const Token& token)
        : intValue(0)
        , line(token.line)
        , column(token.column)
        , type(token.type)
        , text(token.text)
    {
        if (token == TT_LITERAL_CHAR_ARRAY)
        {
            size_t size = strlen(token.charValue) + 1;
            charValue = new char [size];
            memcpy(charValue, token.charValue, size);
        }
        else
        {
            intValue = token.intValue;
        }
    }

//------------------------------------------------------------------------------
    Token& Token::operator =(const Token& token)
    {
        if (&token == this)
        {
            return *this;
        }
        else
        {
            if (type == TT_LITERAL_CHAR_ARRAY)
            {
                delete [] charValue;
            }

            line = token.line;
            column = token.column;
            type = token.type;
            text = token.text;

            intValue = 0;

            if (token == TT_LITERAL_CHAR_ARRAY)
            {
                size_t size = strlen(token.charValue) + 1;
                charValue = new char [size];
                memcpy(charValue, token.charValue, size);
            }
            else
            {
                intValue = token.intValue;
            }
        }
    }

//------------------------------------------------------------------------------
    Token::~Token()
    {
        if (type == TT_LITERAL_CHAR_ARRAY)
        {
            delete [] charValue;
        }
    }

//------------------------------------------------------------------------------
    Compiler::Token::operator const ETokenType&() const
    {
        return type;
    }

} // namespace Compiler
