#include "Token.hpp"

#include <sstream>

#include "utils.hpp"

namespace Compiler
{
//------------------------------------------------------------------------------
    Token::Token(const ETokenType &type, const std::string &text,
                 const unsigned &line, const unsigned &column)
        : type(type)
        , text(text)
        , line(line)
        , column(column)
        // there is bug in GCC wich prevents me from
        // C++11 style initialization of anonymous unions fields
        , intValue(0)
    {

    }

//------------------------------------------------------------------------------
    Token::Token(const ETokenType& type)
        : type(type)
        , intValue(0)
    {

    }

//------------------------------------------------------------------------------
    Token::Token()
        : intValue(0)
    {

    }

//------------------------------------------------------------------------------
    Token::Token(const ETokenType& type, const std::string& text)
        : type(type)
        , text(text)
        , intValue(0)
    {

    }

//------------------------------------------------------------------------------
    Token& Token::operator =(Token&& token)
    {
        if (&token != this)
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
        return *this;
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
        if (&token != this)
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
        return *this;
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

//------------------------------------------------------------------------------
    void ThrowInvalidTokenError(const Token& token, const std::string& descriptionText)
    {
        std::stringstream ss;
        ss << "unexpected token " << TokenTypeToString(token.type) << " : \""
           << token.text << "\" at " << token.line << "-" << token.column;
        if (!descriptionText.empty())
        {
            ss << ", " << descriptionText;
        }
        throw std::logic_error(ss.str());
    }

} // namespace Compiler
