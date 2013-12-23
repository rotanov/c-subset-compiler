#pragma once

#include "constants.hpp"

namespace Compiler
{
    struct ITokenStream
    {

        virtual void EmitInvalid(const string& source, const int line,
                                 const int column) = 0;

        virtual void EmitKeyword(const string& source, ETokenType token_type,
                                 const int line, const int column) = 0;

        virtual void EmitPunctuation(const string& source, ETokenType token_type,
                                     const int line, const int column) = 0;

        virtual void EmitIdentifier(const string& source, const int line,
                                    const int column) = 0;

        virtual void EmitLiteral(const string& source, EFundamentalType type,
                                 const void* data, size_t nbytes, const int line,
                                 const int column) = 0;

        virtual void EmitLiteralArray(const string& source, size_t num_elements,
                                      EFundamentalType type, const void* data,
                                      size_t nbytes, const int line,
                                      const int column) = 0;

        virtual void EmitEof(const int line, const int column) = 0;

        virtual void Flush() const
        {

        }

        virtual ~ITokenStream()
        {

        }
    };

} // namespace Compiler
