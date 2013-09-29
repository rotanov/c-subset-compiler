#pragma once

#include <vector>
#include <iostream>

//#include "token.hpp"
#include "ipretokenstream.hpp"

namespace Compiler
{
    class PreTokenizer
    {
    public:
        PreTokenizer(const std::vector<char>& input, IPreTokenStream& output);

        virtual ~PreTokenizer()
        {
            delete [] source_;
        }

    private:
        // TS_ for Tokenizer State
        enum State
        {
            TS_START,
            TS_WHITESPACE,
            TS_COMMENT,
            TS_INLINE_COMMENT,
            TS_MATCHING_IDENTIFIER,
            TS_MATCHING_PP_NUMBER,
            TS_MATCHING_CHARACTER_LITERAL,
            TS_MATCHING_S_CHAR_SEQUENCE,
            TS_FINISHED,
        };

        State state_;
        int pos_;
        unsigned sourceSize_;
        int* source_;
        IPreTokenStream& output_;
        int outStart_;
        int whitespaceSequenceBegin_;

        void DecodeUTF_();
        void ProcessTrigraphs_();
        void ProcessSplicing_();
        void Process_();

    };

} // namespace Compiler
