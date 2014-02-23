#include "Parser.hpp"

#include <sstream>
#include <unordered_set>
#include <iostream>
#include <stack>
#include <cassert>
#include <tuple>

#include <boost/bind.hpp>

#include "utils.hpp"
#include "prettyPrinting.hpp"

namespace Compiler
{
//------------------------------------------------------------------------------
    Parser* theParser = NULL;

    Parser::Parser()
    {
        theParser = this;
        // environment
        shared_ptr<SymbolTable> internalSymbols = make_shared<SymbolTable>(EScopeType::INTERNAL);
        symTables_.push_back(internalSymbols);

        internalSymbols->AddType(make_shared<SymbolChar>());
        internalSymbols->AddType(make_shared<SymbolInt>());
        internalSymbols->AddType(make_shared<SymbolFloat>());
        internalSymbols->AddType(make_shared<SymbolVoid>());
        // TODO: set print return (ref) type to `void`
        internalSymbols->AddVariable(
            make_shared<SymbolVariable>("print",
                make_shared<SymbolFunctionType>(
                    LookupType("void"),
                    make_shared<SymbolTableWithOrder>(EScopeType::PARAMETERS))));


        shared_ptr<SymbolTable> globalSymbols = make_shared<SymbolTable>(EScopeType::GLOBAL);
        symTables_.push_back(globalSymbols);

        parseCoroutine_ = boost::move(Coroutine(boost::bind(&Parser::ParseTranslationUnit_, this, _1), Token()));
    }

//------------------------------------------------------------------------------
    Parser::~Parser()
    {

    }

//------------------------------------------------------------------------------
    shared_ptr<SymbolTable> Parser::GetInternalSymbolTable() const
    {
//        assert(symTables_.size() == 2);
        return symTables_[0];
    }

//------------------------------------------------------------------------------
    shared_ptr<SymbolTable> Parser::GetGlobalSymbolTable() const
    {
//        assert(symTables_.size() == 2);
        return symTables_[1];
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> Parser::ParsePrimaryExpression_(CallerType& caller)
    {
        Token token = TakeToken_(caller);
        switch(token.type)
        {
            case TT_IDENTIFIER:
            case TT_LITERAL_INT:
            case TT_LITERAL_FLOAT:
            case TT_LITERAL_CHAR:
            case TT_LITERAL_CHAR_ARRAY:
            {
                string text = token.text;
                shared_ptr<SymbolType> type;

                switch (token.type)
                {
                    case TT_IDENTIFIER:
                        if (!(LookupFunction(text)
                              || LookupVariable(text)))
                        {
                            ThrowInvalidTokenError(token, "undeclared identifier");
                        }

                        if (LookupVariable(text) != NULL)
                        {
                            type = LookupVariable(text)->GetRefSymbol();
                        }

                        if (type == NULL)
                        {
                            type = LookupFunction(text)->GetRefSymbol();
                        }

                        break;

                    case TT_LITERAL_INT:
                        type = LookupType("int");
                        break;

                    case TT_LITERAL_FLOAT:
                        type = LookupType("float");
                        break;

                    case TT_LITERAL_CHAR:
                        type = LookupType("char");
                        break;

                    case TT_LITERAL_CHAR_ARRAY:
                        type = make_shared<SymbolArray>();
                        break;

                    default:
                        assert(false);
                        break;
                }

                shared_ptr<ASTNode> node = make_shared<ASTNode>(token);
                node->SetTypeSym(type);
                if (token == TT_LITERAL_CHAR_ARRAY)
                {
                    stringTable_.push_back(node);
                }
                return node;
                break;
            }

            case OP_LPAREN:
            {
                shared_ptr<ASTNode> r = ParseExpression_(caller);

                token = TakeToken_(caller);
                if (token != OP_RPAREN)
                {
                    ThrowInvalidTokenError(token, "')' expected");
                }
                return r;
                break;
            }

            default:
            {
                ThrowInvalidTokenError(token, "unexpected in primary-expression");
                tokenStack_.push_back(token);
                return NULL;
                break;
            }
        }
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> Parser::ParseBinaryOperator_(CallerType& caller, int priority)
    {
        shared_ptr<ASTNode> left = NULL;

        if (nodeStack_.empty())
        {
            left = ParseCastExpression_(caller);
        }
        else
        {
            left = nodeStack_.back();
            nodeStack_.pop_back();
        }

        Token token = TakeToken_(caller);

        auto it = binaryOperatorTypeToPrecedence.find(token.type);
        while (it != binaryOperatorTypeToPrecedence.end()
               && it->second >= priority)
        {
            left = make_shared<ASTNodeBinaryOperator>(token, left, ParseBinaryOperator_(caller, it->second + 1));
            token = TakeToken_(caller);
            it = binaryOperatorTypeToPrecedence.find(token.type);
        }

        tokenStack_.push_back(token);
        return left;
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> Parser::ParseExpression_(CallerType& caller)
    {
        shared_ptr<ASTNode> node = ParseAssignmentExpression_(caller);
        Token token = TakeToken_(caller);
        if (token == OP_COMMA)
        {
            shared_ptr<ASTNodeCommaOperator> nodeCommaOp = make_shared<ASTNodeCommaOperator>(token);
            nodeCommaOp->PushOperand(node);
            while (token == OP_COMMA)
            {
                nodeCommaOp->PushOperand(ParseAssignmentExpression_(caller));
                token = TakeToken_(caller);
            }
            node = nodeCommaOp;
        }
        tokenStack_.push_back(token);
        return node;
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> Parser::ParseAssignmentExpression_(CallerType& caller)
    {
        Token tokenParen = TakeToken_(caller);
        if (tokenParen == OP_LPAREN)
        {
            Token tokenSpecQualifier = TakeToken_(caller);

            tokenStack_.push_back(tokenSpecQualifier);
            tokenStack_.push_back(tokenParen);

            if (IsSpecifierQualifier_(tokenSpecQualifier))
            {
                // it is cast expression therefore conditional-expression here
                return ParseConditionalExpression_(caller);
            }
        }
        else
        {
            tokenStack_.push_back(tokenParen);
        }

        {
            shared_ptr<ASTNode> node = ParseUnaryExpression_(caller);
            Token token = TakeToken_(caller);
            if (IsAssignmentOperator(token.type))
            {
                return make_shared<ASTNodeAssignment>(token, node, ParseAssignmentExpression_(caller));
            }
            else
            {
                // return node, so ParseBinOp will get it
                // return token(binop) as well
                tokenStack_.push_back(token);
                nodeStack_.push_back(node);
                return ParseConditionalExpression_(caller);
            }
        }
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> Parser::ParseCastExpression_(Parser::CallerType& caller)
    {
        Token token = TakeTokenIf_(caller, OP_LPAREN);
        if (token == OP_LPAREN)
        {
             token = TakeToken_(caller);
            if (IsSpecifierQualifier_(token))
            {
                tokenStack_.push_back(token);
                DeclarationSpecifiers declSpec = ParseDeclarationSpecifiers_(caller);

                token = TakeToken_(caller);
                if (token != OP_RPAREN)
                {
                    tokenStack_.push_back(token);

                    shared_ptr<SymbolVariable> variable = ParseDeclarator_(caller, declSpec, true);
                    shared_ptr<ASTNode> result = make_shared<ASTNodeCast>(
                                                   make_shared<ASTNodeTypeName>(
                                                     variable->GetRefSymbol()),
                                                   ParseCastExpression_(caller));

                    token = TakeToken_(caller);
                    if (token != OP_RPAREN)
                    {
                        ThrowInvalidTokenError(token, "right parentesis expected");
                    }

                    return result;
                }
                else
                {
                    return make_shared<ASTNodeCast>(make_shared<ASTNodeTypeName>(declSpec.typeSymbol), ParseCastExpression_(caller));
                }
            }
            else
            {
                tokenStack_.push_back(Token(OP_LPAREN, "("));
                tokenStack_.push_back(token);
                return ParseUnaryExpression_(caller);
            }
        }
        else
        {
            return ParseUnaryExpression_(caller);
        }
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> Parser::ParseUnaryExpression_(CallerType& caller)
    {
        Token token;
        std::vector<shared_ptr<ASTNode>> nodes;
        bool parsing = true;

        while (parsing)
        {
            token = TakeToken_(caller);
            if (IsUnaryOperator(token)
                || token == OP_INC
                || token == OP_DEC
                || token == KW_SIZEOF)
            {
                nodes.push_back(make_shared<ASTNodeUnaryOperator>(token));
            }
            else
            {
                tokenStack_.push_back(token);

                shared_ptr<ASTNode> temp;

                if (nodes.size() == 0
                    || nodes.back()->token == OP_INC
                    || nodes.back()->token == OP_DEC)
                {
                    temp = ParsePostfixExpression_(caller);
                }
                else if (IsUnaryOperator(nodes.back()->token))
                {
                    temp = ParseCastExpression_(caller);
                }
                else if (nodes.back()->token == KW_SIZEOF)
                {
                    token = TakeToken_(caller);

                    if (token == OP_LPAREN)
                    {
                        token = TakeToken_(caller);
                        if (IsSpecifierQualifier_(token))
                        {
                            tokenStack_.push_back(token);
                            DeclarationSpecifiers declSpec = ParseDeclarationSpecifiers_(caller);
                            token = TakeToken_(caller);
                            if (token != OP_RPAREN)
                            {
                                tokenStack_.push_back(token);
                                shared_ptr<SymbolVariable> variable = ParseDeclarator_(caller, declSpec, true);
                                temp = make_shared<ASTNodeTypeName>(variable->GetRefSymbol());

                                token = TakeToken_(caller);
                                if (token != OP_RPAREN)
                                {
                                    ThrowInvalidTokenError(token, "right parentesis expected");
                                }
                            }
                            else
                            {
                                temp = make_shared<ASTNodeTypeName>(declSpec.typeSymbol);
                            }
                        }
                        else
                        {
                            tokenStack_.push_back(token);
                            tokenStack_.push_back(Token(OP_LPAREN));
                            temp = ParsePostfixExpression_(caller);
                        }
                    }
                    else
                    {
                        tokenStack_.push_back(token);
                        temp = ParsePostfixExpression_(caller);
                    }
                }
                else
                {
                    assert(false);
                }

                nodes.push_back(temp);
                parsing = false;
            }
        }

        for (int i = nodes.size() - 2; i >= 0; i--)
        {
            static_pointer_cast<ASTNodeUnaryOperator>(nodes[i])->SetOperand(nodes[i + 1]);
        }

        return nodes[0];
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> Parser::ParseConditionalExpression_(CallerType& caller)
    {
        shared_ptr<ASTNode> conditionNode = ParseBinaryOperator_(caller, 0);
        Token token = TakeToken_(caller);

        if (token == OP_QMARK)
        {
            shared_ptr<ASTNode> thenExpressionNode = ParseExpression_(caller);
            Token colonToken = TakeToken_(caller);

            if (colonToken == OP_COLON)
            {
                return make_shared<ASTNodeConditionalOperator>(token, conditionNode,
                    thenExpressionNode, ParseConditionalExpression_(caller));
            }
            else
            {
                ThrowInvalidTokenError(colonToken, "':' expected in conditional-expression");
            }
        }
        else
        {
            tokenStack_.push_back(token);
            return conditionNode;
        }
        return NULL;
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> Parser::ParsePostfixExpression_(CallerType& caller)
    {
        shared_ptr<ASTNode> node = ParsePrimaryExpression_(caller);
        Token token = TakeToken_(caller);

        while (true)
        {
            switch (token.type)
            {
                case OP_LSQUARE:
                {
                    // postfix-expression '[' expression ']'
                    node = make_shared<ASTNodeArraySubscript>(token, node, ParseExpression_(caller));
                    token = TakeToken_(caller);
                    if (token != OP_RSQUARE)
                    {
                        ThrowInvalidTokenError(token, "']' expected in postfix-expression");
                    }
                    token = TakeToken_(caller);
                    break;
                }

                case OP_LPAREN:
                {
                    // postfix-expression '(' {expression} ')' // argument-expression-list
                    Token prevToken = token;
                    token = TakeToken_(caller);

                    vector<shared_ptr<ASTNode>> parameters;

                    if (token != OP_RPAREN)
                    {
                        tokenStack_.push_back(token);

                        do
                        {
                            parameters.push_back(ParseAssignmentExpression_(caller));
                            token = TakeToken_(caller);

                        } while (token == OP_COMMA);

                        if (token != OP_RPAREN)
                        {
                            ThrowInvalidTokenError(token, "')' expected in postfix-expression");
                        }
                    }

                    node = make_shared<ASTNodeFunctionCall>(token, node, parameters);

                    token = TakeToken_(caller);
                    break;
                }

                case OP_DOT:
                case OP_ARROW:
                {
                    // postfix-expression '.' identifier
                    // postfix-expression '->' identifier
                    Token identifierToken = TakeToken_(caller);
                    if (identifierToken == TT_IDENTIFIER)
                    {
                        node = make_shared<ASTNodeStructureAccess>(token, node, make_shared<ASTNode>(identifierToken));
                    }
                    else
                    {
                        ThrowInvalidTokenError(identifierToken, "identifier expected");
                    }
                    token = TakeToken_(caller);
                    break;
                }

                case OP_INC:
                case OP_DEC:
                {
                    // postfix-expression '++'
                    // postfix-expression '--'
                    while (token == OP_INC
                           || token == OP_DEC)
                    {
                        node = make_shared<ASTNodeUnaryOperator>(token, node);
                        token = TakeToken_(caller);
                    }
                    break;
                }

                default:
                {
                    // primary-expression
                    tokenStack_.push_back(token);
                    return node;
                    break;
                }
            }
        }
    }

//------------------------------------------------------------------------------
    Parser::PointerChainHeadTail Parser::ParsePointer_(Parser::CallerType& caller)
    {
        shared_ptr<SymbolType> tail = make_shared<SymbolPointer>();
        shared_ptr<SymbolType> head = tail;

        Token token(OP_STAR);

        while (token == OP_STAR)
        {
            token = TakeToken_(caller);

            bool constant = false;
            while (token == KW_CONST)
            {
                if (!constant)
                {
                    constant = true;
                    tail = make_shared<SymbolConst>(tail);
                }
                token = TakeToken_(caller);
            }

            if (token == OP_STAR)
            {
                tail = make_shared<SymbolPointer>(tail);
            }
        }

        tokenStack_.push_back(token);

        return std::make_tuple(head, tail);
    }

//------------------------------------------------------------------------------
    shared_ptr<Symbol> Parser::ParseDeclaration_(Parser::CallerType& caller)
    {
        DeclarationSpecifiers declSpec = ParseDeclarationSpecifiers_(caller);
        Token token = TakeTokenIf_(caller, OP_SEMICOLON);

        if (token == OP_SEMICOLON)
        {
            // end of declSpec is legal, omitting optional init-declarator-list
            return NULL;
        }
        else
        {
            // here goes declarator
            // if we see `{` after declarator then it's function definition
            // and compound-statement should be parsed
            // else it is declaration
            shared_ptr<Symbol> ravalue = ParseInitDeclaratorList_(caller, declSpec);
            return ravalue;
        }

    }

//------------------------------------------------------------------------------
    DeclarationSpecifiers Parser::ParseDeclarationSpecifiers_(CallerType& caller)
    {
//        twice typedef is an error

//        If the same qualifier (const) appears more than once in the same specifier-qualifier-list, either
//        directly or via one or more typedefs, the behavior is the same as if it appeared only
//        once.

//        If the specification of an array type includes any type qualifiers, the element type is so-
//        qualified, not the array type. If the specification of a function type includes any type
//        qualifiers, the behavior is undefined. Both of these can occur through the use of typedefs.

//        For two qualified types to be compatible, both shall have the identically qualified version
//        of a compatible type; the order of type qualifiers within a list of specifiers or qualifiers
//        does not affect the specified type.

//        `void`, `char`, `int`, `float`
//        `struct`
//        typedef-name
//        all of the above could begin type-specifier part

//        `;`, `(`, `*`, identifier - end of decl spec


        DeclarationSpecifiers declSpec;
        Token token = TakeToken_(caller);
        bool constant = false;

        bool identifierFound = false;

        while (token != OP_SEMICOLON
               && token != OP_LPAREN
               && token != OP_STAR
               && token != OP_COMMA // unnamed parameter in fwd function declaration case
               && token != OP_RPAREN // same as above, but last in parameter list
               && !identifierFound)
        {
            switch (token)
            {
                case KW_CONST:
                    constant = true;
                    break;

                case KW_TYPEDEF:
                    if (declSpec.isTypedef)
                    {
                        ThrowInvalidTokenError(token, "more than one `typedef` not allowed");
                    }
                    declSpec.isTypedef = true;
                    break;

                case KW_STRUCT:
                    declSpec.typeSymbol = ParseStructSpecifier_(caller);
                    break;

                case KW_INT:
                case KW_FLOAT:
                case KW_VOID:
                case KW_CHAR:
                case TT_IDENTIFIER:
                    if (declSpec.typeSymbol != NULL)
                    {
                        if (LookupType(token.text) != NULL)
                        {
                            ThrowInvalidTokenError(token, "only one type per declaration specification is expected");
                        }
                        else
                        {
                            identifierFound = true;
                            tokenStack_.push_back(token);
                        }
                    }
                    else
                    {
                        declSpec.typeSymbol = LookupType(token.text);
                        if (declSpec.typeSymbol == NULL)
                        {
                            identifierFound = true;
                            tokenStack_.push_back(token);
                        }
                    }
                    break;

                default:
                    ThrowInvalidTokenError(token, "unexpected in declaration");
                    break;
            }
            token = TakeToken_(caller);
        }
        tokenStack_.push_back(token);

        if (declSpec.typeSymbol == NULL)
        {
            ThrowError_("type not specified in declaration");
        }

        if (declSpec.typeSymbol->GetType() == ESymbolType::TYPE_TYPEDEF)
        {
            shared_ptr<SymbolTypedef> symTypedef = static_pointer_cast<SymbolTypedef>(declSpec.typeSymbol);
            declSpec.typeSymbol = symTypedef->GetRefSymbol();
        }

        if (constant
            && declSpec.typeSymbol->GetType() != ESymbolType::TYPE_CONST)
        {
            declSpec.typeSymbol = make_shared<SymbolConst>(declSpec.typeSymbol);
        }

        return declSpec;
    }

//------------------------------------------------------------------------------
    shared_ptr<Symbol> Parser::ParseInitDeclaratorList_(Parser::CallerType& caller, DeclarationSpecifiers& declSpec)
    {
        shared_ptr<SymbolVariable> declarator = ParseDeclarator_(caller, declSpec);
        Token token = TakeToken_(caller);

        if (declarator->GetType() == ESymbolType::VARIABLE
            && declarator ->GetRefSymbol()->GetType() == ESymbolType::TYPE_FUNCTION
            && !declSpec.isTypedef)
        {
            if (token == OP_LBRACE
                // function definition allowed only on the global level
                && symTables_.size() == 2)
            {
                return declarator;
            }
            else if (symTables_.size() == 2)
            {
                AddFunction_(declarator);
            }
        }

        while (token != OP_SEMICOLON)
        {
            if (token == OP_ASS)
            {
                tokenStack_.push_back(token);
                ParseInitializer_(caller, declarator);
            }
            else if (token == OP_COMMA)
            {
                declarator = ParseDeclarator_(caller, declSpec);
                if (declarator->GetType() == ESymbolType::VARIABLE
                    && declarator->GetRefSymbol()->GetType() == ESymbolType::TYPE_FUNCTION
                    && symTables_.size() == 2
                    && !declSpec.isTypedef)
                {
                    AddFunction_(declarator);
                }
            }
            else if (token != OP_SEMICOLON)
            {
                ThrowInvalidTokenError(token, "`,` or `;` expected in init-declarator-list");
            }
            token = TakeToken_(caller);
        }

        return NULL;
    }

//------------------------------------------------------------------------------
    shared_ptr<SymbolVariable> Parser::ParseDeclarator_(Parser::CallerType& caller, DeclarationSpecifiers& declSpec, bool abstract)
    {
        shared_ptr<SymbolVariable> declaratorVariable = NULL;

        std::function<shared_ptr<Symbol>()> f = [&]() -> shared_ptr<Symbol>
        {
            shared_ptr<SymbolType> leftmostPointer = NULL;
            shared_ptr<SymbolType> rightmostPointer = NULL;
            shared_ptr<Symbol> centralType = NULL;
            Token token = TakeToken_(caller);

            // pointer part
            if (token == OP_STAR)
            {
                std::tie(leftmostPointer, rightmostPointer) = ParsePointer_(caller);
                token = TakeToken_(caller);
            }

            // direct declarator
            if (token == TT_IDENTIFIER)
            {
                if (abstract)
                {
                    ThrowInvalidTokenError(token, "unexpected identifier in abstract-declarator");
                }
                declaratorVariable = make_shared<SymbolVariable>(token.text);
                centralType = declaratorVariable;
            }
            else if (token == OP_LPAREN)
            {
                centralType = f();
                token = TakeToken_(caller);
                if (token != OP_RPAREN)
                {
                    ThrowInvalidTokenError(token, "`)` expected to close in declarator");
                }
            }
            else
            {
                // now we are sure there is no identifier
                // if we are in func parameters scope then we're allowed
                // to declare it anonymous, else it is treated as error
                shared_ptr<SymbolTable> topSymTable = symTables_.back();
                if (topSymTable->GetScopeType() == EScopeType::PARAMETERS
                    || abstract)
                {
                    tokenStack_.push_back(token);
                    declaratorVariable = make_shared<SymbolVariable>(GenerateParameterName_());
                    centralType = declaratorVariable;
                }
                else
                {
                    ThrowInvalidTokenError(token, "identifier or `(` expected");
                }
            }

            token = TakeToken_(caller);

            shared_ptr<Symbol> rightmostType = centralType;
            shared_ptr<SymbolType> temp = NULL;

            while (token == OP_LPAREN
                   || token == OP_LSQUARE)
            {
                if (token == OP_LPAREN)
                {
                    shared_ptr<SymbolTableWithOrder> parametersSymTable = make_shared<SymbolTableWithOrder>(EScopeType::PARAMETERS);
                    shared_ptr<SymbolFunctionType> type = make_shared<SymbolFunctionType>(parametersSymTable);
                    symTables_.push_back(parametersSymTable);
                    temp = type;

                    ParseParameterList(caller, *type);

                    symTables_.pop_back();

                    token = TakeToken_(caller);

                    if (token != OP_RPAREN)
                    {
                        ThrowInvalidTokenError(token, "closing `)` expected for parameter-list in declarator");
                    }
                }
                else if (token == OP_LSQUARE)
                {
                    shared_ptr<SymbolArray> type = make_shared<SymbolArray>();
                    temp = type;
                    token = TakeTokenIf_(caller, OP_RSQUARE);

                    if (token != OP_RSQUARE)
                    {
                        shared_ptr<ASTNode> sizeInitializer = ParseAssignmentExpression_(caller);
                        type->SetSizeInitializer(sizeInitializer);
                        token = TakeToken_(caller);
                        if (token != OP_RSQUARE)
                        {
                            ThrowInvalidTokenError(token, "closing `]` expected for array declarator");
                        }
                    }
                    else
                    {
                        ThrowInvalidTokenError(token, "arrays with unspecified length are unsupported");
                    }
                }

                assert(IfSymbolIsRef(rightmostType));
                static_pointer_cast<SymbolTypeRef>(rightmostType)->SetRefSymbol(temp);
                rightmostType = temp;

                token = TakeToken_(caller);
            }
            tokenStack_.push_back(token);

            if (rightmostPointer != NULL)
            {
                assert(IfSymbolIsRef(rightmostType));
                static_pointer_cast<SymbolTypeRef>(rightmostType)->SetRefSymbol(rightmostPointer);
                return leftmostPointer;
            }
            else
            {
                return rightmostType;
            }

        };

        static_pointer_cast<SymbolTypeRef>(f())->SetRefSymbol(declSpec.typeSymbol);

        if (declSpec.isTypedef)
        {
            shared_ptr<SymbolTypedef> symTypedef = make_shared<SymbolTypedef>(declaratorVariable->name);
            symTypedef->SetRefSymbol(declaratorVariable->GetRefSymbol());
            AddType_(symTypedef);
        }
        else
        {
            if (declaratorVariable->GetRefSymbol()->GetType() == ESymbolType::TYPE_FUNCTION)
            {
                // if we found function declaration at global scope
                // there may be function body
                // soo AddFunction_ shall be called at higher level
                if (symTables_.size() != 2)
                {
                    AddFunction_(declaratorVariable);
                }
            }
            else
            {
                if (declaratorVariable->GetRefSymbol()->GetType() == ESymbolType::TYPE_VOID)
                {
                    ThrowError_("variable " + declaratorVariable->name
                                + " declared as void");
                }
                AddVariable_(declaratorVariable);
            }
        }

        return declaratorVariable;
    }

//------------------------------------------------------------------------------
    void Parser::ParseParameterList(Parser::CallerType& caller, SymbolFunctionType& symFuncType)
    {
        // A declaration of a parameter as ‘‘array of type’’ shall be adjusted
        // to ‘‘qualified pointer to type’’

        // A declaration of a parameter as ‘‘function returning type’’ shall be
        // adjusted to ‘‘pointer to function returning type’’

        // If, in a parameter declaration, an identifier can be treated either
        // as a typedef name or as a parameter name, it shall be taken as a typedef name.

        // TODO: check size and types with symFuncType

        Token token = TakeToken_(caller);

        while (token != OP_RPAREN)
        {
            tokenStack_.push_back(token);

            DeclarationSpecifiers declSpec = ParseDeclarationSpecifiers_(caller);

            if (declSpec.isTypedef)
            {
                ThrowError_("typedef storage class specifier can not appear in parameter declaration");
            }

            ParseDeclarator_(caller, declSpec);

            token = TakeToken_(caller);

            if (token == OP_COMMA)
            {
                token = TakeToken_(caller);
            }
            else if (token != OP_RPAREN)
            {
                ThrowInvalidTokenError(token, "`,` or `)` expected at the end of parameter declarator");
            }
        }
        tokenStack_.push_back(token);
    }

//------------------------------------------------------------------------------
    void Parser::ParseInitializer_(Parser::CallerType& caller, shared_ptr<SymbolVariable> declarator)
    {
        // it's `=` now
        Token token = TakeToken_(caller);
        // for errors
        Token eqToken = token;

        std::stack<shared_ptr<SymbolType>> typeSymStack;
        typeSymStack.push(GetRefSymbol(declarator));
        auto top = typeSymStack.top();
        auto type = top->GetType();

        token = TakeTokenIf_(caller, OP_LBRACE);
        bool isBraced = (token == OP_LBRACE);
        int initializerCount = 0;

        if ((type == ESymbolType::TYPE_ARRAY
             || type == ESymbolType::TYPE_STRUCT)
            && !isBraced)
        {
            ThrowInvalidTokenError(token, "array or struct initializer list must be braced with `{` `}`");
        }

        do
        {
            shared_ptr<ASTNode> initializerExpression = ParseAssignmentExpression_(caller);
            declarator->PushInitializer(initializerExpression);
            initializerCount++;
            token = TakeToken_(caller);
        }
        while (isBraced
               && token == OP_COMMA);

        if (initializerCount != declarator->GetAbsoluteElementCount())
        {
            ThrowInvalidTokenError(eqToken, "initializer count doesn't correspond to object field count");
        }

        if (isBraced)
        {
            if (token != OP_RBRACE)
            {
                ThrowInvalidTokenError(token, "`}` exprected to close init-declarator-list");
            }
        }
        else
        {
            tokenStack_.push_back(token);
        }
    }

//------------------------------------------------------------------------------
    shared_ptr<SymbolStruct> Parser::ParseStructSpecifier_(Parser::CallerType& caller)
    {
        // `struct` already parsed
        Token token = TakeTokenIf_(caller, TT_IDENTIFIER);
        Token tagToken;

        if (token == TT_IDENTIFIER)
        {
            tagToken = token;
        }
        else if (token != OP_LBRACE)
        {
            ThrowInvalidTokenError(token, "anonymous struct shall be declared with struct-declaration-list");
        }

        // anonymous struct:
        if (tagToken != TT_IDENTIFIER)
        {
            tagToken = Token(TT_IDENTIFIER, GenerateStuctName_());
        }

        shared_ptr<SymbolType> symStructPresent = LookupType(tagToken.text);

        shared_ptr<SymbolStruct> symStruct = NULL;
        if (symStructPresent != NULL)
        {
            if (symStructPresent->GetType() == ESymbolType::TYPE_STRUCT)
            {
                // yup, it's actually a struct
                symStruct = static_pointer_cast<SymbolStruct>(symStructPresent);
            }
            else
            {
                // it's already here, but it's not struct
                // let's see if it's on the stack's top
                if (symTables_.back()->LookupType(tagToken.text) != NULL)
                {
                    ThrowError_("type " + tagToken.text + " already defined in this scope");
                }
            }
        }

        if (symStruct == NULL)
        {
            symStruct = make_shared<SymbolStruct>(tagToken.text);
            AddType_(symStruct);
        }

        token = TakeTokenIf_(caller, OP_LBRACE);

        if (token == OP_LBRACE)
        {
            if (symStruct->complete)
            {
                ThrowError_("redefinition of type " + tagToken.text + ", type is alreade complete");
            }

            shared_ptr<SymbolTableWithOrder> fieldsSymTable = make_shared<SymbolTableWithOrder>(EScopeType::STRUCTURE);
            symStruct->SetFieldsSymTable(fieldsSymTable);

            symTables_.push_back(fieldsSymTable);

            while (token != OP_RBRACE)
            {
                DeclarationSpecifiers declSpec = ParseDeclarationSpecifiers_(caller);
                if (declSpec.isTypedef)
                {
                    ThrowError_("struct declaration can not contain typedef");
                }
                token = TakeTokenIf_(caller, OP_SEMICOLON);

                if (token == OP_SEMICOLON)
                {
                    ThrowInvalidTokenError(token, "a member must be defined by type");
                }

                while (token != OP_SEMICOLON)
                {
                    ParseDeclarator_(caller, declSpec);
                    token = TakeToken_(caller);
                    if (token != OP_COMMA
                        && token != OP_SEMICOLON)
                    {
                        ThrowInvalidTokenError(token, "`,` or `;` expected");
                    }
                }

                token = TakeTokenIf_(caller, OP_RBRACE);
            }

            symTables_.pop_back();
            symStruct->complete = true;
        }

        return symStruct;
    }

//------------------------------------------------------------------------------
    void Parser::ParseTranslationUnit_(CallerType& caller)
    {
        shared_ptr<Symbol> symbol = NULL;
        Token token = TakeTokenIf_(caller, TT_EOF);

        while (token != TT_EOF)
        {
            symbol = ParseDeclaration_(caller);
            if (symbol != NULL) // ? nullptr ?
            {
                shared_ptr<SymbolVariable> symFun = static_pointer_cast<SymbolVariable>(symbol);
                // allowing recursive calls
                AddFunction_(symFun);
                symFun = LookupFunction(symFun->name);
                shared_ptr<SymbolFunctionType> symType = static_pointer_cast<SymbolFunctionType>(symFun->GetRefSymbol());
                symTables_.push_back(symType->GetSymbolTable());
                // at this point symbol has ESymbolType::VARIABLE of type ESymbolType::TYPE_FUNCTION
                // and `{` already eaten

                if (symType->GetBody() != NULL)
                {
                    // TODO: invalid error message because token here is a return type of function beging redeclared
                    // we need to pass somehow column and line of a function declaration through symbol variable
                    ThrowInvalidTokenError(token, "redefinition of " + symFun->GetQualifiedName());
                }
                else
                {
                    symType->SetBody(ParseCompoundStatement_(caller));
                }
                symTables_.pop_back();                
            }

            token = TakeTokenIf_(caller, TT_EOF);
        }
        // globals
        Flush();
    }

//------------------------------------------------------------------------------
    shared_ptr<CompoundStatement> Parser::ParseCompoundStatement_(CallerType& caller)
    {
        shared_ptr<SymbolTable> symTable = make_shared<SymbolTable>(EScopeType::BLOCK);
        symTables_.push_back(symTable);
        shared_ptr<CompoundStatement> compoundStatement = make_shared<CompoundStatement>(symTable);

        Token token = TakeTokenIf_(caller, OP_RBRACE);

        while (token != OP_RBRACE)
        {
            if (IsDeclarationSpecifier_(token))
            {
                // shall return NULL here
                ParseDeclaration_(caller);
            }
            else
            {
                compoundStatement->AddStatement(ParseStatement_(caller));
            }

            token = TakeTokenIf_(caller, OP_RBRACE);

            if (token == TT_EOF)
            {
                ThrowInvalidTokenError(token, "unexpected end of file");
            }
        }
        symTables_.pop_back();
        return compoundStatement;
    }

//------------------------------------------------------------------------------
    shared_ptr<SelectionStatement> Parser::ParseSelectionStatement_(CallerType& caller)
    {
        Token token = TakeTokenIf_(caller, KW_IF);
        if (token == KW_IF)
        {
            shared_ptr<SelectionStatement> selectionStatement = make_shared<SelectionStatement>();
            token = TakeTokenIf_(caller, OP_LPAREN);
            if (token == OP_LPAREN)
            {
                shared_ptr<ASTNode> conditionExpression = ParseExpression_(caller);
                token = TakeToken_(caller);
                if (token != OP_RPAREN)
                {
                    ThrowInvalidTokenError(token, "`)` expected to close selection-statement condition expression");
                }
                selectionStatement->SetConditionExpression(conditionExpression);
                shared_ptr<Statement> statementForIf = ParseStatement_(caller);
                selectionStatement->SetStatementForIf(statementForIf);
                token = TakeTokenIf_(caller, KW_ELSE);
                if (token == KW_ELSE)
                {
                    shared_ptr<Statement> statementForElse = ParseStatement_(caller);
                    selectionStatement->SetStatementForElse(statementForElse);
                }
                return selectionStatement;
            }
            else
            {
                ThrowInvalidTokenError(token, "`(` expected after `if`");
            }
        }
        else
        {
            return NULL;
        }
        return NULL;
    }

//------------------------------------------------------------------------------
    shared_ptr<IterationStatement> Parser::ParseIterationStatement_(CallerType& caller)
    {
        Token token = TakeToken_(caller);
        if (token == KW_FOR)
        {
            return ParseForStatement_(caller);
        }
        else if (token == KW_DO)
        {
            return ParseDoStatement_(caller);
        }
        else if (token == KW_WHILE)
        {
            return ParseWhileStatement_(caller);
        }
        else
        {
            ThrowInvalidTokenError(token, "unexpected in the iteration statement");
        }
        return NULL;
    }

//------------------------------------------------------------------------------
    shared_ptr<ForStatement> Parser::ParseForStatement_(Parser::CallerType& caller)
    {
        // `for` alread eaten
        Token token = TakeToken_(caller);
        if (token == OP_LPAREN)
        {
            shared_ptr<ForStatement> forStatement = make_shared<ForStatement>();
            iterationStatementStack_.push_back(forStatement);
            shared_ptr<SymbolTable> symbols = make_shared<SymbolTable>(EScopeType::LOOP);
            symTables_.push_back(symbols);
            forStatement->SetSymbolTable(symbols);

            token = TakeToken_(caller);
            if (IsDeclarationSpecifier_(token))
            {
                tokenStack_.push_back(token);
                ParseDeclaration_(caller);
            }
            else
            {
                shared_ptr<ASTNode> initializingExpression = NULL;
                if (token == OP_SEMICOLON)
                {
                    initializingExpression = make_shared<ASTNode>(Token(TT_INVALID, "stub"));
                }
                else
                {
                    tokenStack_.push_back(token);
                    initializingExpression = ParseExpression_(caller);
                    token = TakeToken_(caller);
                    if (token != OP_SEMICOLON)
                    {
                        ThrowInvalidTokenError(token, "`;` expected after clause-1 of `for` iteration-statement");
                    }
                }
                forStatement->SetInitializingExpression(initializingExpression);
            }

            token = TakeToken_(caller);
            shared_ptr<ASTNode> controllingExpression = NULL;
            if (token == OP_SEMICOLON)
            {
                controllingExpression = make_shared<ASTNode>(Token(TT_INVALID, "stub"));
            }
            else
            {
                tokenStack_.push_back(token);
                controllingExpression = ParseExpression_(caller);\
                token = TakeToken_(caller);
                if (token != OP_SEMICOLON)
                {
                    ThrowInvalidTokenError(token, "`;` expected after controlling expression of `for` iteration-statement");
                }
            }

            forStatement->SetControllingExpression(controllingExpression);

            token = TakeToken_(caller);
            shared_ptr<ASTNode> iterationExpression = NULL;
            if (token == OP_RPAREN)
            {
                iterationExpression = make_shared<ASTNode>(Token(TT_INVALID, "stub"));
            }
            else
            {
                tokenStack_.push_back(token);
                iterationExpression = ParseExpression_(caller);
                token = TakeToken_(caller);
                if (token != OP_RPAREN)
                {
                    ThrowInvalidTokenError(token, "`)` expected after iteration expression of `for` iteration-statement");
                }
            }

            forStatement->SetIterationExpression(iterationExpression);

            shared_ptr<Statement> statement = ParseStatement_(caller);
            symTables_.pop_back();
            forStatement->SetLoopStatement(statement);
            iterationStatementStack_.pop_back();

            return forStatement;
        }
        else
        {
            ThrowInvalidTokenError(token, "`(` expected after '`for`");
        }
        return NULL;
    }

//------------------------------------------------------------------------------
    shared_ptr<DoStatement> Parser::ParseDoStatement_(Parser::CallerType& caller)
    {
        // `do` already eaten
        shared_ptr<DoStatement> doStatement = make_shared<DoStatement>();
        iterationStatementStack_.push_back(doStatement);
        shared_ptr<Statement> loopStatement = ParseStatement_(caller);

        Token token = TakeToken_(caller);
        if (token != KW_WHILE)
        {
            ThrowInvalidTokenError(token, "`while` expected after loop-statement of `do` iteration-statement");
        }

        token = TakeToken_(caller);
        if (token != OP_LPAREN)
        {
            ThrowInvalidTokenError(token, "`(` expected before controlling-expression of `do` iteration-statement");
        }

        shared_ptr<ASTNode> controllingExpression = ParseExpression_(caller);

        token = TakeToken_(caller);
        if (token != OP_RPAREN)
        {
            ThrowInvalidTokenError(token, "`)` expected after controlling-expression of `do` iteration-statement");
        }

        token = TakeToken_(caller);
        if (token != OP_SEMICOLON)
        {
            ThrowInvalidTokenError(token, "`;` at the end of `do` iteration-statement");
        }

        doStatement->SetControllingExpression(controllingExpression);
        doStatement->SetLoopStatement(loopStatement);
        iterationStatementStack_.pop_back();
        return doStatement;
    }

//------------------------------------------------------------------------------
    shared_ptr<WhileStatement> Parser::ParseWhileStatement_(Parser::CallerType& caller)
    {
        // `while` already eaten
        shared_ptr<WhileStatement> whileStatement = make_shared<WhileStatement>();
        iterationStatementStack_.push_back(whileStatement);

        Token token = TakeToken_(caller);
        if (token != OP_LPAREN)
        {
            ThrowInvalidTokenError(token, "`(` expected before controlling-expression of `while` iteration-statement");
        }

        shared_ptr<ASTNode> controllingExpression = ParseExpression_(caller);

        token = TakeToken_(caller);
        if (token != OP_RPAREN)
        {
            ThrowInvalidTokenError(token, "`)` expected after controlling-expression of `while` iteration-statement");
        }

        shared_ptr<Statement> loopStatement = ParseStatement_(caller);

        whileStatement->SetControllingExpression(controllingExpression);
        whileStatement->SetLoopStatement(loopStatement);
        iterationStatementStack_.pop_back();
        return whileStatement;
    }

//------------------------------------------------------------------------------
    shared_ptr<JumpStatement> Parser::ParseJumpStatement_(CallerType& caller)
    {
        Token token = TakeToken_(caller);
        shared_ptr<JumpStatement> jumpStatement = make_shared<JumpStatement>(token);
        if (token == KW_RETURN)
        {
            token = TakeTokenIf_(caller, OP_SEMICOLON);
            if (token != OP_SEMICOLON)
            {
                shared_ptr<ASTNode> expression = ParseExpression_(caller);
                jumpStatement->SetReturnExpression(expression);
                token = TakeToken_(caller);
            }
        }
        else
        {
            if (iterationStatementStack_.empty())
            {
                ThrowError_("`break` or `continue` statement must belong to loop");
            }
            jumpStatement->SetRefLoopStatement(iterationStatementStack_.back());
            token = TakeToken_(caller);
        }

        if (token != OP_SEMICOLON)
        {
            ThrowInvalidTokenError(token, "semicolon `;` expected at the end of jump-statement");
        }
        return jumpStatement;
    }

//------------------------------------------------------------------------------
    shared_ptr<ExpressionStatement> Parser::ParseExpressionStatement_(CallerType& caller)
    {
        shared_ptr<ExpressionStatement> expressionStatement = make_shared<ExpressionStatement>();
        Token token = TakeTokenIf_(caller, OP_SEMICOLON);
        if (token != OP_SEMICOLON)
        {
            shared_ptr<ASTNode> expression = ParseExpression_(caller);
            expressionStatement->SetExpression(expression);
            token = TakeTokenIf_(caller, OP_SEMICOLON);
            if (token != OP_SEMICOLON)
            {
                ThrowInvalidTokenError(token, "semicolon expected at the end of expression-statement");
            }
        }
        return expressionStatement;
    }

//------------------------------------------------------------------------------
    shared_ptr<Statement> Parser::ParseStatement_(CallerType& caller)
    {
        Token token = TakeTokenIf_(caller, OP_LBRACE);
        switch (token.type)
        {
            case OP_LBRACE:
                return ParseCompoundStatement_(caller);

            case KW_IF:
                return ParseSelectionStatement_(caller);

            case KW_WHILE:
            case KW_FOR:
            case KW_DO:
                return ParseIterationStatement_(caller);

            case KW_CONTINUE:
            case KW_BREAK:
            case KW_RETURN:
                return ParseJumpStatement_(caller);

            default:
                return ParseExpressionStatement_(caller);
        }
    }

//------------------------------------------------------------------------------
    void Parser::ThrowError_(const std::string& descriptionText)
    {
        throw std::logic_error(descriptionText);
    }

//------------------------------------------------------------------------------
    void Parser::ResumeParse_(const Token& token)
    {
        parseCoroutine_(token);
    }

//------------------------------------------------------------------------------
    Token Parser::TakeToken_(CallerType& caller)
    {
        Token token;
        if (tokenStack_.empty())
        {
            caller();
            token = caller.get();
        }
        else
        {
            token = tokenStack_.back();
            tokenStack_.pop_back();
        }
        return token;
    }

//------------------------------------------------------------------------------
    Token Parser::TakeTokenIf_(Parser::CallerType& caller, const ETokenType& tokenType)
    {
        Token token = TakeToken_(caller);
        if (token != tokenType)
        {
            tokenStack_.push_back(token);
        }
        return token;
    }

//------------------------------------------------------------------------------
    Token Parser::TakeTokenIf_(Parser::CallerType& caller, bool condition)
    {
        Token token = TakeToken_(caller);
        if (!condition)
        {
            tokenStack_.push_back(token);
        }
        return token;
    }

//------------------------------------------------------------------------------
    void Parser::Flush() const
    {
        PrintSymbolTable(symTables_[1].get());
    }

//------------------------------------------------------------------------------
    bool Parser::IsDeclarationSpecifier_(const Token& token) const
    {
        if (token == TT_IDENTIFIER)
        {
            return LookupType(token.text) != NULL;
        }
        else
        {
            return token == KW_CONST
                    || token == KW_TYPEDEF
                    || token == KW_VOID
                    || token == KW_CHAR
                    || token == KW_INT
                    || token == KW_FLOAT
                    || token == KW_STRUCT;
        }
    }

//------------------------------------------------------------------------------
    bool Parser::IsSpecifierQualifier_(const Token& token) const
    {
        return IsDeclarationSpecifier_(token) && token != KW_TYPEDEF;
    }

//------------------------------------------------------------------------------
    std::string Parser::GenerateStuctName_()
    {
        anonymousGenerator_++;
        return "<struct-n-" + std::to_string(anonymousGenerator_) + ">";
    }

//------------------------------------------------------------------------------
    std::string Parser::GenerateParameterName_()
    {
        anonymousGenerator_++;
        return "<arg-n-" + std::to_string(anonymousGenerator_) + ">";
    }

//------------------------------------------------------------------------------
    shared_ptr<SymbolType> Parser::LookupType(const std::string& name) const
    {
        return LookupSymbolHelper_(name, &SymbolTable::LookupType);
    }

//------------------------------------------------------------------------------
    shared_ptr<SymbolVariable> Parser::LookupVariable(const std::string& name) const
    {
        return LookupSymbolHelper_(name, &SymbolTable::LookupVariable);
    }

//------------------------------------------------------------------------------
    shared_ptr<SymbolVariable> Parser::LookupFunction(const std::string& name) const
    {
        return LookupSymbolHelper_(name, &SymbolTable::LookupFunction);
    }

//------------------------------------------------------------------------------
    void Parser::AddType_(shared_ptr<SymbolType> symType)
    {
        assert(symType != NULL);
        AddType_(symType, symType->name);
    }

//------------------------------------------------------------------------------
    void Parser::AddType_(shared_ptr<SymbolType> symType, const std::string& name)
    {
        assert(symType != NULL);
        shared_ptr<SymbolTable> symbols = symTables_.back();

        switch (symType->GetType())
        {
            case ESymbolType::TYPE_STRUCT:
                if (symbols->GetScopeType() == EScopeType::PARAMETERS)
                {
                    ThrowError_("type declarations not allowed in parameter list");
                }
                break;

            default:
                break;
        }

        if (symbols->LookupType(name) != NULL)
        {
            ThrowError_("redefinition of type " + name);
        }

        symbols->AddType(symType, name);
    }

//------------------------------------------------------------------------------
    void Parser::AddVariable_(shared_ptr<SymbolVariable> symVar)
    {
        shared_ptr<SymbolTable> symbols = symTables_.back();
        if (symbols->LookupVariable(symVar->name) != NULL)
        {
            ThrowError_("redeclaration of " + symVar->GetQualifiedName());
        }

        if (symVar->GetRefSymbol()->GetType() == ESymbolType::TYPE_STRUCT)
        {
            shared_ptr<SymbolStruct> symStruct = static_pointer_cast<SymbolStruct>(symVar->GetRefSymbol());
            if (!symStruct->complete)
            {
                ThrowError_("type " + symStruct->GetQualifiedName()
                            + " is incomplete. cannot declare variable "
                            + symVar->name + " of incomplete type");
            }
        }

        symbols->AddVariable(symVar);
    }

//------------------------------------------------------------------------------
    void Parser::AddFunction_(shared_ptr<SymbolVariable> symFun)
    {
        shared_ptr<SymbolTable> symbols = symTables_.back();

        if (symTables_.size() == 2)
        {
            // adding function at global scope
            shared_ptr<SymbolVariable> symFunPresent = symbols->LookupFunction(symFun->name);
            if (symFunPresent != NULL)
            {
                shared_ptr<SymbolFunctionType> symFunTypePresent = static_pointer_cast<SymbolFunctionType>(symFunPresent->GetRefSymbol());
                shared_ptr<SymbolFunctionType> symFunTypeAdding = static_pointer_cast<SymbolFunctionType>(symFun->GetRefSymbol());
                if (!symFunTypePresent->IfTypeFits(symFunTypeAdding))
                {
                    ThrowError_("declaration of function " + symFun->name + " - incompatible signature");
                }
            }
            else
            {
                symbols->AddFunction(symFun);
            }
        }
        else
        {
            if (symbols->LookupVariable(symFun->name) != NULL)
            {
                ThrowError_("redeclaration of " + symFun->GetQualifiedName());
            }
            else
            {
                symbols->AddFunction(symFun);
            }
        }
    }

//------------------------------------------------------------------------------
    void Parser::EmitInvalid(const string &source, const int line, const int column)
    {
        Token token(TT_INVALID, source, line, column);
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void Parser::EmitKeyword(const string &source, ETokenType token_type, const int line, const int column)
    {
        Token token(token_type, source, line, column);
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void Parser::EmitPunctuation(const string &source, ETokenType token_type, const int line, const int column)
    {
        Token token(token_type, source, line, column);

        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void Parser::EmitIdentifier(const string &source, const int line, const int column)
    {
        Token token(TT_IDENTIFIER, source, line, column);
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void Parser::EmitLiteral(const string &source,
                             EFundamentalType type,
                             const void *data, size_t nbytes,
                             const int line, const int column)
    {
        UNUSED(nbytes);

        std::unordered_map<EFundamentalType, ETokenType> ftToTtMap =
        {
            {FT_INT, TT_LITERAL_INT},
            {FT_FLOAT, TT_LITERAL_FLOAT},
            {FT_CHAR, TT_LITERAL_CHAR},
        };

        assert(ftToTtMap.find(type) != ftToTtMap.end());

        Token token(ftToTtMap[type], source, line, column);
        token.size = nbytes;

        switch (type)
        {
            case FT_INT:
                token.intValue = *reinterpret_cast<const int*>(data);
                break;

            case FT_FLOAT:
                token.floatValue = *reinterpret_cast<const float*>(data);
                break;

            case FT_CHAR:
                token.intValue = *reinterpret_cast<const char*>(data);
                break;

            default:
                ThrowError_("invalid literal type");
                break;
        }
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void Parser::EmitLiteralArray(const string &source,
                                  size_t num_elements,
                                  EFundamentalType type,
                                  const void *data, size_t nbytes,
                                  const int line, const int column)
    {
        UNUSED(num_elements);
        assert(type == FT_CHAR);
        Token token(TT_LITERAL_CHAR_ARRAY, "\"" + source + "\"", line, column);
        token.charValue = new char [nbytes];
        memcpy(token.charValue, data, nbytes);
        token.size = nbytes;
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void Parser::EmitEof(const int line, const int column)
    {
        Token token(TT_EOF, "", line, column);
        ResumeParse_(token);
    }

} // namespace Compiler
