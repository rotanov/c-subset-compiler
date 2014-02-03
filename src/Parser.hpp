#pragma once

#include <string>
#include <vector>
#include <queue>
#include <functional>
#include <type_traits>
#include <cassert>

#include <boost/coroutine/all.hpp>
#include <boost/bind.hpp>

#include "ITokenStream.hpp"
#include "Token.hpp"
#include "ASTNode.hpp"
#include "SymbolTable.hpp"
#include "Statement.hpp"

namespace Compiler
{
//    template <typename T, T> struct mf_proxy;

//    template <typename T, typename R, typename ...Args, R (T::*mf)(Args...)>
//    struct mf_proxy<R (T::*)(Args...), mf>
//    {
//        typedef R ReturnType;
//        static R call(T & obj, Args &&... args)
//        {
//            return (obj.*mf)(std::forward<Args>(args)...);
//        }
//    };

    class DeclarationSpecifiers
    {
    public:
        bool isTypedef{false};
        shared_ptr<SymbolType> typeSymbol{NULL};

        // ???
        void Invalidate()
        {
            isTypedef = false;
            typeSymbol = NULL;
        }
    };

    class Parser : public ITokenStream
    {
        typedef boost::coroutines::coroutine<void(const Token&)> Coroutine;
        typedef Coroutine::caller_type CallerType;

    protected:
        shared_ptr<SymbolTable> GetInternalSymbolTable() const;
        shared_ptr<SymbolTable> GetGlobalSymbolTable() const;

    private:
        std::vector<Token> tokenStack_;
        std::vector<shared_ptr<ASTNode>> nodeStack_;
        std::vector<shared_ptr<IterationStatement>> iterationStatementStack_;
        Coroutine parseCoroutine_;
        std::vector<shared_ptr<SymbolTable>> symTables_;
        int anonymousGenerator_{0};

        // expressions
        shared_ptr<ASTNode> ParsePrimaryExpression_(CallerType& caller);
        shared_ptr<ASTNode> ParseBinaryOperator_(CallerType& caller, int priority);
        shared_ptr<ASTNode> ParseExpression_(CallerType& caller);
        shared_ptr<ASTNode> ParseAssignmentExpression_(CallerType& caller);
        shared_ptr<ASTNode> ParseCastExpression_(CallerType& caller);
        shared_ptr<ASTNode> ParseUnaryExpression_(CallerType& caller);
        shared_ptr<ASTNode> ParseConditionalExpression_(CallerType& caller);
        shared_ptr<ASTNode> ParsePostfixExpression_(CallerType& caller);

        // type-name
        ASTNode* ParseTypeName_(CallerType& caller);
        ASTNode* ParseSpecifierQualifierList_(CallerType& caller);

        typedef std::tuple<shared_ptr<SymbolType>, shared_ptr<SymbolType>> PointerChainHeadTail;
        PointerChainHeadTail ParsePointer_(CallerType& caller);

        ASTNode* ParseParameterList_(CallerType& caller);

        shared_ptr<Symbol> ParseDeclaration_(CallerType& caller);
        DeclarationSpecifiers ParseDeclarationSpecifiers_(CallerType& caller);
        shared_ptr<Symbol> ParseInitDeclaratorList_(CallerType& caller, DeclarationSpecifiers& declSpec);
        shared_ptr<SymbolVariable> ParseDeclarator_(CallerType& caller, DeclarationSpecifiers& declSpec, bool abstract = false);
        void ParseParameterList(CallerType& caller, SymbolFunctionType& symFuncType);

        shared_ptr<ASTNode> ParseInitializer_(CallerType& caller);

        // declaration
        shared_ptr<SymbolStruct> ParseStructSpecifier_(CallerType& caller);

        void ParseTranslationUnit_(CallerType& caller);

        shared_ptr<Statement> ParseStatement_(CallerType& caller);
        shared_ptr<CompoundStatement> ParseCompoundStatement_(CallerType& caller);
        shared_ptr<SelectionStatement> ParseSelectionStatement_(CallerType& caller);
        shared_ptr<IterationStatement> ParseIterationStatement_(CallerType& caller);
        shared_ptr<ForStatement> ParseForStatement_(CallerType& caller);
        shared_ptr<DoStatement> ParseDoStatement_(CallerType& caller);
        shared_ptr<WhileStatement> ParseWhileStatement_(CallerType& caller);
        shared_ptr<JumpStatement> ParseJumpStatement_(CallerType& caller);
        shared_ptr<ExpressionStatement> ParseExpressionStatement_(CallerType& caller);

        void ThrowError_(const std::string& descriptionText);
        void ResumeParse_(const Token& token);
        Token WaitForTokenReady_(CallerType& caller);

        // waits for token, returns it and pushes it back if token is not of tokenType
        Token WithdrawTokenIf_(CallerType& caller, const ETokenType& tokenType = TT_INVALID);
        Token WithdrawTokenIf_(CallerType& caller, bool condition);

        bool IsDeclarationSpecifier_(const Token& token) const;
        bool IsSpecifierQualifier_(const Token& token) const;

        std::string GenerateStuctName_();
        std::string GenerateParameterName_();

        void AddType_(shared_ptr<SymbolType> symType);
        void AddType_(shared_ptr<SymbolType> symType, const std::string& name);
        void AddVariable_(shared_ptr<SymbolVariable> symVar);
        void AddFunction_(shared_ptr<SymbolVariable> symFun);

        template <typename R, typename C, class... ArgTypes>
        R LookupSymbolHelper_(const std::string& name, R (C::*lookuper)(ArgTypes...) const) const
        {
            R symbol = NULL;
            for (int i = symTables_.size() - 1; i >= 0; i--)
            {
                symbol = (symTables_[i].get()->*lookuper)(name);
                if (symbol != NULL)
                {
                    break;
                }
            }
            return symbol;
        }

    public:
        Parser();
        virtual ~Parser();

        virtual void Flush() const;

        virtual void EmitInvalid(
            const string& source,
            const int line,
            const int column);

        virtual void EmitKeyword(
            const string& source,
            ETokenType token_type,
            const int line,
            const int column);

        virtual void EmitPunctuation(
            const string& source,
            ETokenType token_type,
            const int line,
            const int column);

        virtual void EmitIdentifier(
            const string& source,
            const int line,
            const int column);

        virtual void EmitLiteral(
            const string& source,
            EFundamentalType type,
            const void* data,
            size_t nbytes,
            const int line,
            const int column);

        virtual void EmitLiteralArray(
            const string& source,
            size_t num_elements,
            EFundamentalType type,
            const void* data,
            size_t nbytes,
            const int line,
            const int column);

        virtual void EmitEof(const int line, const int column);

//------------------------------------------------------------------------------
        shared_ptr<SymbolType> LookupType(const std::string& name) const;
        shared_ptr<SymbolVariable> LookupVariable(const std::string& name) const;
        shared_ptr<SymbolVariable> LookupFunction(const std::string& name) const;

    };

} // namespace Compiler
