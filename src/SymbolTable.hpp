#pragma once

#include <string>
#include <vector>
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
        TYPE_FUNCTION,
        VARIABLE,
    };

    // symbol tables with scope type of sctructure of parameters
    // are of class SymbolTableWithOrder by agreement
    enum class EScopeType
    {
        UNDEFINED,
        INTERNAL,
        GLOBAL,
        PARAMETERS,
        STRUCTURE,
        BODY,
        BLOCK,
    };

    class SymbolType;
    class SymbolVariable;
    class SymbolStruct;

//------------------------------------------------------------------------------
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
        virtual void SetTypeSymbol(SymbolType* symType);
        virtual std::string GetQualifiedName() const = 0;

        std::string name{""};

    private:

    };

//------------------------------------------------------------------------------
    class SymbolTable
    {
    public:
        template <typename T>
        using TSymbols = std::unordered_map<std::string, T*>;

        SymbolTable(const EScopeType scope);
        virtual ~SymbolTable();

        EScopeType GetScopeType() const;

        void AddType(SymbolType* symbolType);
        void AddFunction(SymbolVariable* symbolVariable);
        virtual void AddVariable(SymbolVariable* symbolVariable);

        SymbolVariable* LookupVariable(const std::string& name) const;
        SymbolType* LookupType(const std::string& name) const;
        SymbolVariable* LookupFunction(const std::string& name) const;

        TSymbols<SymbolVariable> variables;
        TSymbols<SymbolType> types;
        TSymbols<SymbolVariable> functions;

    protected:
        SymbolTable();

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

        EScopeType scope_{EScopeType::UNDEFINED};
    };

//------------------------------------------------------------------------------
    class SymbolTableWithOrder : public SymbolTable
    {
    public:
        SymbolTableWithOrder(const EScopeType scope);
        virtual ~SymbolTableWithOrder();

        virtual void AddVariable(SymbolVariable* symbolVariable);

        std::vector<SymbolVariable*> orderedVariables;

    private:
    };

//------------------------------------------------------------------------------
    class SymbolType : public Symbol
    {
    public:
        SymbolType(const std::string& name);

        bool constant{false};

        virtual std::string GetQualifiedName() const;

    private:

    };

//------------------------------------------------------------------------------
    class SymbolVariable : public Symbol
    {
    public:
        SymbolVariable(const std::string& name);
        SymbolVariable(const std::string &name, SymbolType* symType);

        virtual ESymbolType GetSymbolType() const;
        virtual void SetTypeSymbol(SymbolType* symType);
        SymbolType* GetTypeSymbol() const;
        virtual std::string GetQualifiedName() const;

    protected:
        SymbolType* type_{NULL};
    };

//------------------------------------------------------------------------------
    class CompoundStatement;
    class SymbolFunctionType : public SymbolType
    {
    public:
        SymbolFunctionType(SymbolTableWithOrder* parametersSymTable);

        virtual ESymbolType GetSymbolType() const;
        virtual void SetTypeSymbol(SymbolType *symType);
        virtual std::string GetQualifiedName() const;
        void AddParameter(SymbolVariable* parameter);
        SymbolTableWithOrder* GetSymbolTable() const;
        void SetBody(CompoundStatement* body);
        CompoundStatement* GetBody() const;

    private:
        SymbolType* returnType_{NULL};
        SymbolTableWithOrder* parameters_{NULL};
        CompoundStatement* body_{NULL};
    };

//------------------------------------------------------------------------------
    class SymbolStruct : public SymbolType
    {
    public:
        SymbolStruct(SymbolTableWithOrder* membersSymTable, const std::string name = "");

        virtual ESymbolType GetSymbolType() const;
        virtual std::string GetQualifiedName() const;
        void AddField(SymbolVariable* field);
        SymbolTableWithOrder* GetSymbolTable() const;

    private:
        SymbolTableWithOrder* fields_{NULL};
    };

//------------------------------------------------------------------------------
    class SymbolChar : public SymbolType
    {
    public:
        SymbolChar();

        virtual ESymbolType GetSymbolType() const;

    };

//------------------------------------------------------------------------------
    class SymbolInt : public SymbolType
    {
    public:
        SymbolInt();

        virtual ESymbolType GetSymbolType() const;

    };

//------------------------------------------------------------------------------
    class SymbolFloat : public SymbolType
    {
    public:
        SymbolFloat();

        virtual ESymbolType GetSymbolType() const;

    };

//------------------------------------------------------------------------------
    class SymbolVoid : public SymbolType
    {
    public:
        SymbolVoid();

        virtual ESymbolType GetSymbolType() const;

    };

//------------------------------------------------------------------------------
    class SymbolPointer : public SymbolType
    {
    public:
        SymbolPointer();

        SymbolType* GetRefSymbol() const;
        virtual ESymbolType GetSymbolType() const;
        virtual void SetTypeSymbol(SymbolType *symType);

        virtual std::string GetQualifiedName() const;

    private:
        SymbolType* refSymbol_{NULL};

    };

//------------------------------------------------------------------------------
    class ASTNode;
    class SymbolArray : public SymbolType
    {
    public:
        SymbolArray();

        virtual ESymbolType GetSymbolType() const;
        void SetInitializer(ASTNode* initializerExpression);
        virtual void SetTypeSymbol(SymbolType *symType);

        virtual std::string GetQualifiedName() const;

    private:
        SymbolType* elementType_{NULL};
        ASTNode* sizeInitializer_{NULL};
        unsigned size_{0};

    };
} // namespace Compiler
