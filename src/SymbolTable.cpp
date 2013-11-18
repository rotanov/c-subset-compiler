#include "SymbolTable.hpp"

#include "ASTNode.hpp"

namespace Compiler
{
    Symbol::Symbol(const std::string& name)
        : name(name)
    {

    }

    Symbol::~Symbol()
    {

    }

    void Symbol::SetTypeSymbol(SymbolType* symType)
    {
        throw std::logic_error("SetType not implemented for this symbol");
    }

    SymbolTable::SymbolTable(const EScopeType scope)
        : scope_(scope)
    {
        assert(scope != EScopeType::UNDEFINED);
    }

    SymbolTable::~SymbolTable()
    {

    }

    EScopeType SymbolTable::GetScopeType() const
    {
        return scope_;
    }

    void SymbolTable::AddType(SymbolType* symbolType)
    {
        assert(symbolType != NULL);

        std::string key = symbolType->name;
//        if (symbolType->GetSymbolType() == ESymbolType::TYPE_STRUCT)
//        {
//            key = "struct " + key;
//        }

        assert(types.find(key) == types.end());

        types[key] = symbolType;
    }

    void SymbolTable::AddFunction(SymbolFunction* symbolFunction)
    {
        assert(symbolFunction != NULL);

        std::string key = symbolFunction->name;

        assert(types.find(key) == types.end());

        functions[key] = symbolFunction;
    }

    void SymbolTable::AddVariable(SymbolVariable* symbolVariable)
    {
        assert(symbolVariable != NULL);

        std::string key = symbolVariable->name;

        assert(types.find(key) == types.end());

        variables[key] = symbolVariable;
    }

    SymbolVariable*SymbolTable::LookupVariable(const std::string& name) const
    {
        return LookupHelper_(variables, name);
    }

    SymbolType*SymbolTable::LookupType(const std::string& name) const
    {
        return LookupHelper_(types, name);
    }

    SymbolFunction*SymbolTable::LookupFunction(const std::string& name) const
    {
        return LookupHelper_(functions, name);
    }

    SymbolType::SymbolType(const std::string& name)
        : Symbol(name)
    {

    }

    std::string SymbolType::GetQualifiedName() const
    {
        return (constant ? "const " : "") + name;
    }

    SymbolVariable::SymbolVariable(const std::string& name)
        : Symbol(name)
    {

    }

    SymbolVariable::SymbolVariable(const std::string& name, SymbolType* symType)
        : Symbol(name)
    {
        SetTypeSymbol(symType);
    }

    ESymbolType SymbolVariable::GetSymbolType() const
    {
        return ESymbolType::VARIABLE;
    }

    void SymbolVariable::SetTypeSymbol(SymbolType* symType)
    {
        assert(type_ == NULL);
        type_ = symType;
    }

    std::string SymbolVariable::GetQualifiedName() const
    {
        assert(type_ != NULL);
        return "variable " + name + " of type " + type_->GetQualifiedName();
    }

    SymbolFunction::SymbolFunction(const std::string& name)
        : Symbol(name)
    {

    }

    SymbolFunction::SymbolFunction(const std::string& name, SymbolFunctionType* symType)
        : Symbol(name)
    {
        SetTypeSymbol(symType);
    }

    ESymbolType SymbolFunction::GetSymbolType() const
    {
        return ESymbolType::FUNCTION;
    }

    void SymbolFunction::SetTypeSymbol(SymbolFunctionType* symType)
    {
        assert(symType != NULL);
        assert(type_ == NULL);
        type_ = symType;
    }

    SymbolFunctionType* SymbolFunction::GetTypeSymbol() const
    {
        assert(type_ != NULL);
        return type_;
    }

    std::string SymbolFunction::GetQualifiedName() const
    {
        assert(type_ != NULL);
        return "function " + name + " of type " + type_->GetQualifiedName();
    }

    SymbolChar::SymbolChar()
        : SymbolType("char")
    {

    }

    ESymbolType SymbolChar::GetSymbolType() const
    {
        return ESymbolType::TYPE_CHAR;
    }

    SymbolInt::SymbolInt()
        : SymbolType("int")
    {

    }

    ESymbolType SymbolInt::GetSymbolType() const
    {
        return ESymbolType::TYPE_INT;
    }

    SymbolFloat::SymbolFloat()
        : SymbolType("float")
    {

    }

    ESymbolType SymbolFloat::GetSymbolType() const
    {
        return ESymbolType::TYPE_INT;
    }

    SymbolVoid::SymbolVoid()
        : SymbolType("void")
    {

    }

    ESymbolType SymbolVoid::GetSymbolType() const
    {
        return ESymbolType::TYPE_VOID;
    }

    SymbolFunctionType::SymbolFunctionType(SymbolTable* parametersSymTable)
        : SymbolType("function")
        , parameters_(parametersSymTable)
    {
        assert(parameters_ != NULL);
    }

    ESymbolType SymbolFunctionType::GetSymbolType() const
    {
        return ESymbolType::TYPE_FUNCTION;
    }

    void SymbolFunctionType::SetTypeSymbol(SymbolType* symType)
    {
        assert(symType != NULL);
        ESymbolType type = symType->GetSymbolType();
        if (type == ESymbolType::TYPE_ARRAY
            || type == ESymbolType::TYPE_FUNCTION)
        {
            throw std::logic_error("function return type can't be either array or function");
        }
        returnType_ = symType;
    }

    std::string SymbolFunctionType::GetQualifiedName() const
    {
        assert(returnType_ != NULL);
        std::string argStr;
        for (auto& a : orderedParameters_)
        {
            argStr += a->GetQualifiedName() + ", ";
        }
        return "function(" + argStr + ") returning " + returnType_->GetQualifiedName();
    }

    void SymbolFunctionType::AddParameter(SymbolVariable* parameter)
    {
        assert(parameter != NULL);
        orderedParameters_.push_back(parameter);
        parameters_->AddVariable(parameter);
    }

    SymbolTable*SymbolFunctionType::GetSymbolTable() const
    {
        assert(parameters_ != NULL);
        return parameters_;
    }

    SymbolStruct::SymbolStruct(SymbolTable* membersSymTable, const std::string name)
        : SymbolType(name)
        , fields_(membersSymTable)
    {
        assert(fields_ != NULL);
    }

    ESymbolType SymbolStruct::GetSymbolType() const
    {
        return ESymbolType::TYPE_STRUCT;
    }

    std::string SymbolStruct::GetQualifiedName() const
    {
        // TODO: don't print members everytime
        std::string membersStr;
        if (fields_ != NULL)
        {
            for (auto& s : fields_->variables)
            {
                membersStr += "\n    " + s.second->GetQualifiedName();
            }
        }
        return "struct " + name + membersStr + "\n";
    }

    void SymbolStruct::AddField(SymbolVariable* field)
    {
        assert(field != NULL);
        orderedFields_.push_back(field);
        fields_->AddVariable(field);
    }

    SymbolPointer::SymbolPointer()
        : SymbolType("pointer")
    {
    }

    SymbolType* SymbolPointer::GetRefSymbol() const
    {
        refSymbol_;
    }

    ESymbolType SymbolPointer::GetSymbolType() const
    {
        return ESymbolType::TYPE_POINTER;
    }

    void SymbolPointer::SetTypeSymbol(SymbolType* symType)
    {
        assert(symType != NULL);
        refSymbol_ = symType;
    }

    std::string SymbolPointer::GetQualifiedName() const
    {
        assert(refSymbol_ != NULL);
        return (constant ? "const " : "") + name + " to " + refSymbol_->GetQualifiedName();
    }

    SymbolArray::SymbolArray()
        : SymbolType("array")
    {

    }

    ESymbolType SymbolArray::GetSymbolType() const
    {
        return ESymbolType::TYPE_ARRAY;
    }

    void SymbolArray::SetInitializer(ASTNode* initializerExpression)
    {
        assert(initializerExpression != NULL);
        sizeInitializer_ = initializerExpression;
    }

    void SymbolArray::SetTypeSymbol(SymbolType* symType)
    {
        assert(symType != NULL);
        ESymbolType type = symType->GetSymbolType();
        if (type == ESymbolType::TYPE_FUNCTION
            || type == ESymbolType::TYPE_VOID)
        {
            throw std::logic_error("type of an array elements can't be either void or function");
        }
        elementType_ = symType;
    }

    std::string SymbolArray::GetQualifiedName() const
    {
        assert(elementType_ != NULL);
        std::string size;
        if (sizeInitializer_->token == TT_LITERAL_INT)
        {
            size = " " + sizeInitializer_->token.text;
        }
        return name + size + " of " + elementType_->GetQualifiedName();
    }

} // namespace Compiler
