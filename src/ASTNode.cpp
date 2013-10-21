#include "ASTNode.hpp"

namespace Compiler
{
//------------------------------------------------------------------------------
        ASTNode::~ASTNode()
        {
            while(!children_.empty())
            {
                delete children_.back();
                children_.pop_back();
            }
        }

//------------------------------------------------------------------------------
        ASTNode::ASTNode(const Token &token)
            : token(token)
        {

        }

} // namespace
