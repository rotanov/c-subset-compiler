#include "Token.hpp"

namespace Compiler
{
//------------------------------------------------------------------------------
    Token::Token(const ETokenType &type, const std::string &value,
                 const unsigned &line, const unsigned &column)
        : type(type)
        , value(value)
        , line(line)
        , column(column)
    {

    }

//------------------------------------------------------------------------------
    Token::Token(const ETokenType& type)
        :type(type)
    {

    }

//------------------------------------------------------------------------------
    Token::Token()
    {

    }

//------------------------------------------------------------------------------
//    bool Token::operator ==(const ETokenType& rhs) const
//    {
//        return type == rhs;
//    }

////------------------------------------------------------------------------------
//    bool Token::operator !=(const ETokenType& rhs) const
//    {
//        return !(*this == rhs);
//    }

} // namespace Compiler
