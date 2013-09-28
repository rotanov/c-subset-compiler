#pragma once

#include <iostream>
#include <exception>
#include <sstream>

#include "constants.hpp"
#include "utils.hpp"

namespace Compiler
{
    using std::cout;
    using std::endl;
    using std::logic_error;
    using std::stringstream;

    struct DebugTokenOutputStream
    {
        // output: <line>-<column>: invalid <source>
        void EmitInvalid(const string& source, const int line, const int column)
        {
            stringstream ss;
            ss << "Invalid token at " << line << "-" << column << ": " + source;
            throw logic_error(ss.str());
            cout << line << "-" << column << ": " << "invalid " << source << endl;
        }

        // output: <line>-<column>: keyword <token_type> <source>
        void EmitKeyword(const string& source, ETokenType token_type,
                         const int line, const int column)
        {
            cout << line << "-" << column << ": " << cout << "keyword "
                 << KeywordTypeToStringMap.at(token_type) << " " << source << endl;
        }

        // output: <line>-<column>: punctuation <token_type> <source>
        void EmitPunctuation(const string& source, ETokenType token_type,
                             const int line, const int column)
        {
            cout << line << "-" << column << ": " << "punctuation "
                 << PunctuationTypeToStringMap.at(token_type) << " " << source <<  endl;
        }

        // output: <line>-<column>: identifier <source>
        void EmitIdentifier(const string& source, const int line, const int column)
        {
            cout << line << "-" << column << ": " << "identifier " << " " << source << endl;
        }

        // output: <line>-<column>: literal <type> <source> <hexdump(data,nbytes)>
        void EmitLiteral(const string& source, EFundamentalType type, const void* data,
                         size_t nbytes, const int line, const int column)
        {
            cout << line << "-" << column << ": " << "literal " << " " << FundamentalTypeToStringMap.at(type)
                 << " " << source << " " << HexDump(data, nbytes) << endl;
        }

        // output: <line>-<column>: literal <source> array of <num_elements> <type> <hexdump(data,nbytes)>
        void EmitLiteralArray(const string& source, size_t num_elements,
                              EFundamentalType type, const void* data, size_t nbytes,
                              const int line, const int column)
        {
            cout << line << "-" << column << ": " << "literal " << source << " array of "
                 << num_elements << " " << FundamentalTypeToStringMap.at(type)
                 << " " << HexDump(data, nbytes) << endl;
        }

        // output : eof
        void EmitEof()
        {
            cout << "eof" << endl;
        }
    };

} // namespace Compiler
