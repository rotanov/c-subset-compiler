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
        using namespace std;

        cout << asmHeader;

        shared_ptr<SymbolTable> internalSymbols = GetInternalSymbolTable();
        shared_ptr<SymbolTable> globalSymbols = GetGlobalSymbolTable();

        for (auto& f : globalSymbols->functions)
        {
            cout << "PUBLIC _" << f.second->name << endl;
        }
        if (globalSymbols->functions.size() > 0)
        {
            cout << endl;
        }

        if (globalSymbols->variables.size() > 0)
        {
            // TODO: take order into account
            cout << "_DATA SEGMENT" << endl;
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
                        stringstream ss;
                        ss << hex << size;
                        sizeName += ":0" + ss.str() + "H";
                    }
                }

                cout << "COMM _" << v->name << ":" << sizeName << endl;
            }
            cout << "_DATA ENDS" << endl;
        }

        if (stringTable_.size() > 0)
        {
            cout << "_DATA SEGMENT" << endl;

            int size = 1000;
            for (auto& s : stringTable_)
            {
                Token& token = s->token;
                cout << "$SG" << size << " ";
                for (int i = 0; i < token.size; i++)
                {
                    stringstream ss;
                    unsigned value = static_cast<unsigned char>(token.charValue[i]);
                    ss << hex << value;
                    cout << "DB 0" << ss.str() << "H" << endl;
                }
                size += token.size;
                int pad = (4 - size % 4) * (size % 4 != 0);
                size += pad;
                if (pad != 0)
                {
                    cout << "ORG $+" << pad << endl;
                }
            }

            cout << "_DATA ENDS" << endl;
        }

        cout << asmFooter;
    }

} // namespace Compiler
