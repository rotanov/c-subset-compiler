#include "ASTNode.hpp"

namespace Compiler
{
//------------------------------------------------------------------------------
        ASTNode::~ASTNode()
        {

        }

//------------------------------------------------------------------------------
        ASTNode::ASTNode(const Token &token)
            : token(token)
        {

        }

} // namespace
