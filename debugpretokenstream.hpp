#pragma once

#include "ipretokenstream.hpp"

#include <string>
#include <iostream>

namespace Compiler
{
    using std::string;
    using std::cout;
    using std::endl;

    bool FindAndReplace(string& str, const string& substr, const string& replacement)
    {
        size_t i = 0;
        while ((i = str.find(substr, i)) != string::npos)
        {
            str.replace(i, substr.length(), replacement);
            i += replacement.length();
        }
    }

    void ProcessString(string& data)
    {
        FindAndReplace(data, "\\\n", "");
    }

    struct DebugPreTokenStream : IPreTokenStream
    {
        void emit_whitespace_sequence()
        {
            cout << "whitespace-sequence 0 " << endl;
        }

        void emit_new_line()
        {
            cout << "new-line 0 " << endl;
        }

        void emit_identifier(const string& data)
        {
            write_token("identifier", data);
        }

        void emit_pp_number(const string& data)
        {
            write_token("pp-number", data);
        }

        void emit_character_literal(const string& data)
        {
            write_token("character-literal", data);
        }

        void emit_string_literal(const string& data)
        {
            write_token("string-literal", data);
        }

        void emit_preprocessing_op_or_punc(const string& data)
        {
            write_token("preprocessing-op-or-punc", data);
        }

        void emit_non_whitespace_char(const string& data)
        {
            write_token("non-whitespace-character", data);
        }

        void emit_eof()
        {
            cout << "eof" << endl;
        }

    private:
        void write_token(const string& type, const string& data)
        {
    //        string str = data;
    //        ProcessString(str);
            cout << type << " " << data.size() << " ";
            cout.write(data.data(), data.size());
            cout << endl;
        }
    };

} // namespace Compiler
