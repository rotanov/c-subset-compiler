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
        std::string name{""};

        Symbol(const std::string& name);
        virtual ~Symbol();

        virtual ESymbolType GetType() const = 0;
        virtual std::string GetQualifiedName() const = 0;
        virtual bool IfTypeFits(shared_ptr<Symbol> symbol) const = 0;

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
        int virtual GetSize() const = 0;
    };
//------------------------------------------------------------------------------
    class SymbolTypeRef
            : public SymbolType
            , public std::enable_shared_from_this<SymbolTypeRef>
    {
    public:
        SymbolTypeRef(const std::string& name);
        SymbolTypeRef(const std::string& name, shared_ptr<SymbolType> type);

        void SetRefSymbol(shared_ptr<SymbolType> type);
        shared_ptr<SymbolType> GetRefSymbol() const;
        int virtual GetSize() const;

    protected:
        shared_ptr<SymbolType> type_{NULL};
    };

//------------------------------------------------------------------------------
    class ASTNode;
    class SymbolVariable : public SymbolTypeRef
    {
    public:
        SymbolVariable(const std::string& name);
        SymbolVariable(const std::string &name, shared_ptr<SymbolType> symType);

        virtual ESymbolType GetType() const;
        virtual std::string GetQualifiedName() const;
        virtual bool IfTypeFits(shared_ptr<Symbol> symbol) const;
        void PushInitializer(shared_ptr<ASTNode> initializer);

        int offset{-1};

    protected:
        std::vector<shared_ptr<ASTNode>> initializers_;
    };

//------------------------------------------------------------------------------
    class CompoundStatement;
    class SymbolFunctionType : public SymbolTypeRef
    {
    public:
        SymbolFunctionType(shared_ptr<SymbolTableWithOrder> parametersSymTable);
        SymbolFunctionType(shared_ptr<SymbolType> returnType, shared_ptr<SymbolTableWithOrder> parametersSymTable);

        virtual ESymbolType GetType() const;
        virtual std::string GetQualifiedName() const;
        virtual bool IfTypeFits(shared_ptr<Symbol> symbol) const;
        void AddParameter(shared_ptr<SymbolVariable> parameter);
        shared_ptr<SymbolTableWithOrder> GetSymbolTable() const;
        void SetBody(shared_ptr<CompoundStatement> body);
        shared_ptr<CompoundStatement> GetBody() const;

    private:
        shared_ptr<SymbolTableWithOrder> parameters_{NULL};
        shared_ptr<CompoundStatement> body_{NULL};
    };

//------------------------------------------------------------------------------
    class SymbolStruct : public SymbolType
    {
    public:
        SymbolStruct(const std::string name = "");

        virtual ESymbolType GetType() const;
        virtual std::string GetQualifiedName() const;
        void AddField(shared_ptr<SymbolVariable> field);
        shared_ptr<SymbolTableWithOrder> GetSymbolTable() const;
        void SetFieldsSymTable(shared_ptr<SymbolTableWithOrder> fieldsSymTable);
        virtual bool IfTypeFits(shared_ptr<Symbol> symbol) const;
        int virtual GetSize() const;

        bool complete{false};

    private:
        shared_ptr<SymbolTableWithOrder> fields_{NULL};
        int size_;
    };

//------------------------------------------------------------------------------
    class SymbolChar : public SymbolType
    {
    public:
        SymbolChar();

        virtual ESymbolType GetType() const;
        virtual bool IfTypeFits(shared_ptr<Symbol> symbol) const;
        virtual int GetSize() const;
    };

//------------------------------------------------------------------------------
    class SymbolInt : public SymbolType
    {
    public:
        SymbolInt();

        virtual ESymbolType GetType() const;
        virtual bool IfTypeFits(shared_ptr<Symbol> symbol) const;
        virtual int GetSize() const;
    };

//------------------------------------------------------------------------------
    class SymbolFloat : public SymbolType
    {
    public:
        SymbolFloat();

        virtual ESymbolType GetType() const;
        virtual bool IfTypeFits(shared_ptr<Symbol> symbol) const;
        virtual int GetSize() const;
    };

//------------------------------------------------------------------------------
    class SymbolVoid : public SymbolType
    {
    public:
        SymbolVoid();

        virtual ESymbolType GetType() const;
        virtual bool IfTypeFits(shared_ptr<Symbol> symbol) const;
        virtual int GetSize() const;
    };

//------------------------------------------------------------------------------
    class SymbolPointer : public SymbolTypeRef
    {
    public:
        SymbolPointer();
        SymbolPointer(shared_ptr<SymbolType> symType);

        virtual ESymbolType GetType() const;
        virtual std::string GetQualifiedName() const;
        virtual bool IfTypeFits(shared_ptr<Symbol> symbol) const;

    };

//------------------------------------------------------------------------------
    class ASTNode;

    class SymbolArray : public SymbolTypeRef
    {
    public:
        SymbolArray();

        virtual ESymbolType GetType() const;
        virtual std::string GetQualifiedName() const;
        virtual bool IfTypeFits(shared_ptr<Symbol> symbol) const;
        void SetSizeInitializer(shared_ptr<ASTNode> initializerExpression);
        virtual int GetSize() const;

    private:
        shared_ptr<ASTNode> sizeInitializer_{NULL};
        unsigned size_{0};

    };

//------------------------------------------------------------------------------
    class SymbolConst : public SymbolTypeRef
    {
    public:
        SymbolConst();
        SymbolConst(shared_ptr<SymbolType> symType);

        virtual ESymbolType GetType() const;
        virtual std::string GetQualifiedName() const;
        virtual bool IfTypeFits(shared_ptr<Symbol> symbol) const;

    private:
        shared_ptr<SymbolType> refSymbol_{NULL};
    };

//------------------------------------------------------------------------------
    class SymbolTypedef : public SymbolTypeRef
    {
    public:
        SymbolTypedef(const std::string& name);

        virtual ESymbolType GetType() const;
        virtual std::string GetQualifiedName() const;
        virtual bool IfTypeFits(shared_ptr<Symbol> symbol) const;

    };

//------------------------------------------------------------------------------
    bool IfSymbolIsRef(shared_ptr<Symbol> symbol);
    bool IfInteger(shared_ptr<SymbolType> symbol);
    bool IfArithmetic(shared_ptr<SymbolType> symbol);
    bool IfScalar(shared_ptr<SymbolType> symbol);
    bool IfOfType(shared_ptr<SymbolType> symbol, ESymbolType type);
    bool IfConst(shared_ptr<SymbolType> symbol);
    shared_ptr<SymbolType> GetRefSymbol(shared_ptr<Symbol> symbol);
    shared_ptr<SymbolType> CalcCommonArithmeticType(shared_ptr<SymbolType> left, shared_ptr<SymbolType> right);
    shared_ptr<SymbolType> GetActualType(shared_ptr<SymbolType> symbol);
    shared_ptr<SymbolType> GetArrayType(shared_ptr<SymbolType> symbol);

} // namespace Compiler
