#pragma once

#include "constants.hpp"

namespace Compiler
{
    struct Token
    {
        ETokenType type = TT_INVALID;
        std::string value = "";
        unsigned line = 0;
        unsigned column = 0;

        Token(const ETokenType& type, const std::string& value,
              const unsigned& line, const unsigned& column);
        Token(const ETokenType &type);
        Token();

//        bool operator ==(const ETokenType& rhs) const;
//        bool operator !=(const ETokenType& rhs) const;

        operator const ETokenType& () const
        {
            return type;
        }
    };

} // namespace Compiler
