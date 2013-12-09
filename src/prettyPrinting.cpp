#include "prettyPrinting.hpp"

#include <functional>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <iostream>

#include "ASTNode.hpp"
#include "SymbolTable.hpp"
#include "Statement.hpp"

namespace Compiler
{
    void PrintAST(ASTNode* root, int indentLevel)
    {
        auto print = [&]() -> decltype(std::operator <<(std::cout, std::string()))
        {
            return std::cout << std::string(indentLevel * 2, ' ');
        };

        struct PrintTreeNode
        {
            std::string text;
            std::vector<PrintTreeNode*> children;
            unsigned depth = 0;
        };

        std::vector<int> offsetByDepth;
        offsetByDepth.resize(1);
        offsetByDepth[0] = 0;
        unsigned depth = 0;
        const int spacing = 2;

        std::function<void(ASTNode*, PrintTreeNode*, int)> f =
                [&](ASTNode* next, PrintTreeNode* print, int estimate)
        {
            assert(next != NULL);

            if (offsetByDepth.size() < depth + 1)
            {
                offsetByDepth.resize(depth + 1, 0);
            }

            int spaceCount = estimate - offsetByDepth[depth];
            spaceCount = spaceCount == 1 ? 2 : spaceCount;

            if (next->GetChildCount() == 0)
            {
                assert(print != NULL);
                print->text = std::string(spaceCount, ' ') + next->token.text;
                print->depth = depth;
                offsetByDepth[depth] += next->token.text.size() + spaceCount;
            }
            else
            {
                int maxLeft = -1;
                int prevMaxLeft = -1;
                for (int i = 0; i < next->GetChildCount(); i++)
                {
                    PrintTreeNode* printNode = new PrintTreeNode;
                    print->children.push_back(printNode);

                    depth++;
                    f(next->GetChild(i).get(), printNode, (maxLeft + spacing) * (i != 0) + estimate * (i == 0));
                    prevMaxLeft = maxLeft;
                    maxLeft = *std::max_element(offsetByDepth.begin() + depth,
                                                    offsetByDepth.end());
                    depth--;
                }

                print->depth = depth;

                if (prevMaxLeft != -1)
                {
                    int decorationCount = prevMaxLeft + spacing - estimate
                                          - (next->token.text.size() - 1);

                    print->text = std::string(spaceCount, ' ')
                                  + next->token.text
                                  + std::string(decorationCount, '-');

                    offsetByDepth[depth] += next->token.text.size() + prevMaxLeft
                                            + spacing - estimate + spaceCount - (next->token.text.size() - 1);
                }
                else
                {
                    print->text = std::string(spaceCount, ' ')
                                  + next->token.text;
                    offsetByDepth[depth] += next->token.text.size() + spaceCount;
                }
            }
        };

        PrintTreeNode printRoot;
        f(root, &printRoot, 0);

        std::queue<PrintTreeNode*> queue;
        queue.push(&printRoot);
        vector<PrintTreeNode*> line;
        depth = 0;

        print();
        while (!queue.empty())
        {
            PrintTreeNode* next = queue.front();
            queue.pop();

            for (size_t i = 0; i < next->children.size(); i++)
            {
                queue.push(next->children[i]);
            }

            std::cout << next->text;
            line.push_back(next);

            if (!queue.empty() && depth != queue.front()->depth)
            {
                std::cout << std::endl;
                depth = queue.front()->depth;
                print();
                for (auto node : line)
                {
                    for (size_t i = 0; i < node->children.size(); i++)
                    {
                        std::string& text = node->children[i]->text;
                        auto leadingSpaceCount = text.find_first_not_of(' ');
                        std::cout << std::string(leadingSpaceCount, ' ');
                        std::cout << "|";
                        std::cout << std::string(text.size() - leadingSpaceCount - 1, ' ');
                    }
                }
                std::cout << std::endl;
                print();
                line.clear();
            }
        }
    }

    void PrintSymbolTable(SymbolTable* symTable, int indentLevel)
    {
        assert(symTable != NULL);
        SymbolTableWithOrder* symTableOrdered = NULL;
        if (symTable->GetScopeType() == EScopeType::PARAMETERS
            || symTable->GetScopeType() == EScopeType::STRUCTURE)
        {
            symTableOrdered = static_cast<SymbolTableWithOrder*>(symTable);
        }
        // utility -------------------------------------------------------------
        auto print = [&]() -> decltype(std::operator <<(std::cout, std::string()))
        {
            return std::cout << std::string(indentLevel * 2, ' ');
        };

        auto printn = [&](int n) -> decltype(std::operator <<(std::cout, std::string()))
        {
            return std::cout << std::string(n * 2, ' ');
        };

        auto splitter = [&]()
        {
            print() << std::string(48 - indentLevel * 2, '-') << std::endl;
        };

        auto prolog = [&](const std::string text, int n)
        {
            printn(n) << ">" << std::string(48 - n * 2 - 1 - text.size(), '-') << text << std::endl;
        };

        auto epilog = [&](const std::string text, int n)
        {
            printn(n) << "<" << std::string(48 - n * 2 - 1 - text.size(), '-') << text << std::endl;
        };

        // types ---------------------------------------------------------------
        if (symTable->types.size() > 0)
        {
            print() << "types:" << std::endl;
            splitter();
        }
        for (auto type : symTable->types)
        {
            std::string typeName = type.first;
            SymbolType* typeSym = type.second.get();
            print() << typeSym->GetQualifiedName() << std::endl;

            switch (typeSym->GetSymbolType())
            {
                case ESymbolType::TYPE_STRUCT:
                {
                    SymbolStruct* symStruct = static_cast<SymbolStruct*>(typeSym);
                    if (symStruct->complete)
                    {
                        PrintSymbolTable(symStruct->GetSymbolTable().get(), indentLevel + 1);
                    }
                    break;
                }

                default:
                {
                    break;
                }
            }
            print() << std::endl;
        }

        // variables -----------------------------------------------------------
        if (symTable->variables.size() > 0)
        {
            print() << "variables:" << std::endl;
            splitter();
        }
        if (symTableOrdered != NULL)
        {
            for (auto var : symTableOrdered->orderedVariables)
            {
                print() << var->GetQualifiedName() << std::endl;
            }
        }
        else
        {
            for (auto var : symTable->variables)
            {
                std::string varName = var.first;
                SymbolVariable* varSym = var.second.get();
                print() << varSym->GetQualifiedName() << std::endl;
            }
        }

        // functions -----------------------------------------------------------
        if (symTable->functions.size() > 0)
        {
            print() << "functions:" << std::endl;
            splitter();
        }

        std::function<void(Statement*, int)> f = [&](Statement* statement, int depth)
        {
            if (statement->GetStatementType() == EStatementType::COMPOUND)
            {
                CompoundStatement* compoundStatement = static_cast<CompoundStatement*>(statement);
                PrintSymbolTable(compoundStatement->GetSymbolTable().get(), indentLevel + 1);
            }
            else if (statement->GetStatementType() == EStatementType::ITERATION_FOR)
            {
                ForStatement* forStatement = static_cast<ForStatement*>(statement);
                PrintSymbolTable(forStatement->GetSymbolTable().get(), indentLevel + 1);
            }

            for (int i = 0; i < statement->GetChildCount(); i++)
            {
                ASTNode* node = statement->GetChild(i).get();
                if (node->IsStatement())
                {
                    Statement* statement = static_cast<Statement*>(node);
                    // printn(depth) << statement->token.text << std::endl;
                    // splittern(depth);
                    prolog(statement->token.text, depth);
                    f(statement, depth + 1);
                    epilog(statement->token.text, depth);
                    // splittern(depth);
                    // printn(depth) << "end of " << statement->token.text << std::endl;
                }
                else
                {
                    PrintAST(node, depth);
                    std::cout << std::endl << std::endl;
                }
            }

        };

        for (auto function : symTable->functions)
        {
            std::string functionName = function.first;
            SymbolVariable* functionSym = function.second.get();
            print() << functionSym->GetQualifiedName() << std::endl;
            SymbolFunctionType* symFunType = static_cast<SymbolFunctionType*>(functionSym->GetTypeSymbol().get());
            if (symFunType->GetSymbolTable() != NULL)
            {
                PrintSymbolTable(symFunType->GetSymbolTable().get(), indentLevel + 1);
            }
            CompoundStatement* body = symFunType->GetBody().get();
            if (body != NULL)
            {
                f(body, indentLevel);
            }
            else
            {
                // function declaration present, but definition missing
            }
        }
    }

} // namespace Compiler
