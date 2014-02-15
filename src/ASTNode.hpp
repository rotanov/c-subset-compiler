#pragma once

#include "Token.hpp"

#include <vector>
#include <cassert>

#include "utils.hpp"
#include "Visitor.hpp"

namespace Compiler
{
    class SymbolType;
////////////////////////////////////////////////////////////////////////////////
    class ASTNode
    {
    public:
        Token token;

        ASTNode(const Token& token);
        ASTNode(const Token& token, shared_ptr<SymbolType> type);
        virtual ~ASTNode();

        int GetChildCount() const;
        shared_ptr<ASTNode> GetChild(const int index);
        void SetTypeSym(shared_ptr<SymbolType> type);
        shared_ptr<SymbolType> GetTypeSym() const;
        virtual bool IsConstExpr() const;
        virtual int EvalToInt() const;

    protected:
        std::vector<shared_ptr<ASTNode>> children_;
        shared_ptr<SymbolType> typeSym_{NULL};
    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeBinaryOperator
            : public ASTNode
            , public IVisitableBase
    {
        COMPILER_DECLARE_VISITABLE()

    public:
        ASTNodeBinaryOperator(const Token& token, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right);
        virtual bool IsConstExpr() const;
        int EvalToInt() const;
    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeAssignment
            : public ASTNode
            , public IVisitableBase
    {
        COMPILER_DECLARE_VISITABLE()

    public:
        ASTNodeAssignment(const Token& token, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right);
        virtual bool IsConstExpr() const;
    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeUnaryOperator
            : public ASTNode
            , public IVisitableBase
    {
        COMPILER_DECLARE_VISITABLE()

    public:
        ASTNodeUnaryOperator(const Token& token);
        ASTNodeUnaryOperator(const Token& token, shared_ptr<ASTNode> node);

        void SetOperand(shared_ptr<ASTNode> node);
        shared_ptr<ASTNode> GetOperand();
        virtual bool IsConstExpr() const;
        virtual int EvalToInt() const;

    private:
        void CheckTypes_();

    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeConditionalOperator
            : public ASTNode
            , public IVisitableBase
    {
        COMPILER_DECLARE_VISITABLE()

    public:
        ASTNodeConditionalOperator(const Token& token
                                   , shared_ptr<ASTNode> condition
                                   , shared_ptr<ASTNode> thenExpression
                                   , shared_ptr<ASTNode> elseExpression);

        virtual bool IsConstExpr() const;
        virtual int EvalToInt() const;
    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeArraySubscript
            : public ASTNode
            , public IVisitableBase
    {
        COMPILER_DECLARE_VISITABLE()

    public:
        ASTNodeArraySubscript(const Token& token, shared_ptr<ASTNode> left, shared_ptr<ASTNode> right);
        virtual bool IsConstExpr() const;
    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeFunctionCall
            : public ASTNode
            , public IVisitableBase
    {
        COMPILER_DECLARE_VISITABLE()

    public:
        ASTNodeFunctionCall(const Token& token, shared_ptr<ASTNode> caller, vector<shared_ptr<ASTNode>>& parameters);
        virtual bool IsConstExpr() const;
    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeStructureAccess
            : public ASTNode
            , public IVisitableBase
    {
        COMPILER_DECLARE_VISITABLE()

    public:
        ASTNodeStructureAccess(const Token& token, shared_ptr<ASTNode> lhs, shared_ptr<ASTNode> rhs);
        virtual bool IsConstExpr() const;
    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeTypeName
            : public ASTNode
            , public IVisitableBase
    {
        COMPILER_DECLARE_VISITABLE()

    public:
        ASTNodeTypeName(shared_ptr<SymbolType> typeNameSymbol);
        virtual bool IsConstExpr() const;

    private:
        shared_ptr<SymbolType> typeSymbol_{NULL};
    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeCast
            : public ASTNode
            , public IVisitableBase
    {
        COMPILER_DECLARE_VISITABLE()

    public:
        ASTNodeCast(shared_ptr<ASTNodeTypeName> left, shared_ptr<ASTNode> right);
        virtual bool IsConstExpr() const;
    };

////////////////////////////////////////////////////////////////////////////////
    class ASTNodeCommaOperator
            : public ASTNode
            , public IVisitableBase
    {
        COMPILER_DECLARE_VISITABLE()

    public:
        ASTNodeCommaOperator(const Token& token);
        virtual bool IsConstExpr() const;
        virtual int EvalToInt() const;
        void PushOperand(const shared_ptr<ASTNode> node);
    };

    // NOTE: in C assignment as well as inc/dec does not yields an lvalue
    // see 6.5.16/3
    bool IfLValue(shared_ptr<ASTNode> node);
    bool IfModifiableLValue(shared_ptr<ASTNode> node);

} // namespace Compiler
