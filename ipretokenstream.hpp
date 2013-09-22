#pragma once

#include <string>

namespace Compiler
{
    struct IPreTokenStream
    {
        virtual void emit_whitespace_sequence() = 0;
        virtual void emit_new_line() = 0;
        virtual void emit_identifier(const std::string& data) = 0;
        virtual void emit_pp_number(const std::string& data) = 0;
        virtual void emit_character_literal(const std::string& data) = 0;
        virtual void emit_string_literal(const std::string& data) = 0;
        virtual void emit_preprocessing_op_or_punc(const std::string& data) = 0;
        virtual void emit_non_whitespace_char(const std::string& data) = 0;
        virtual void emit_eof() = 0;

        virtual ~IPreTokenStream() {}
    };

} // namespace Compiler
