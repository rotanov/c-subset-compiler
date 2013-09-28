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

        virtual void EmitWhitespaceSequence(const int rowOffset);
        virtual void EmitNewLine();
        virtual void EmitIdentifier(const int* data, size_t size);
        virtual void EmitPpNumber(const string& data);
        virtual void EmitCharacterLiteral(const string& data);
        virtual void EmitStringLiteral(const string& data);
        virtual void EmitPunctuation(const string& data);
        virtual void EmitNonWhitespaceChar(const string& data);
        virtual void EmitEof();
        virtual void Flush();

     private:
        struct StringLiteralRecord
        {
            string data;
            vector<int> codePoints;
        };

        DebugTokenOutputStream& output_;
        int* codePoints_;
        int codePointsCount_;
        unsigned codePointsAllocated_;
        vector<StringLiteralRecord> stringLiterals_;
        string characterLiteralData_;
        int line_;
        int column_;

        void DecodeInput_(const string& data);
        void PushStringRecord_(const StringLiteralRecord& record);
        void FlushAdjacentStringLiterals_();
    };

} // namespace Compiler
