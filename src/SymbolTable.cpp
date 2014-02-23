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
        assert(symbolVariable->GetRefSymbol()->GetType() == ESymbolType::TYPE_FUNCTION);

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

    int SymbolType::GetAbsoluteElementCount()
    {
        assert(false);
        return 0;
    }

    SymbolVariable::SymbolVariable(const std::string& name)
        : SymbolTypeRef(name)
    {

    }

    SymbolVariable::SymbolVariable(const std::string& name, shared_ptr<SymbolType> symType)
        : SymbolTypeRef(name, symType)
    {
    }

    ESymbolType SymbolVariable::GetType() const
    {
        return ESymbolType::VARIABLE;
    }

    std::string SymbolVariable::GetQualifiedName() const
    {
        assert(type_ != NULL);
        return "variable " + name + " of type " + type_->GetQualifiedName();
    }

    bool SymbolVariable::IfTypeFits(shared_ptr<Symbol> symbol) const
    {
        if (symbol->GetType() == ESymbolType::VARIABLE)
        {
            assert(type_ != NULL);
            return type_->IfTypeFits(static_pointer_cast<SymbolVariable>(symbol)->GetRefSymbol());
        }
        else
        {
            return false;
        }
    }

    void SymbolVariable::PushInitializer(shared_ptr<ASTNode> initializer)
    {
        assert(initializer != NULL);
        initializers_.push_back(initializer);
    }

    int SymbolVariable::GetAbsoluteElementCount()
    {
        auto typeSym = GetActualType(shared_from_this());

        switch (typeSym->GetType())
        {
            case ESymbolType::TYPE_ARRAY:
            case ESymbolType::TYPE_STRUCT:
                return typeSym->GetAbsoluteElementCount();

            default:
                return 1;
        }
    }

    SymbolChar::SymbolChar()
        : SymbolType("char")
    {

    }

    ESymbolType SymbolChar::GetType() const
    {
        return ESymbolType::TYPE_CHAR;
    }

    bool SymbolChar::IfTypeFits(shared_ptr<Symbol> symbol) const
    {
        return symbol->GetType() == ESymbolType::TYPE_CHAR;
    }

    int SymbolChar::GetSize()
    {
        return 1;
    }

    SymbolInt::SymbolInt()
        : SymbolType("int")
    {

    }

    ESymbolType SymbolInt::GetType() const
    {
        return ESymbolType::TYPE_INT;
    }

    bool SymbolInt::IfTypeFits(shared_ptr<Symbol> symbol) const
    {
        return symbol->GetType() == ESymbolType::TYPE_INT;
        // || symbol->GetSymbolType() == ESymbolType::TYPE_CHAR;
    }

    int SymbolInt::GetSize()
    {
        return 4;
    }

    SymbolFloat::SymbolFloat()
        : SymbolType("float")
    {

    }

    ESymbolType SymbolFloat::GetType() const
    {
        return ESymbolType::TYPE_FLOAT;
    }

    bool SymbolFloat::IfTypeFits(shared_ptr<Symbol> symbol) const
    {
        return symbol->GetType() == ESymbolType::TYPE_FLOAT;
               //|| symbol->GetSymbolType() == ESymbolType::TYPE_INT
        //|| symbol->GetSymbolType() == ESymbolType::TYPE_CHAR;
    }

    int SymbolFloat::GetSize()
    {
        return 4;
    }

    SymbolVoid::SymbolVoid()
        : SymbolType("void")
    {

    }

    ESymbolType SymbolVoid::GetType() const
    {
        return ESymbolType::TYPE_VOID;
    }

    bool SymbolVoid::IfTypeFits(shared_ptr<Symbol> symbol) const
    {
        return symbol->GetType() == ESymbolType::TYPE_VOID;
    }

    int SymbolVoid::GetSize()
    {
        assert(false);
        return 0;
    }

    SymbolFunctionType::SymbolFunctionType(shared_ptr<SymbolTableWithOrder> parametersSymTable)
        : SymbolTypeRef("function")
        , parameters_(parametersSymTable)
    {
        assert(parameters_ != NULL);
    }

    SymbolFunctionType::SymbolFunctionType(shared_ptr<SymbolType> returnType, shared_ptr<SymbolTableWithOrder> parametersSymTable)
        : SymbolTypeRef("function", returnType)
        , parameters_(parametersSymTable)
    {
        assert(parameters_ != NULL);
    }

    ESymbolType SymbolFunctionType::GetType() const
    {
        return ESymbolType::TYPE_FUNCTION;
    }

    std::string SymbolFunctionType::GetQualifiedName() const
    {
        assert(type_ != NULL);
        std::string argStr;
        for (auto& a : parameters_->orderedVariables)
        {
            argStr += a->GetQualifiedName();
            if (a != parameters_->orderedVariables.back())
            {
                argStr += ", ";
            }
        }
        return "function(" + argStr + ") returning " + type_->GetQualifiedName();
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

    bool SymbolFunctionType::IfTypeFits(shared_ptr<Symbol> symbol) const
    {
        if (symbol->GetType() == ESymbolType::TYPE_FUNCTION)
        {
            shared_ptr<SymbolFunctionType> funSym = static_pointer_cast<SymbolFunctionType>(symbol);
            bool r = type_->IfTypeFits(funSym->GetRefSymbol());
            r = r && (parameters_->orderedVariables.size()
                      == funSym->GetSymbolTable()->orderedVariables.size());

            for (unsigned i = 0; r && i < parameters_->orderedVariables.size(); i++)
            {
                r = r && (parameters_->orderedVariables[i]->IfTypeFits(funSym->GetSymbolTable()->orderedVariables[i]));
            }
            return r;
        }
        else
        {
            return false;
        }
    }

    SymbolStruct::SymbolStruct(const std::string name)
        : SymbolType(name)
        , size_(-1)
    {
    }

    ESymbolType SymbolStruct::GetType() const
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

    bool SymbolStruct::IfTypeFits(shared_ptr<Symbol> symbol) const
    {
        // TODO: implement
        return false;
    }

    int SymbolStruct::GetSize()
    {
        if (size_ != -1)
        {
            return size_;
        }
        else
        {
            int& size = const_cast<SymbolStruct*>(this)->size_;
            size = 0;
            auto vars = const_cast<SymbolStruct*>(this)->fields_->orderedVariables;
            for (int i = 0; i < vars.size(); i++)
            {
                auto v = vars[i];
                auto sizeIncrement = GetActualType(v)->GetSize();
                sizeIncrement += (4 - sizeIncrement % 4) * (sizeIncrement % 4 != 0);
                size += sizeIncrement;
            }
            return size;
        }
    }

    int SymbolStruct::GetAbsoluteElementCount()
    {
        int r = 0;

        if (fields_ != NULL)
        {
            auto vars = fields_->orderedVariables;

            for (int i = 0; i < vars.size(); i++)
            {
                r += vars[i]->GetAbsoluteElementCount();
            }
        }

        return r;
    }

    SymbolPointer::SymbolPointer()
        : SymbolTypeRef("pointer")
    {
    }

    SymbolPointer::SymbolPointer(shared_ptr<SymbolType> symType)
        : SymbolTypeRef("pointer", symType)
    {
    }

    ESymbolType SymbolPointer::GetType() const
    {
        return ESymbolType::TYPE_POINTER;
    }

    std::string SymbolPointer::GetQualifiedName() const
    {
        assert(type_ != NULL);
        return name + " to " + type_->GetQualifiedName();
    }

    bool SymbolPointer::IfTypeFits(shared_ptr<Symbol> symbol) const
    {
        return symbol->GetType() == ESymbolType::TYPE_POINTER
               && type_->IfTypeFits(static_pointer_cast<SymbolPointer>(symbol)->GetRefSymbol());
    }

    SymbolArray::SymbolArray()
        : SymbolTypeRef("array")
    {

    }

    SymbolArray::SymbolArray(shared_ptr<SymbolType> symType)
        : SymbolTypeRef("array", symType)
    {

    }

    ESymbolType SymbolArray::GetType() const
    {
        return ESymbolType::TYPE_ARRAY;
    }

    void SymbolArray::SetSizeInitializer(shared_ptr<ASTNode> initializerExpression)
    {
        assert(initializerExpression != NULL);

        if (!initializerExpression->IsConstExpr())
        {
            ThrowInvalidTokenError(initializerExpression->token, "size initializer have to be constant expression");
        }

        sizeInitializer_ = initializerExpression;
        elementCount_ = sizeInitializer_->EvalToInt();

        if (elementCount_ <= 0)
        {
            ThrowInvalidTokenError(initializerExpression->token, "size of an array must be >= 0");
        }
    }

    int SymbolArray::GetSize()
    {
        assert(elementCount_ != 0);
        if (size_ == -1)
        {
            size_ = elementCount_ * GetArrayType(this->shared_from_this())->GetSize();
        }
        return size_;
    }

    int SymbolArray::GetElementCount() const
    {
        return elementCount_;
    }

    int SymbolArray::GetAbsoluteElementCount()
    {
        int r = this->GetElementCount();
        auto symType = Compiler::GetRefSymbol(this->shared_from_this());

        switch (symType->GetType())
        {
            case ESymbolType::TYPE_ARRAY:
            case ESymbolType::TYPE_STRUCT:
                return r * symType->GetAbsoluteElementCount();

            default:
                return r;
        }
    }

    std::string SymbolArray::GetQualifiedName() const
    {
        assert(type_ != NULL);
        std::string size;
        if (sizeInitializer_ != NULL)
        {
            size = " " + std::to_string(sizeInitializer_->EvalToInt());
        }
        return name + size + " of " + type_->GetQualifiedName();
    }

    bool SymbolArray::IfTypeFits(shared_ptr<Symbol> symbol) const
    {
        // TODO: add dims check
        return symbol->GetType() == ESymbolType::TYPE_ARRAY
                && type_->IfTypeFits(static_pointer_cast<SymbolArray>(symbol)->GetRefSymbol());
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
        : SymbolTypeRef("const")
    {
    }

    SymbolConst::SymbolConst(shared_ptr<SymbolType> symType)
        : SymbolTypeRef("const", symType)
    {
    }

    ESymbolType SymbolConst::GetType() const
    {
        return ESymbolType::TYPE_CONST;
    }

    std::string SymbolConst::GetQualifiedName() const
    {
        // name is always "const" here
        assert(type_ != NULL);
        return name + " " + type_->GetQualifiedName();
    }

    bool SymbolConst::IfTypeFits(shared_ptr<Symbol> symbol) const
    {
        return symbol->GetType() == ESymbolType::TYPE_CONST
                && type_->IfTypeFits(static_pointer_cast<SymbolConst>(symbol)->GetRefSymbol());
    }

    SymbolTypedef::SymbolTypedef(const std::string& name)
        : SymbolTypeRef(name)
    {

    }

    ESymbolType SymbolTypedef::GetType() const
    {
        return ESymbolType::TYPE_TYPEDEF;
    }

    std::string SymbolTypedef::GetQualifiedName() const
    {
        return "typedef " + name + " " + type_->GetQualifiedName();
    }

    bool SymbolTypedef::IfTypeFits(shared_ptr<Symbol> symbol) const
    {
        return symbol->GetType() == ESymbolType::TYPE_TYPEDEF
                && type_->IfTypeFits(static_pointer_cast<SymbolTypedef>(symbol)->GetRefSymbol());
    }

    SymbolTypeRef::SymbolTypeRef(const std::string& name)
        : SymbolType(name)
    {

    }

    SymbolTypeRef::SymbolTypeRef(const std::string& name, shared_ptr<SymbolType> type)
        : SymbolType(name)
        , type_(type)
    {
        assert(type != NULL);
    }

    void SymbolTypeRef::SetRefSymbol(shared_ptr<SymbolType> type)
    {
        assert(type_ == NULL);
        assert(type != NULL);

        // TODO: organize these check properly

        if (this->GetType() == ESymbolType::TYPE_ARRAY)
        {
            ESymbolType symType = type->GetType();
            if (symType == ESymbolType::TYPE_FUNCTION
                || symType == ESymbolType::TYPE_VOID)
            {
                throw std::logic_error("type of an array elements can't be either void or function");
            }
        }

        if (this->GetType() == ESymbolType::TYPE_FUNCTION)
        {
            ESymbolType symType = type->GetType();
            if (symType == ESymbolType::TYPE_ARRAY
                || symType == ESymbolType::TYPE_FUNCTION)
            {
                // TODO: drag column and line somehow
                throw std::logic_error("function return type can't be either array or function");
            }
        }

        type_ = type;
    }

//------------------------------------------------------------------------------
    shared_ptr<SymbolType> SymbolTypeRef::GetRefSymbol() const
    {
        assert(type_ != NULL);
        return type_;
    }

//------------------------------------------------------------------------------
    int SymbolTypeRef::GetSize()
    {
        return GetActualType(const_cast<SymbolTypeRef*>(this)->shared_from_this())->GetSize();
    }

//------------------------------------------------------------------------------

    bool IfSymbolIsRef(shared_ptr<Symbol> symbol)
    {
        switch (symbol->GetType())
        {
            case ESymbolType::TYPE_ARRAY:
            case ESymbolType::TYPE_CONST:
            case ESymbolType::TYPE_FUNCTION:
            case ESymbolType::TYPE_POINTER:
            case ESymbolType::TYPE_TYPEDEF:
            case ESymbolType::VARIABLE:
                return true;

            default:
                return false;
        }
    }

    bool IfArithmetic(shared_ptr<SymbolType> symbol)
    {
        switch (symbol->GetType())
        {
            case ESymbolType::TYPE_CHAR:
            case ESymbolType::TYPE_FLOAT:
            case ESymbolType::TYPE_INT:
                return true;

            case ESymbolType::TYPE_TYPEDEF:
            case ESymbolType::TYPE_CONST:
            case ESymbolType::VARIABLE:
                return IfArithmetic(GetRefSymbol(symbol));

            default:
                return false;
        }
    }

    bool IfScalar(shared_ptr<SymbolType> symbol)
    {
        return IfOfType(symbol, ESymbolType::TYPE_POINTER)
               || IfArithmetic(symbol);
    }

    shared_ptr<SymbolType> GetRefSymbol(shared_ptr<Symbol> symbol)
    {
        assert(IfSymbolIsRef(symbol));
        return static_pointer_cast<SymbolTypeRef>(symbol)->GetRefSymbol();
    }

    bool IfInteger(shared_ptr<SymbolType> symbol)
    {
        switch (symbol->GetType())
        {
            case ESymbolType::TYPE_CHAR:
            case ESymbolType::TYPE_INT:
                return true;

            case ESymbolType::TYPE_TYPEDEF:
            case ESymbolType::TYPE_CONST:
            case ESymbolType::VARIABLE:
                return IfInteger(static_pointer_cast<SymbolTypeRef>(symbol)->GetRefSymbol());

            default:
                return false;
        }
    }

    shared_ptr<SymbolType> CalcCommonArithmeticType(shared_ptr<SymbolType> left, shared_ptr<SymbolType> right)
    {
        assert(IfArithmetic(left) && IfArithmetic(right));

        if (IfOfType(left, ESymbolType::TYPE_FLOAT)
            || IfOfType(right, ESymbolType::TYPE_FLOAT))
        {
            return make_shared<SymbolFloat>();
        }
        else if (IfOfType(left, ESymbolType::TYPE_INT)
                 || IfOfType(right, ESymbolType::TYPE_INT))
        {
            return make_shared<SymbolInt>();
        }
        else
        {
            return make_shared<SymbolChar>();
        }
    }

    bool IfOfType(shared_ptr<SymbolType> symbol, ESymbolType type)
    {
        auto symType = symbol->GetType();

        switch (symType)
        {
            case ESymbolType::TYPE_TYPEDEF:
            case ESymbolType::TYPE_CONST:
            case ESymbolType::VARIABLE:
                return IfOfType(GetRefSymbol(symbol), type);

            default:
                return symType == type;
        }
    }

    shared_ptr<SymbolType> GetActualType(shared_ptr<SymbolType> symbol)
    {
        auto symType = symbol->GetType();

        switch (symType)
        {
            case ESymbolType::TYPE_TYPEDEF:
            case ESymbolType::TYPE_CONST:
            case ESymbolType::VARIABLE:
                return GetActualType(GetRefSymbol(symbol));

            default:
                return symbol;
        }
    }

    shared_ptr<SymbolType> GetArrayType(shared_ptr<SymbolType> symbol)
    {
        assert(symbol->GetType() == ESymbolType::TYPE_ARRAY);
        shared_ptr<SymbolArray> arraySymbol = static_pointer_cast<SymbolArray>(symbol);
        shared_ptr<SymbolType> refTypeSymbol = GetActualType(arraySymbol->GetRefSymbol());
        if (refTypeSymbol->GetType() != ESymbolType::TYPE_ARRAY)
        {
            return refTypeSymbol;
        }
        else
        {
            return GetArrayType(refTypeSymbol);
        }
    }

    bool IfConst(shared_ptr<SymbolType> symbol)
    {
        assert(symbol != NULL);

        switch (symbol->GetType())
        {
            case ESymbolType::TYPE_CONST:
                return true;

            case ESymbolType::TYPE_ARRAY:
            case ESymbolType::TYPE_FUNCTION:
            case ESymbolType::TYPE_TYPEDEF:
            case ESymbolType::VARIABLE:
                return IfConst(GetRefSymbol(symbol));

            case ESymbolType::TYPE_STRUCT:
            {
                auto symStruct = static_pointer_cast<SymbolStruct>(symbol);

                for (auto f : symStruct->GetSymbolTable()->orderedVariables)
                {
                    if (IfConst(f))
                    {
                        return true;
                    }
                }
                return false;
            }

            default:
                return false;
        }
    }


} // namespace Compiler
