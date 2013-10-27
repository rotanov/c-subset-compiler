#pragma once

#include <string>

namespace Compiler
{
    class Symbol
    {
    public:
        Symbol(const std::string& name)
            : name_(name)
        {

        }

        virtual ~Symbol()
        {

        }

    private:
        std::string name_;
    };

    class SymbolType;
    class SymbolVariable;
    class SymbolFunction;
    class SymbolStruct;

    class SymbolTable
    {
    public:

    };

    class SymbolTableGlobal
    {
    public:

    };

} // namespace Compiler
