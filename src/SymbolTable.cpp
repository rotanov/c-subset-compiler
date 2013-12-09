#include "SymbolTable.hpp"

#include "ASTNode.hpp"
#include "Statement.hpp"

#include <iostream>

namespace Compiler
{
    Symbol::Symbol(const std::string& name)
        : name(name)
    {

    }

    Symbol::~Symbol()
    {

    }

    void Symbol::SetTypeSymbol(shared_ptr<SymbolType> /*symType*/)
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

    void SymbolTable::AddType(shared_ptr<SymbolType> symbolType)
    {
        assert(symbolType != NULL);
        //        if (symbolType->GetSymbolType() == ESymbolType::TYPE_STRUCT)
        //        {
        //            key = "struct " + key;
        //        }
        AddType(symbolType, symbolType->name);
    }

    void SymbolTable::AddType(shared_ptr<SymbolType> symbolType, const std::string& key)
    {
        assert(symbolType != NULL);
        assert(types.find(key) == types.end());
        types[key] = symbolType;
    }

    void SymbolTable::AddFunction(shared_ptr<SymbolVariable> symbolVariable)
    {
        assert(symbolVariable != NULL);
        assert(symbolVariable->GetTypeSymbol()->GetSymbolType() == ESymbolType::TYPE_FUNCTION);

        std::string key = symbolVariable->name;

        assert(functions.find(key) == functions.end());

        functions[key] = symbolVariable;
    }

    void SymbolTable::AddVariable(shared_ptr<SymbolVariable> symbolVariable)
    {
        assert(symbolVariable != NULL);

        std::string key = symbolVariable->name;

        // wrong assert since we have forward declarations
        // place proper checks at higher level
        // assert(variables.find(key) == variables.end());

        variables[key] = symbolVariable;
    }

    shared_ptr<SymbolVariable> SymbolTable::LookupVariable(const std::string& name) const
    {
        return LookupHelper_(variables, name);
    }

    shared_ptr<SymbolType> SymbolTable::LookupType(const std::string& name) const
    {
        return LookupHelper_(types, name);
    }

    shared_ptr<SymbolVariable> SymbolTable::LookupFunction(const std::string& name) const
    {
        return LookupHelper_(functions, name);
    }

    SymbolType::SymbolType(const std::string& name)
        : Symbol(name)
    {

    }

    std::string SymbolType::GetQualifiedName() const
    {
        return name;
    }

    SymbolVariable::SymbolVariable(const std::string& name)
        : Symbol(name)
    {

    }

    SymbolVariable::SymbolVariable(const std::string& name, shared_ptr<SymbolType> symType)
        : Symbol(name)
    {
        SetTypeSymbol(symType);
    }

    ESymbolType SymbolVariable::GetSymbolType() const
    {
        return ESymbolType::VARIABLE;
    }

    void SymbolVariable::SetTypeSymbol(shared_ptr<SymbolType> symType)
    {
        assert(type_ == NULL);
        type_ = symType;
    }

    shared_ptr<SymbolType> SymbolVariable::GetTypeSymbol() const
    {
        assert(type_ != NULL);
        return type_;
    }

    std::string SymbolVariable::GetQualifiedName() const
    {
        assert(type_ != NULL);
        return "variable " + name + " of type " + type_->GetQualifiedName();
    }

    void SymbolVariable::SetInitializer(shared_ptr<ASTNode> initializer)
    {
        initializer_ = initializer;
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

    SymbolFunctionType::SymbolFunctionType(shared_ptr<SymbolTableWithOrder> parametersSymTable)
        : SymbolType("function")
        , parameters_(parametersSymTable)
    {
        assert(parameters_ != NULL);
    }

    ESymbolType SymbolFunctionType::GetSymbolType() const
    {
        return ESymbolType::TYPE_FUNCTION;
    }

    void SymbolFunctionType::SetTypeSymbol(shared_ptr<SymbolType> symType)
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
            argStr += a->GetQualifiedName();
            if (a != parameters_->orderedVariables.back())
            {
                argStr += ", ";
            }
        }
        return "function(" + argStr + ") returning " + returnType_->GetQualifiedName();
    }

    void SymbolFunctionType::AddParameter(shared_ptr<SymbolVariable> parameter)
    {
        assert(parameter != NULL);
        parameters_->AddVariable(parameter);
    }

    shared_ptr<SymbolTableWithOrder> SymbolFunctionType::GetSymbolTable() const
    {
        assert(parameters_ != NULL);
        return parameters_;
    }

    void SymbolFunctionType::SetBody(shared_ptr<CompoundStatement> body)
    {
        assert(body_ == NULL);
        assert(body != NULL);
        body_ = body;
    }

    shared_ptr<CompoundStatement> SymbolFunctionType::GetBody() const
    {
//        assert(body_ != NULL);
        return body_;
    }

    SymbolStruct::SymbolStruct(const std::string name)
        : SymbolType(name)
    {
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

    void SymbolStruct::AddField(shared_ptr<SymbolVariable> field)
    {
        assert(field != NULL);
        assert(fields_ != NULL);
        fields_->AddVariable(field);
    }

    shared_ptr<SymbolTableWithOrder> SymbolStruct::GetSymbolTable() const
    {
        assert(fields_ != NULL);
        return fields_;
    }

    void SymbolStruct::SetFieldsSymTable(shared_ptr<SymbolTableWithOrder> fieldsSymTable)
    {
        assert(fieldsSymTable != NULL);
        assert(fields_ == NULL);
        fields_ = fieldsSymTable;
    }

    SymbolPointer::SymbolPointer()
        : SymbolType("pointer")
    {
    }

    SymbolPointer::SymbolPointer(shared_ptr<SymbolType> symType)
        : SymbolType("pointer")
    {
        SetTypeSymbol(symType);
    }

    shared_ptr<SymbolType> SymbolPointer::GetRefSymbol() const
    {
        return refSymbol_;
    }

    ESymbolType SymbolPointer::GetSymbolType() const
    {
        return ESymbolType::TYPE_POINTER;
    }

    void SymbolPointer::SetTypeSymbol(shared_ptr<SymbolType> symType)
    {
        assert(symType != NULL);
        refSymbol_ = symType;
    }

    std::string SymbolPointer::GetQualifiedName() const
    {
        assert(refSymbol_ != NULL);
        return name + " to " + refSymbol_->GetQualifiedName();
    }

    SymbolArray::SymbolArray()
        : SymbolType("array")
    {

    }

    ESymbolType SymbolArray::GetSymbolType() const
    {
        return ESymbolType::TYPE_ARRAY;
    }

    void SymbolArray::SetSizeInitializer(shared_ptr<ASTNode> initializerExpression)
    {
        assert(initializerExpression != NULL);
        sizeInitializer_ = initializerExpression;
    }

    void SymbolArray::SetTypeSymbol(shared_ptr<SymbolType> symType)
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
        if (sizeInitializer_ != NULL
            && sizeInitializer_->token == TT_LITERAL_INT)
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

    void SymbolTableWithOrder::AddVariable(shared_ptr<SymbolVariable> symbolVariable)
    {
        assert(symbolVariable != NULL);

        std::string key = symbolVariable->name;

        assert(variables.find(key) == variables.end());

        variables[key] = symbolVariable;
        orderedVariables.push_back(symbolVariable);
    }

    SymbolConst::SymbolConst()
        : SymbolType("const")
    {

    }

    SymbolConst::SymbolConst(shared_ptr<SymbolType> symType)
        : SymbolType("const")
    {
        SetTypeSymbol(symType);
    }

    shared_ptr<SymbolType> SymbolConst::GetRefSymbol() const
    {
        return refSymbol_;
    }

    ESymbolType SymbolConst::GetSymbolType() const
    {
        return ESymbolType::TYPE_CONST;
    }

    void SymbolConst::SetTypeSymbol(shared_ptr<SymbolType> symType)
    {
        assert(symType != NULL);
        refSymbol_ = symType;
    }

    std::string SymbolConst::GetQualifiedName() const
    {
        // name is always "const" here
        assert(refSymbol_ != NULL);
        return name + " " + refSymbol_->GetQualifiedName();
    }

    SymbolTypedef::SymbolTypedef(const std::string& name)
        : SymbolType(name)
    {

    }

    shared_ptr<SymbolType> SymbolTypedef::GetTypeSymbol() const
    {
        return type_;
    }

    ESymbolType SymbolTypedef::GetSymbolType() const
    {
        return ESymbolType::TYPE_TYPEDEF;
    }

    void SymbolTypedef::SetTypeSymbol(shared_ptr<SymbolType> symType)
    {
        assert(symType != NULL);
        type_ = symType;
    }

    std::string SymbolTypedef::GetQualifiedName() const
    {
        return "typedef " + name + " " + type_->GetQualifiedName();
    }

} // namespace Compiler
