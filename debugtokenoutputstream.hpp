#pragma once

#include <iostream>

#include "constants.hpp"
#include "utils.hpp"

namespace Compiler
{
    using std::cout;
    using std::endl;

    struct DebugTokenOutputStream
    {
        // output: invalid <source>
        void emit_invalid(const string& source)
        {
            cout << "invalid " << source << endl;
        }

        // output: simple <source> <token_type>
        void emit_simple(const string& source, ETokenType token_type)
        {
            cout << "simple " << source << " " << TokenTypeToStringMap.at(token_type) << endl;
        }

        // output: identifier <source>
        void emit_identifier(const string& source)
        {
            cout << "identifier " << source << endl;
        }

        // output: literal <source> <type> <hexdump(data,nbytes)>
        void emit_literal(const string& source, EFundamentalType type, const void* data, size_t nbytes)
        {
            cout << "literal " << source << " " << FundamentalTypeToStringMap.at(type) << " " << HexDump(data, nbytes) << endl;
        }

        // output: literal <source> array of <num_elements> <type> <hexdump(data,nbytes)>
        void emit_literal_array(const string& source, size_t num_elements, EFundamentalType type, const void* data, size_t nbytes)
        {
            cout << "literal " << source << " array of " << num_elements << " " << FundamentalTypeToStringMap.at(type) << " " << HexDump(data, nbytes) << endl;
        }

        // output : eof
        void emit_eof()
        {
            cout << "eof" << endl;
        }
    };

} // namespace Compiler
