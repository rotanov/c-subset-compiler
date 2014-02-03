#include "codegen.hpp"

namespace Compiler
{

    CodeGenerator::CodeGenerator()
        : Parser()
    {

    }

    CodeGenerator::~CodeGenerator()
    {

    }

    void CodeGenerator::Flush() const
    {
        std::cout << asmHeader;

        shared_ptr<SymbolTable> internalSymbols = GetInternalSymbolTable();
        shared_ptr<SymbolTable> globalSymbols = GetGlobalSymbolTable();

        for (auto& f : globalSymbols->functions)
        {
            std::cout << "PUBLIC _" << f.second->name << std::endl;
        }
        if (globalSymbols->functions.size() > 0)
        {
            std::cout << std::endl;
        }

        if (globalSymbols->variables.size() > 0)
        {
            // order ignored
            std::cout << "_DATA SEGMENT" << std::endl;
            for (auto& f : globalSymbols->variables)
            {
                // TODO: initializer present case
                auto& v = f.second;
                auto t = GetActualType(v);
                string sizeName;
                if (t->GetType() == ESymbolType::TYPE_CHAR)
                {
                    sizeName = "BYTE";
                }
                else if (t->GetType() == ESymbolType::TYPE_INT
                         || t->GetType() == ESymbolType::TYPE_FLOAT
                         || t->GetType() == ESymbolType::TYPE_POINTER)
                {
                    sizeName = "DWORD";
                }
                else
                {
                    int size = t->GetSize();
                    if (t->GetType() == ESymbolType::TYPE_ARRAY)
                    {
                        shared_ptr<SymbolType> arrayTypeSymbol = GetArrayType(t);
                        ESymbolType arrayType = arrayTypeSymbol->GetType();
                        if (arrayType == ESymbolType::TYPE_INT
                            || arrayType == ESymbolType::TYPE_FLOAT)
                        {
                            sizeName = "DWORD";
                            size /= arrayTypeSymbol->GetSize();
                        }
                        else
                        {
                            sizeName = "BYTE";
                        }
                    }
                    else
                    {
                        sizeName = "BYTE";
                    }
                    if (size != 1)
                    {
                        std::stringstream ss;
                        ss << std::hex << size;
                        sizeName += ":0" + ss.str() + "H";
                    }
                }

                std::cout << "COMM _" << v->name << ":" << sizeName << std::endl;
            }
            std::cout << "_DATA ENDS" << std::endl;
        }

        std::cout << asmFooter;
    }

} // namespace Compiler
