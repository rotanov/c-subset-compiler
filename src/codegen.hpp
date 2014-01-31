#pragma once

#include "Visitor.hpp"
#include "ASTNode.hpp"
#include "Statement.hpp"

namespace Compiler
{
    static const char* asmHeader = R"(
    .686P
    .XMM
    include listing.inc
    .model	flat

INCLUDELIB LIBCMT
INCLUDELIB OLDNAMES
)";


    class AsmCommand
    {
    public:
        AsmCommand()
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

    CodeGenVisitor codegenInstance;

} // namespace Compiler
