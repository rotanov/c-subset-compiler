#pragma once

#include <string>

namespace Compiler
{
    struct IPreTokenStream
    {
        virtual void EmitWhitespaceSequence(const int rowOffset) = 0;
        virtual void EmitNewLine() = 0;
        virtual void EmitIdentifier(const int* data, size_t size) = 0;
        virtual void EmitPpNumber(const std::string& data) = 0;
        virtual void EmitCharacterLiteral(const std::string& data) = 0;
        virtual void EmitStringLiteral(const std::string& data) = 0;
        virtual void EmitPunctuation(const std::string& data) = 0;
        virtual void EmitNonWhitespaceChar(const std::string& data) = 0;
        virtual void EmitEof() = 0;
        virtual void Flush() = 0;

        virtual ~IPreTokenStream() {}
    };

} // namespace Compiler
