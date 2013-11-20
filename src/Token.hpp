#pragma once

#include "constants.hpp"

namespace Compiler
{
    struct Token
    {
        ETokenType type{TT_INVALID};
        std::string text{""};
        unsigned line{0};
        unsigned column{0};

        Token(const ETokenType& type, const std::string& text, const unsigned& line, const unsigned& column);
        Token(const ETokenType& type, const std::string& text);
        Token(const ETokenType &type);
        Token();
        ~Token();

        // !!! GCC bug: non static member initializer doesn't work for
        // union member (in this particular case at least)
        union
        {
            int intValue;
            float floatValue;
            char* charValue;
        };

        operator const ETokenType& () const;
    };

} // namespace Compiler
