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
    {

    }

//------------------------------------------------------------------------------
    Token::Token(const ETokenType& type)
        : type(type)
    {

    }

//------------------------------------------------------------------------------
    Token::Token()
    {

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
