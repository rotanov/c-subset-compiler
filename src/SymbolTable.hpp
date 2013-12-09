#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <memory>

namespace Compiler
{
    using std::shared_ptr;
    using std::weak_ptr;
    using std::make_shared;
    using std::static_pointer_cast;
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
        TYPE_CONST,
        TYPE_TYPEDEF,
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
        LOOP,
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
        virtual void SetTypeSymbol(shared_ptr<SymbolType> /*symType*/);
        virtual std::string GetQualifiedName() const = 0;

        std::string name{""};

    private:

    };

//------------------------------------------------------------------------------
    class SymbolTable
    {
    public:
        template <typename T>
        using TSymbols = std::unordered_map<std::string, shared_ptr<T>>;

        SymbolTable(const EScopeType scope);
        virtual ~SymbolTable();

        EScopeType GetScopeType() const;

        void AddType(shared_ptr<SymbolType> symbolType);
        void AddType(shared_ptr<SymbolType> symbolType, const std::string& key);
        void AddFunction(shared_ptr<SymbolVariable> symbolVariable);
        virtual void AddVariable(shared_ptr<SymbolVariable> symbolVariable);

        shared_ptr<SymbolVariable> LookupVariable(const std::string& name) const;
        shared_ptr<SymbolType> LookupType(const std::string& name) const;
        shared_ptr<SymbolVariable> LookupFunction(const std::string& name) const;

        TSymbols<SymbolVariable> variables;
        TSymbols<SymbolType> types;
        TSymbols<SymbolVariable> functions;

    protected:
        SymbolTable();

        template <typename T>
        shared_ptr<T> LookupHelper_(const TSymbols<T>& container, const std::string& key) const
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

        virtual void AddVariable(shared_ptr<SymbolVariable> symbolVariable);

        std::vector<shared_ptr<SymbolVariable>> orderedVariables;

    private:
    };

//------------------------------------------------------------------------------
    class SymbolType : public Symbol
    {
    public:
        SymbolType(const std::string& name);

        virtual std::string GetQualifiedName() const;

    };

//------------------------------------------------------------------------------
    class ASTNode;
    class SymbolVariable : public Symbol
    {
    public:
        SymbolVariable(const std::string& name);
        SymbolVariable(const std::string &name, shared_ptr<SymbolType> symType);

        virtual ESymbolType GetSymbolType() const;
        virtual void SetTypeSymbol(shared_ptr<SymbolType> symType);
        shared_ptr<SymbolType> GetTypeSymbol() const;
        virtual std::string GetQualifiedName() const;
        void SetInitializer(shared_ptr<ASTNode> initializer);

    protected:
        shared_ptr<SymbolType> type_{NULL};
        shared_ptr<ASTNode> initializer_{NULL};
    };

//------------------------------------------------------------------------------
    class CompoundStatement;
    class SymbolFunctionType : public SymbolType
    {
    public:
        SymbolFunctionType(shared_ptr<SymbolTableWithOrder> parametersSymTable);

        virtual ESymbolType GetSymbolType() const;
        virtual void SetTypeSymbol(shared_ptr<SymbolType> symType);
        virtual std::string GetQualifiedName() const;
        void AddParameter(shared_ptr<SymbolVariable> parameter);
        shared_ptr<SymbolTableWithOrder> GetSymbolTable() const;
        void SetBody(shared_ptr<CompoundStatement> body);
        shared_ptr<CompoundStatement> GetBody() const;

    private:
        shared_ptr<SymbolType> returnType_{NULL};
        shared_ptr<SymbolTableWithOrder> parameters_{NULL};
        shared_ptr<CompoundStatement> body_{NULL};
    };

//------------------------------------------------------------------------------
    class SymbolStruct : public SymbolType
    {
    public:
        SymbolStruct(const std::string name = "");

        virtual ESymbolType GetSymbolType() const;
        virtual std::string GetQualifiedName() const;
        void AddField(shared_ptr<SymbolVariable> field);
        shared_ptr<SymbolTableWithOrder> GetSymbolTable() const;
        void SetFieldsSymTable(shared_ptr<SymbolTableWithOrder> fieldsSymTable);

        bool complete{false};

    private:
        shared_ptr<SymbolTableWithOrder> fields_{NULL};
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
        SymbolPointer(shared_ptr<SymbolType> symType);

        shared_ptr<SymbolType> GetRefSymbol() const;
        virtual ESymbolType GetSymbolType() const;
        virtual void SetTypeSymbol(shared_ptr<SymbolType> symType);

        virtual std::string GetQualifiedName() const;

    private:
        shared_ptr<SymbolType> refSymbol_{NULL};

    };

//------------------------------------------------------------------------------
    class ASTNode;
    class SymbolArray : public SymbolType
    {
    public:
        SymbolArray();

        virtual ESymbolType GetSymbolType() const;
        void SetSizeInitializer(shared_ptr<ASTNode> initializerExpression);
        virtual void SetTypeSymbol(shared_ptr<SymbolType> symType);

        virtual std::string GetQualifiedName() const;

    private:
        shared_ptr<SymbolType> elementType_{NULL};
        shared_ptr<ASTNode> sizeInitializer_{NULL};
        unsigned size_{0};

    };

//------------------------------------------------------------------------------
    class SymbolConst : public SymbolType
    {
    public:
        SymbolConst();
        SymbolConst(shared_ptr<SymbolType> symType);

        shared_ptr<SymbolType> GetRefSymbol() const;
        virtual ESymbolType GetSymbolType() const;
        virtual void SetTypeSymbol(shared_ptr<SymbolType> symType);
        virtual std::string GetQualifiedName() const;

    private:
        shared_ptr<SymbolType> refSymbol_{NULL};
    };

//------------------------------------------------------------------------------
    class SymbolTypedef : public SymbolType
    {
    public:
        SymbolTypedef(const std::string& name);

        shared_ptr<SymbolType> GetTypeSymbol() const;
        virtual ESymbolType GetSymbolType() const;
        virtual void SetTypeSymbol(shared_ptr<SymbolType> symType);
        virtual std::string GetQualifiedName() const;

    private:
        shared_ptr<SymbolType> type_{NULL};
    };

} // namespace Compiler
