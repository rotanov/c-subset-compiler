#pragma once

#include <string>
#include <unordered_map>
#include <cassert>
#include <stdexcept>

namespace Compiler
{
    // code-style note: since we have strongly typed enums there is a point to
    // rethink naming of enum members: they pollute namespace no more.
    // so prefixes are already removed, may be all-cpas should be too?
    enum class ESymbolType
    {
        TYPE_VOID,
        TYPE_FLOAT,
        TYPE_INT,
        TYPE_CHAR,
        TYPE_ARRAY,
        TYPE_POINTER,
        TYPE_STRUCT,
        VARIABLE,
        FUNCTION,
    };

    class SymbolType;
    class SymbolVariable;
    class SymbolFunction;
    class SymbolStruct;

    class Symbol
    {
    public:
        Symbol(const std::string& name);
        virtual ~Symbol();

        virtual ESymbolType GetSymbolType() const = 0;

        // seems like fault in overal design
        // but sticking with it for now
        // because I just want it to work
        // !!! also confusing with type-of-symbol-getter above
        virtual void SetTypeSymbol(SymbolType* symType)
        {
            throw std::logic_error("SetType not implemented for this symbol");
        }

        virtual bool IsComplete() const { return true; }

        std::string name{""};

    private:

    };

    class SymbolTable
    {
    public:
        SymbolTable();

        virtual ~SymbolTable();

        void AddType(SymbolType* symbolType);
        void AddFunction(SymbolFunction* symbolFunction);
        void AddVariable(SymbolVariable* symbolVariable);

        SymbolVariable* LookupVariable(const std::string& name) const;
        SymbolType* LookupType(const std::string& name) const;
        SymbolFunction* LookupFunction(const std::string& name) const;

    private:
        template <typename T>
        using TSymbols = std::unordered_map<std::string, T*>;

        TSymbols<SymbolVariable> variables_;
        TSymbols<SymbolType> types_;
        TSymbols<SymbolFunction> functions_;

        template <typename T>
        T* LookupHelper_(const TSymbols<T>& container, const std::string& key) const
        {
            if (container.find(key) != container.end())
            {
                return container.at(key);
            }
            else
            {
                return NULL;
            }
        }
    };

    class SymbolType : public Symbol
    {
    public:
        SymbolType(const std::string& name);

        virtual bool IsStruct() const;

        bool constant;
    private:

    };

    class SymbolVariable : public Symbol
    {
    public:
        virtual ESymbolType GetSymbolType() const;

        virtual void SetTypeSymbol(SymbolType* symType)
        {
            assert(typeSymbol_ == NULL);
            typeSymbol_ = symType;
        }


    private:
        SymbolType* typeSymbol_{NULL};
    };

    class SymbolFunction : public Symbol
    {
    public:
        virtual ESymbolType GetSymbolType() const;

    private:

    };

    class SymbolStruct : public SymbolType
    {
    public:
        virtual ESymbolType GetSymbolType() const { return ESymbolType::TYPE_STRUCT; }

    private:
    };

    class SymbolChar : public SymbolType
    {
    public:
        SymbolChar();

        virtual ESymbolType GetSymbolType() const { return ESymbolType::TYPE_CHAR; }
    };

    class SymbolInt : public SymbolType
    {
    public:
        SymbolInt();

        virtual ESymbolType GetSymbolType() const { return ESymbolType::TYPE_INT; }
    };

    class SymbolFloat : public SymbolType
    {
    public:
        SymbolFloat();

        virtual ESymbolType GetSymbolType() const { return ESymbolType::TYPE_INT; }
    };

    class SymbolVoid : public SymbolType
    {
    public:
        SymbolVoid();

        virtual ESymbolType GetSymbolType() const { return ESymbolType::TYPE_VOID; }
    };

    class SymbolPointer : public SymbolType
    {
    public:
        SymbolPointer(SymbolType* refSymbol)
            : SymbolType("*")
        {
            SetRefSymbol(refSymbol);
        }

        void SetRefSymbol(SymbolType* refSymbol)
        {
            assert(refSymbol != NULL);
            refSymbol = refSymbol;
        }

        SymbolType* GetRefSymbol() const
        {
            refSymbol_;
        }

        virtual ESymbolType GetSymbolType() const { return ESymbolType::TYPE_POINTER; }

    private:
        SymbolType* refSymbol_{NULL};

    };

} // namespace Compiler
