#include "prettyPrinting.hpp"

#include <functional>
#include <vector>
#include <string>
#include <queue>
#include <algorithm>
#include <iostream>

#include "ASTNode.hpp"

namespace Compiler
{
    void PrintAST(ASTNode* root)
    {
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
                    f(next->GetChild(i), printNode, (maxLeft + spacing) * (i != 0) + estimate * (i == 0));
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

        while (!queue.empty())
        {
            PrintTreeNode* next = queue.front();
            queue.pop();

            for (int i = 0; i < next->children.size(); i++)
            {
                queue.push(next->children[i]);
            }

            std::cout << next->text;
            line.push_back(next);

            if (!queue.empty() && depth != queue.front()->depth)
            {
                std::cout << std::endl;
                depth = queue.front()->depth;
                for (auto node : line)
                {
                    for (int i = 0; i < node->children.size(); i++)
                    {
                        std::string& text = node->children[i]->text;
                        auto leadingSpaceCount = text.find_first_not_of(' ');
                        std::cout << std::string(leadingSpaceCount, ' ');
                        std::cout << "|";
                        std::cout << std::string(text.size() - leadingSpaceCount - 1, ' ');
                    }
                }
                std::cout << std::endl;
                line.clear();
            }
        }
    }

} // namespace Compiler
