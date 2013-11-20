#include "SymbolTable.hpp"

#include "ASTNode.hpp"
#include "Statement.hpp"

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

    SymbolTable::SymbolTable()
    {

    }

    SymbolTable::SymbolTable(const EScopeType scope)
        : scope_(scope)
    {
        assert(scope != EScopeType::UNDEFINED
                && scope != EScopeType::PARAMETERS
                && scope != EScopeType::STRUCTURE);
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

    void SymbolTable::AddFunction(SymbolVariable* symbolVariable)
    {
        assert(symbolVariable != NULL);
        assert(symbolVariable->GetTypeSymbol()->GetSymbolType() == ESymbolType::TYPE_FUNCTION);

        std::string key = symbolVariable->name;

        assert(functions.find(key) == functions.end());

        functions[key] = symbolVariable;
    }

    void SymbolTable::AddVariable(SymbolVariable* symbolVariable)
    {
        assert(symbolVariable != NULL);

        std::string key = symbolVariable->name;

        assert(variables.find(key) == variables.end());

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

    SymbolVariable* SymbolTable::LookupFunction(const std::string& name) const
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

    SymbolType* SymbolVariable::GetTypeSymbol() const
    {
        assert(type_ != NULL);
        return type_;
    }

    std::string SymbolVariable::GetQualifiedName() const
    {
        assert(type_ != NULL);
        return "variable " + name + " of type " + type_->GetQualifiedName();
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

    SymbolFunctionType::SymbolFunctionType(SymbolTableWithOrder* parametersSymTable)
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
        for (auto& a : parameters_->orderedVariables)
        {
            argStr += a->GetQualifiedName() + ", ";
        }
        return "function(" + argStr + ") returning " + returnType_->GetQualifiedName();
    }

    void SymbolFunctionType::AddParameter(SymbolVariable* parameter)
    {
        assert(parameter != NULL);
        parameters_->AddVariable(parameter);
    }

    SymbolTableWithOrder* SymbolFunctionType::GetSymbolTable() const
    {
        assert(parameters_ != NULL);
        return parameters_;
    }

    void SymbolFunctionType::SetBody(CompoundStatement* body)
    {
        assert(body_ == NULL);
        assert(body != NULL);
        body_ = body;
    }

    CompoundStatement* SymbolFunctionType::GetBody() const
    {
//        assert(body_ != NULL);
        return body_;
    }

    SymbolStruct::SymbolStruct(SymbolTableWithOrder* membersSymTable, const std::string name)
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
//        std::string membersStr;
//        if (fields_ != NULL)
//        {
//            for (auto& s : fields_->variables)
//            {
//                membersStr += "\n    " + s.second->GetQualifiedName();
//            }
//        }
        return "struct " + name;// + membersStr + "\n";
    }

    void SymbolStruct::AddField(SymbolVariable* field)
    {
        assert(field != NULL);
        fields_->AddVariable(field);
    }

    SymbolTableWithOrder* SymbolStruct::GetSymbolTable() const
    {
        assert(fields_ != NULL);
        return fields_;
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

    SymbolTableWithOrder::SymbolTableWithOrder(const EScopeType scope)
        : SymbolTable()
    {
        assert(scope == EScopeType::PARAMETERS
               || scope == EScopeType::STRUCTURE);
        scope_ = scope;
    }

    SymbolTableWithOrder::~SymbolTableWithOrder()
    {

    }

    void SymbolTableWithOrder::AddVariable(SymbolVariable* symbolVariable)
    {
        assert(symbolVariable != NULL);

        std::string key = symbolVariable->name;

        assert(variables.find(key) == variables.end());

        variables[key] = symbolVariable;
        orderedVariables.push_back(symbolVariable);
    }

} // namespace Compiler
