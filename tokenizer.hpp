#pragma once

#include "ipretokenstream.hpp"
#include "debugtokenoutputstream.hpp"

namespace Compiler
{
    class Tokenizer : public IPreTokenStream
    {
    public:
        Tokenizer(DebugTokenOutputStream& output);
        virtual ~Tokenizer();

        virtual void emit_whitespace_sequence();
        virtual void emit_new_line();
        virtual void emit_identifier(const string& data);
        virtual void emit_pp_number(const string& data);
        virtual void emit_character_literal(const string& data);
        virtual void emit_string_literal(const string& data);
        virtual void emit_preprocessing_op_or_punc(const string& data);
        virtual void emit_non_whitespace_char(const string& data);
        virtual void emit_eof();


     private:
        struct StringLiteralRecord
        {
            string data;
            vector<int> codePoints;
        };

        DebugTokenOutputStream& output_;
        int* codePoints_;
        int codePointsCount_;
        int codePointsAllocated_;
        vector<StringLiteralRecord> stringLiterals_;
        string characterLiteralData_;

        void DecodeInput_(const string& data);
        void PushStringRecord_(const StringLiteralRecord& record);
        void FlushAdjacentStringLiterals_();
    };

} // namespace Compiler
