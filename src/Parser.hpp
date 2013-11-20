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
        SymbolType* typeSymbol{NULL};

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

    private:
        std::vector<Token> tokenStack_;
        std::vector<ASTNode*> nodeStack_;
        Coroutine parseCoroutine_;
        std::vector<SymbolTable*> symTables_;

        // expressions
        ASTNode* ParseTopLevelExpression_(CallerType& caller);
        ASTNode* ParsePrimaryExpression_(CallerType& caller);
        ASTNode* ParseBinaryOperator_(CallerType& caller, int priority);
        ASTNode* ParseExpression_(CallerType& caller);
        ASTNode* ParseAssignmentExpression_(CallerType& caller);
        ASTNode* ParseUnaryExpression_(CallerType& caller);
        ASTNode* ParseConditionalExpression_(CallerType& caller);
        ASTNode* ParsePostfixExpression_(CallerType& caller);

//        // type-name
        ASTNode* ParseTypeName_(CallerType& caller);
        ASTNode* ParseSpecifierQualifierList_(CallerType& caller);
        ASTNode* ParseAbstractDeclarator_(CallerType& caller);
        std::tuple<SymbolType*, SymbolType*> ParsePointer_(CallerType& caller, SymbolType* refType = NULL);
        ASTNode* ParseDirectAbstractDeclarator_(CallerType& caller);
        ASTNode* ParseParameterList_(CallerType& caller);

        Symbol* ParseDeclaration_(CallerType& caller);
        DeclarationSpecifiers ParseDeclarationSpecifiers_(CallerType& caller);
        Symbol* ParseInitDeclaratorList_(CallerType& caller, DeclarationSpecifiers& declSpec);
        SymbolVariable* ParseOutermostDeclarator_(CallerType& caller, DeclarationSpecifiers& declSpec);
        Symbol* ParseInnerDeclarator_(CallerType& caller, SymbolVariable*& declaratorVariable);
        void ParseParameterList(CallerType& caller, SymbolFunctionType& symFuncType);

        // declaration
        SymbolStruct* ParseStructSpecifier_(CallerType& caller);

        void ParseTranslationUnit_(CallerType& caller);

        Statement* ParseStatement_(CallerType& caller);
        CompoundStatement* ParseCompoundStatement_(CallerType& caller);
        SelectionStatement* ParseSelectionStatement_(CallerType& caller);
        IterationStatement* ParseIterationStatement_(CallerType& caller);
        JumpStatement* ParseJumpStatement_(CallerType& caller);
        ExpressionStatement* ParseExpressionStatement_(CallerType& caller);

        void ThrowInvalidTokenError_(const Token& token, const std::string& descriptionText = "");
        void ThrowError_(const std::string& descriptionText);
        void ResumeParse_(const Token& token);
        Token WaitForTokenReady_(CallerType& caller);

        // waits for token, returns it and pushes it back if token is not of tokenType
        Token WithdrawTokenIf_(CallerType& caller, const ETokenType& tokenType = TT_INVALID);

        void FlushOutput_();

        bool IsStartsDeclaration_(const Token& token) const;

        SymbolType* LookupType_(const std::string& name) const;
        SymbolVariable* LookupVariable_(const std::string& name) const;
        SymbolVariable* LookupFunction_(const std::string& name) const;

        void AddType_(SymbolType* symType);
        void AddType_(SymbolType* symType, const std::string& name);
        void AddVariable_(SymbolVariable* symVar);
        void AddFunction_(SymbolVariable* symFun);

        template <typename R, typename C, class... ArgTypes>
        R LookupSymbolHelper_(const std::string& name, R (C::*lookuper)(ArgTypes...) const) const
        {
            R symbol = NULL;
            for (int i = symTables_.size() - 1; i >= 0; i--)
            {
                symbol = (symTables_[i]->*lookuper)(name);
                if (symbol != NULL)
                {
                    break;
                }
            }
            return symbol;
        }

    public:
        Parser();

        virtual void EmitInvalid(const string& source,
                                 const int line,
                                 const int column);

        virtual void EmitKeyword(const string& source,
                                 ETokenType token_type,
                                 const int line,
                                 const int column);

        virtual void EmitPunctuation(const string& source,
                                     ETokenType token_type,
                                     const int line,
                                     const int column);

        virtual void EmitIdentifier(const string& source,
                                    const int line,
                                    const int column);

        virtual void EmitLiteral(const string& source,
                                 EFundamentalType type,
                                 const void* data,
                                 size_t nbytes,
                                 const int line,
                                 const int column);

        virtual void EmitLiteralArray(const string& source,
                                      size_t num_elements,
                                      EFundamentalType type,
                                      const void* data,
                                      size_t nbytes,
                                      const int line,
                                      const int column);

        virtual void EmitEof(const int line, const int column);

    };

} // namespace Compiler
