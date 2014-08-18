#pragma once

#include "Visitor.hpp"
#include "ASTNode.hpp"
#include "Statement.hpp"
#include "Parser.hpp"

namespace Compiler
{
static const char* asmHeader = R"(
    .686P
    .XMM
    include listing.inc
    .model flat

    INCLUDELIB LIBCMT
    INCLUDELIB OLDNAMES

    )";

static const char* asmFooter = R"(
    END

    )";

enum class EAsmMnemonic
{
  MOV,
  PUSH,
  POP,
  RET,
  CALL,
  NOP,
};

enum class EAsmRegister
{
  EAX,
  EBX,
  ECX,
  EDX,
  ESI,
  EDI,
  ESP,
  EBP,
};

class AsmArgument
{
public:
private:

};

class AsmInstruction
{
public:
  AsmInstruction()
  {

  }

private:

};

class CodeGenVisitor
    : virtual public IVisitorBase
    , virtual public IVisitor<ASTNodeAssignment>
{
public:
  void VisitOnEnter(ASTNodeAssignment &)
  {

  }

  void VisitOnLeave(ASTNodeAssignment &)
  {

  }
};

class CodeGenerator : public Parser
{
public:
  CodeGenerator();
  ~CodeGenerator();

  virtual void Flush() const;
};

} // namespace Compiler
