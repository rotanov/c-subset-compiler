#pragma once

namespace Compiler
{
class ASTNode;
class SymbolTable;

void PrintAST(ASTNode *root, int indentLevel = 0);
void PrintSymbolTable(SymbolTable* symTable, int indentLevel = 0);

} // namespace Compiler
