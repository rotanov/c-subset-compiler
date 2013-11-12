#include "SymbolTable.hpp"

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

    SymbolTable::~SymbolTable()
    {

    }

    void SymbolTable::AddType(SymbolType* symbolType)
    {
        assert(symbolType != NULL);

        std::string key = symbolType->name;
        if (symbolType->IsStruct())
        {
            key = "struct " + key;
        }

        assert(types_.find(key) == types_.end());

        types_[key] = symbolType;
    }

    void SymbolTable::AddFunction(SymbolFunction* symbolFunction)
    {
        assert(symbolFunction != NULL);

        std::string key = symbolFunction->name;

        assert(types_.find(key) == types_.end());

        functions_[key] = symbolFunction;
    }

    void SymbolTable::AddVariable(SymbolVariable* symbolVariable)
    {
        assert(symbolVariable != NULL);

        std::string key = symbolVariable->name;

        assert(types_.find(key) == types_.end());

        variables_[key] = symbolVariable;
    }

    SymbolVariable*SymbolTable::LookupVariable(const std::string& name) const
    {
        return LookupHelper_(variables_, name);
    }

    SymbolType*SymbolTable::LookupType(const std::string& name) const
    {
        return LookupHelper_(types_, name);
    }

    SymbolFunction*SymbolTable::LookupFunction(const std::string& name) const
    {
        return LookupHelper_(functions_, name);
    }

    SymbolType::SymbolType(const std::string& name)
        : Symbol(name)
    {

    }

    bool SymbolType::IsStruct() const
    {
        return false;
    }

    ESymbolType SymbolVariable::GetSymbolType() const
    {
        return ESymbolType::VARIABLE;
    }

    ESymbolType SymbolFunction::GetSymbolType() const
    {
        return ESymbolType::FUNCTION;
    }

    bool SymbolStruct::IsStruct() const
    {
        return true;
    }

    SymbolChar::SymbolChar()
        : SymbolType("char")
    {

    }

    SymbolInt::SymbolInt()
        : SymbolType("int")
    {

    }

    SymbolFloat::SymbolFloat()
        : SymbolType("float")
    {

    }

    SymbolVoid::SymbolVoid()
        : SymbolType("void")
    {

    }









} // namespace Compiler
