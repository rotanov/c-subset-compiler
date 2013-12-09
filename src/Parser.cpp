#include "Parser.hpp"

#include <sstream>
#include <unordered_set>
#include <iostream>
#include <cassert>
#include <tuple>

#include <boost/bind.hpp>

#include "utils.hpp"
#include "prettyPrinting.hpp"

namespace Compiler
{
//------------------------------------------------------------------------------
    Parser::Parser()
    {
        // environment
        shared_ptr<SymbolTable> internalSymbols = make_shared<SymbolTable>(EScopeType::INTERNAL);
        internalSymbols->AddType(make_shared<SymbolChar>());
        internalSymbols->AddType(make_shared<SymbolInt>());
        internalSymbols->AddType(make_shared<SymbolFloat>());
        internalSymbols->AddType(make_shared<SymbolVoid>());
        symTables_.push_back(internalSymbols);

        shared_ptr<SymbolTable> globalSymbols = make_shared<SymbolTable>(EScopeType::GLOBAL);
        symTables_.push_back(globalSymbols);

        parseCoroutine_ = boost::move(Coroutine(boost::bind(&Parser::ParseTranslationUnit_, this, _1), Token()));
    }

//------------------------------------------------------------------------------
    Parser::~Parser()
    {

    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParseTopLevelExpression_(CallerType& caller)
    {
        while (true)
        {
            PrintAST(ParseExpression_(caller).get());
            std::cout << std::endl << std::endl;

            if (tokenStack_.empty())
            {
                throw std::runtime_error("token stack empty");
            }
            else if (tokenStack_.back() == TT_EOF)
            {
                break;
            }

        };

        FlushOutput_();
        return NULL;
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> Parser::ParsePrimaryExpression_(CallerType& caller)
    {
        Token token = WaitForTokenReady_(caller);
        switch(token.type)
        {
            case TT_IDENTIFIER:
            case TT_LITERAL_INT:
            case TT_LITERAL_FLOAT:
            case TT_LITERAL_CHAR:
            case TT_LITERAL_CHAR_ARRAY:
            {
                return make_shared<ASTNode>(token);
                break;
            }

            case OP_LPAREN:
            {
                shared_ptr<ASTNode> r = ParseExpression_(caller);

                token = WaitForTokenReady_(caller);
                if (token != OP_RPAREN)
                {
                    ThrowInvalidTokenError_(token, "')' expected");
                }
                return r;
                break;
            }

            default:
            {
                ThrowInvalidTokenError_(token, "unexpected in primary-expression");
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
            left = ParseUnaryExpression_(caller);
        }
        else
        {
            left = nodeStack_.back();
            nodeStack_.pop_back();
        }

        Token token = WaitForTokenReady_(caller);

        while (binaryOperatorTypeToPrecedence.find(token.type) != binaryOperatorTypeToPrecedence.end()
               && binaryOperatorTypeToPrecedence.at(token.type) >= priority)
        {
            left = make_shared<ASTNodeBinaryOperator>(token, left, ParseBinaryOperator_(caller, binaryOperatorTypeToPrecedence.at(token.type) + 1));
            token = WaitForTokenReady_(caller);
        }

        tokenStack_.push_back(token);
        return left;
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> Parser::ParseExpression_(CallerType& caller)
    {
        shared_ptr<ASTNode> node = ParseAssignmentExpression_(caller);
        Token token = WaitForTokenReady_(caller);
        while (token == OP_COMMA)
        {
            node = make_shared<ASTNodeBinaryOperator>(token, node, ParseAssignmentExpression_(caller));
            token = WaitForTokenReady_(caller);
        }

        tokenStack_.push_back(token);
        return node;
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> Parser::ParseAssignmentExpression_(CallerType& caller)
    {
        shared_ptr<ASTNode> node = ParseUnaryExpression_(caller);
        Token token = WaitForTokenReady_(caller);
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

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> Parser::ParseUnaryExpression_(CallerType& caller)
    {
        Token token = WaitForTokenReady_(caller);

        if (IsUnaryOperator(token.type)
            || token == OP_INC
            || token == OP_DEC)
        {
            shared_ptr<ASTNode> node = make_shared<ASTNodeUnaryOperator>(token);
            shared_ptr<ASTNode> root = node;
            token = WaitForTokenReady_(caller);
            while (IsUnaryOperator(token.type)
                   || token == OP_INC
                   || token == OP_DEC)
            {
                static_pointer_cast<ASTNodeUnaryOperator>(node)->SetOperand(make_shared<ASTNodeUnaryOperator>(token));
                node = static_pointer_cast<ASTNodeUnaryOperator>(node)->GetOperand();
                token = WaitForTokenReady_(caller);
            }
            tokenStack_.push_back(token);
            static_pointer_cast<ASTNodeUnaryOperator>(node)->SetOperand(ParsePostfixExpression_(caller));
            return root;
        }
        else
        {
            tokenStack_.push_back(token);
            return ParsePostfixExpression_(caller);
        }
    }

//------------------------------------------------------------------------------
    shared_ptr<ASTNode> Parser::ParseConditionalExpression_(CallerType& caller)
    {
        shared_ptr<ASTNode> conditionNode = ParseBinaryOperator_(caller, 0);
        Token token = WaitForTokenReady_(caller);

        if (token == OP_QMARK)
        {
            shared_ptr<ASTNode> thenExpressionNode = ParseExpression_(caller);
            Token colonToken = WaitForTokenReady_(caller);

            if (colonToken == OP_COLON)
            {
                return make_shared<ASTNodeConditionalOperator>(token, conditionNode,
                    thenExpressionNode, ParseConditionalExpression_(caller));
            }
            else
            {
                ThrowInvalidTokenError_(colonToken, "':' expected in conditional-expression");
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
        Token token = WaitForTokenReady_(caller);

        while (true)
        {
            switch (token.type)
            {
                case OP_LSQUARE:
                {
                    // postfix-expression '[' expression ']'
                    node = make_shared<ASTNodeArraySubscript>(token, node, ParseExpression_(caller));
                    token = WaitForTokenReady_(caller);
                    if (token != OP_RSQUARE)
                    {
                        ThrowInvalidTokenError_(token, "']' expected in postfix-expression");
                    }
                    token = WaitForTokenReady_(caller);
                    break;
                }

                case OP_LPAREN:
                {
                    // postfix-expression '(' {expression} ')' // argument-expression-list
                    Token prevToken = token;
                    token = WaitForTokenReady_(caller);

                    if (token != OP_RPAREN)
                    {
                        tokenStack_.push_back(token);
                        shared_ptr<ASTNodeFunctionCall> fcallNode = make_shared<ASTNodeFunctionCall>(prevToken, node);
                        node = fcallNode;

                        do
                        {
                            shared_ptr<ASTNode> nodeAssExpr = ParseAssignmentExpression_(caller);
                            fcallNode->AddArgumentExpressionNode(nodeAssExpr);
                            token = WaitForTokenReady_(caller);

                        } while (token == OP_COMMA);

                        if (token != OP_RPAREN)
                        {
                            ThrowInvalidTokenError_(token, "')' expected in postfix-expression");
                        }
                    }
                    else
                    {
                        node = make_shared<ASTNodeFunctionCall>(token, node);
                    }
                    token = WaitForTokenReady_(caller);
                    break;
                }

                case OP_DOT:
                case OP_ARROW:
                {
                    // postfix-expression '.' identifier
                    // postfix-expression '->' identifier
                    Token identifierToken = WaitForTokenReady_(caller);
                    if (identifierToken == TT_IDENTIFIER)
                    {
                        node = make_shared<ASTNodeStructureAccess>(token, node, make_shared<ASTNode>(identifierToken));
                    }
                    else
                    {
                        ThrowInvalidTokenError_(identifierToken, "identifier expected");
                    }
                    token = WaitForTokenReady_(caller);
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
                        token = WaitForTokenReady_(caller);
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
            token = WaitForTokenReady_(caller);

            bool constant = false;
            while (token == KW_CONST)
            {
                if (!constant)
                {
                    constant = true;
                    tail = make_shared<SymbolConst>(tail);
                }
                token = WaitForTokenReady_(caller);
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
        Token token = WithdrawTokenIf_(caller, OP_SEMICOLON);

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
        Token token = WaitForTokenReady_(caller);
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
                        ThrowInvalidTokenError_(token, "more than one `typedef` not allowed");
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
                        if (LookupType_(token.text) != NULL)
                        {
                            ThrowInvalidTokenError_(token, "only one type per declaration specification is expected");
                        }
                        else
                        {
                            identifierFound = true;
                            tokenStack_.push_back(token);
                        }
                    }
                    else
                    {
                        declSpec.typeSymbol = LookupType_(token.text);
                        if (declSpec.typeSymbol == NULL)
                        {
                            identifierFound = true;
                            tokenStack_.push_back(token);
                        }
                    }
                    break;

                default:
                    ThrowInvalidTokenError_(token, "unexpected in declaration");
                    break;
            }
            token = WaitForTokenReady_(caller);
        }
        tokenStack_.push_back(token);

        if (declSpec.typeSymbol == NULL)
        {
            ThrowError_("type not specified in declaration");
        }

        if (declSpec.typeSymbol->GetSymbolType() == ESymbolType::TYPE_TYPEDEF)
        {
            shared_ptr<SymbolTypedef> symTypedef = static_pointer_cast<SymbolTypedef>(declSpec.typeSymbol);
            declSpec.typeSymbol = symTypedef->GetTypeSymbol();
        }

        if (constant
            && declSpec.typeSymbol->GetSymbolType() != ESymbolType::TYPE_CONST)
        {
            declSpec.typeSymbol = make_shared<SymbolConst>(declSpec.typeSymbol);
        }

        return declSpec;
    }

//------------------------------------------------------------------------------
    shared_ptr<Symbol> Parser::ParseInitDeclaratorList_(Parser::CallerType& caller, DeclarationSpecifiers& declSpec)
    {
        shared_ptr<SymbolVariable> declarator = ParseDeclarator_(caller, declSpec);
        Token token = WaitForTokenReady_(caller);

        if (declarator->GetSymbolType() == ESymbolType::VARIABLE
            && declarator ->GetTypeSymbol()->GetSymbolType() == ESymbolType::TYPE_FUNCTION
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
                // Parse initializer
                // TODO: complex initializer
                ParseInitializer_(caller);
//                declarator->SetInitializer(ParseInitializer_(caller));
            }
            else if (token == OP_COMMA)
            {
                declarator = ParseDeclarator_(caller, declSpec);
                if (declarator->GetSymbolType() == ESymbolType::VARIABLE
                    && declarator->GetTypeSymbol()->GetSymbolType() == ESymbolType::TYPE_FUNCTION
                    && symTables_.size() == 2
                    && !declSpec.isTypedef)
                {
                    AddFunction_(declarator);
                }
            }
            else if (token != OP_SEMICOLON)
            {
                ThrowInvalidTokenError_(token, "`,` or `;` expected in init-declarator-list");
            }
            token = WaitForTokenReady_(caller);
        }

        return NULL;
    }

//------------------------------------------------------------------------------
    shared_ptr<SymbolVariable> Parser::ParseDeclarator_(Parser::CallerType& caller, DeclarationSpecifiers& declSpec)
    {
        shared_ptr<SymbolVariable> declaratorVariable = NULL;

        std::function<shared_ptr<Symbol>()> f = [&]() -> shared_ptr<Symbol>
        {
            shared_ptr<SymbolType> leftmostPointer = NULL;
            shared_ptr<SymbolType> rightmostPointer = NULL;
            shared_ptr<Symbol> centralType = NULL;
            Token token = WaitForTokenReady_(caller);

            // pointer part
            if (token == OP_STAR)
            {
                std::tie(leftmostPointer, rightmostPointer) = ParsePointer_(caller);
                token = WaitForTokenReady_(caller);
            }

            // direct declarator
            if (token == TT_IDENTIFIER)
            {
                declaratorVariable = make_shared<SymbolVariable>(token.text);
                centralType = declaratorVariable;
            }
            else if (token == OP_LPAREN)
            {
                centralType = f();
                token = WaitForTokenReady_(caller);
                if (token != OP_RPAREN)
                {
                    ThrowInvalidTokenError_(token, "`)` expected to close in declarator");
                }
            }
            else
            {
                // now we are sure there is no identifier
                // if we are in func parameters scope then we're allowed
                // to declare it anonymous, else it is treated as error
                shared_ptr<SymbolTable> topSymTable = symTables_.back();
                if (topSymTable->GetScopeType() == EScopeType::PARAMETERS)
                {
                    tokenStack_.push_back(token);
                    declaratorVariable = make_shared<SymbolVariable>(GenerateParameterName_());
                    centralType = declaratorVariable;
                }
                else
                {
                    ThrowInvalidTokenError_(token, "identifier or `(` expected");
                }
            }

            token = WaitForTokenReady_(caller);

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

                    token = WaitForTokenReady_(caller);

                    if (token != OP_RPAREN)
                    {
                        ThrowInvalidTokenError_(token, "closing `)` expected for parameter-list in declarator");
                    }
                }
                else if (token == OP_LSQUARE)
                {
                    shared_ptr<SymbolArray> type = make_shared<SymbolArray>();
                    temp = type;
                    token = WithdrawTokenIf_(caller, OP_RSQUARE);

                    if (token != OP_RSQUARE)
                    {
                        shared_ptr<ASTNode> sizeInitializer = ParseAssignmentExpression_(caller);
                        type->SetSizeInitializer(sizeInitializer);
                        token = WaitForTokenReady_(caller);
                        if (token != OP_RSQUARE)
                        {
                            ThrowInvalidTokenError_(token, "closing `)` expected for parameter-list in declarator");
                        }
                    }
                }

                rightmostType->SetTypeSymbol(temp);
                rightmostType = temp;

                token = WaitForTokenReady_(caller);
            }
            tokenStack_.push_back(token);

            if (rightmostPointer != NULL)
            {
                rightmostType->SetTypeSymbol(rightmostPointer);
                return leftmostPointer;
            }
            else
            {
                return rightmostType;
            }

        };

        f()->SetTypeSymbol(declSpec.typeSymbol);

        if (declSpec.isTypedef)
        {
            shared_ptr<SymbolTypedef> symTypedef = make_shared<SymbolTypedef>(declaratorVariable->name);
            symTypedef->SetTypeSymbol(declaratorVariable->GetTypeSymbol());
            AddType_(symTypedef);
        }
        else
        {
            if (declaratorVariable->GetTypeSymbol()->GetSymbolType() == ESymbolType::TYPE_FUNCTION)
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
                if (declaratorVariable->GetTypeSymbol()->GetSymbolType() == ESymbolType::TYPE_VOID)
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

        Token token = WaitForTokenReady_(caller);

        while (token != OP_RPAREN)
        {
            tokenStack_.push_back(token);

            DeclarationSpecifiers declSpec = ParseDeclarationSpecifiers_(caller);

            if (declSpec.isTypedef)
            {
                ThrowError_("typedef storage class specifier can not appear in parameter declaration");
            }

            ParseDeclarator_(caller, declSpec);

            token = WaitForTokenReady_(caller);

            if (token == OP_COMMA)
            {
                token = WaitForTokenReady_(caller);
            }
            else if (token != OP_RPAREN)
            {
                ThrowInvalidTokenError_(token, "`,` or `)` expected at the end of parameter declarator");
            }
        }
        tokenStack_.push_back(token);
    }

//------------------------------------------------------------------------------
    shared_ptr<Symbol> Parser::ParseInitializer_(Parser::CallerType& caller)
    {
        Token token = WithdrawTokenIf_(caller, OP_LBRACE);

        if (token == OP_LBRACE)
        {
            token = WaitForTokenReady_(caller);
        }
        else
        {
            shared_ptr<ASTNode> initializerExpression = ParseAssignmentExpression_(caller);
        }
        // TODO: return type/value, wtf?
        return NULL;
        // there was forgotten return
        // so it crashed
        // in release only
        // in one test only (out of 99)
        // wasted 6 hours in debugger
        // fought g++ cl keys
        // used print-debugging
        // used hardcore debugging with dasm, memview
        // so discovered this is UB
        // and without return it was
        // corrupting PARENT stack frame return address (in my case by half-byte)
        // and it was overall pain
        // vs treats this as an error by default
        // so
        // FUCK YOU GCC
        // don't you dare ever forgot return statement, guys
    }

//------------------------------------------------------------------------------
    shared_ptr<SymbolStruct> Parser::ParseStructSpecifier_(Parser::CallerType& caller)
    {
        // `struct` already parsed
        Token token = WithdrawTokenIf_(caller, TT_IDENTIFIER);
        Token tagToken;

        if (token == TT_IDENTIFIER)
        {
            tagToken = token;
        }
        else if (token != OP_LBRACE)
        {
            ThrowInvalidTokenError_(token, "anonymous struct shall be declared with struct-declaration-list");
        }

        // anonymous struct:
        if (tagToken != TT_IDENTIFIER)
        {
            tagToken = Token(TT_IDENTIFIER, GenerateStuctName_());
        }

        shared_ptr<SymbolType> symStructPresent = LookupType_(tagToken.text);

        shared_ptr<SymbolStruct> symStruct = NULL;
        if (symStructPresent != NULL)
        {
            if (symStructPresent->GetSymbolType() == ESymbolType::TYPE_STRUCT)
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

        token = WithdrawTokenIf_(caller, OP_LBRACE);

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
                token = WithdrawTokenIf_(caller, OP_SEMICOLON);

                while (token != OP_SEMICOLON)
                {
                    ParseDeclarator_(caller, declSpec);
                    token = WaitForTokenReady_(caller);
                    if (token != OP_COMMA
                        && token != OP_SEMICOLON)
                    {
                        token = WaitForTokenReady_(caller);
                    }
                }

                token = WithdrawTokenIf_(caller, OP_RBRACE);
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
        Token token = WithdrawTokenIf_(caller, TT_EOF);

        while (token != TT_EOF)
        {
            symbol = ParseDeclaration_(caller);
            if (symbol != NULL) // ? nullptr ?
            {
                shared_ptr<SymbolVariable> symFun = static_pointer_cast<SymbolVariable>(symbol);
                shared_ptr<SymbolFunctionType> symType = static_pointer_cast<SymbolFunctionType>(symFun->GetTypeSymbol());
                symTables_.push_back(symType->GetSymbolTable());
                // at this point symbol has ESymbolType::VARIABLE of type ESymbolType::TYPE_FUNCTION
                // and `{` already eaten
                symType->SetBody(ParseCompoundStatement_(caller));
                symTables_.pop_back();
                AddFunction_(symFun);
            }

            token = WithdrawTokenIf_(caller, TT_EOF);
        }
        // globals
        FlushOutput_();
    }

//------------------------------------------------------------------------------
    shared_ptr<CompoundStatement> Parser::ParseCompoundStatement_(CallerType& caller)
    {
        shared_ptr<SymbolTable> symTable = make_shared<SymbolTable>(EScopeType::BLOCK);
        symTables_.push_back(symTable);
        shared_ptr<CompoundStatement> compoundStatement = make_shared<CompoundStatement>(Token(OP_LBRACE), symTable);

        Token token = WithdrawTokenIf_(caller, OP_RBRACE);

        while (token != OP_RBRACE)
        {
            if (IsStartsDeclaration_(token))
            {
                // shall return NULL here
                ParseDeclaration_(caller);
            }
            else
            {
                compoundStatement->AddStatement(ParseStatement_(caller));
            }

            token = WithdrawTokenIf_(caller, OP_RBRACE);

            if (token == TT_EOF)
            {
                ThrowInvalidTokenError_(token, "unexpected end of file");
            }
        }
        symTables_.pop_back();
        return compoundStatement;
    }

//------------------------------------------------------------------------------
    shared_ptr<SelectionStatement> Parser::ParseSelectionStatement_(CallerType& caller)
    {
        Token token = WithdrawTokenIf_(caller, KW_IF);
        if (token == KW_IF)
        {
            shared_ptr<SelectionStatement> selectionStatement = make_shared<SelectionStatement>();
            token = WithdrawTokenIf_(caller, OP_LPAREN);
            if (token == OP_LPAREN)
            {
                shared_ptr<ASTNode> conditionExpression = ParseExpression_(caller);
                token = WaitForTokenReady_(caller);
                if (token != OP_RPAREN)
                {
                    ThrowInvalidTokenError_(token, "`)` expected to close selection-statement condition expression");
                }
                selectionStatement->SetConditionExpression(conditionExpression);
                shared_ptr<Statement> statementForIf = ParseStatement_(caller);
                selectionStatement->SetStatementForIf(statementForIf);
                token = WithdrawTokenIf_(caller, KW_ELSE);
                if (token == KW_ELSE)
                {
                    shared_ptr<Statement> statementForElse = ParseStatement_(caller);
                    selectionStatement->SetStatementForElse(statementForElse);
                }
                return selectionStatement;
            }
            else
            {
                ThrowInvalidTokenError_(token, "`(` expected after `if`");
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
        Token token = WaitForTokenReady_(caller);
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
            ThrowInvalidTokenError_(token, "unexpected in the iteration statement");
        }
        return NULL;
    }

//------------------------------------------------------------------------------
    shared_ptr<ForStatement> Parser::ParseForStatement_(Parser::CallerType& caller)
    {
        // `for` alread eaten
        Token token = WaitForTokenReady_(caller);
        if (token == OP_LPAREN)
        {
            shared_ptr<ForStatement> forStatement = make_shared<ForStatement>();
            iterationStatementStack_.push_back(forStatement);
            shared_ptr<SymbolTable> symbols = make_shared<SymbolTable>(EScopeType::LOOP);
            symTables_.push_back(symbols);
            forStatement->SetSymbolTable(symbols);

            token = WaitForTokenReady_(caller);
            if (IsStartsDeclaration_(token))
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
                    token = WaitForTokenReady_(caller);
                    if (token != OP_SEMICOLON)
                    {
                        ThrowInvalidTokenError_(token, "`;` expected after clause-1 of `for` iteration-statement");
                    }
                }
                forStatement->SetInitializingExpression(initializingExpression);
            }

            token = WaitForTokenReady_(caller);
            shared_ptr<ASTNode> controllingExpression = NULL;
            if (token == OP_SEMICOLON)
            {
                controllingExpression = make_shared<ASTNode>(Token(TT_INVALID, "stub"));
            }
            else
            {
                tokenStack_.push_back(token);
                controllingExpression = ParseExpression_(caller);\
                token = WaitForTokenReady_(caller);
                if (token != OP_SEMICOLON)
                {
                    ThrowInvalidTokenError_(token, "`;` expected after controlling expression of `for` iteration-statement");
                }
            }

            forStatement->SetControllingExpression(controllingExpression);

            token = WaitForTokenReady_(caller);
            shared_ptr<ASTNode> iterationExpression = NULL;
            if (token == OP_RPAREN)
            {
                iterationExpression = make_shared<ASTNode>(Token(TT_INVALID, "stub"));
            }
            else
            {
                tokenStack_.push_back(token);
                iterationExpression = ParseExpression_(caller);
                token = WaitForTokenReady_(caller);
                if (token != OP_RPAREN)
                {
                    ThrowInvalidTokenError_(token, "`)` expected after iteration expression of `for` iteration-statement");
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
            ThrowInvalidTokenError_(token, "`(` expected after '`for`");
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

        Token token = WaitForTokenReady_(caller);
        if (token != KW_WHILE)
        {
            ThrowInvalidTokenError_(token, "`while` expected after loop-statement of `do` iteration-statement");
        }

        token = WaitForTokenReady_(caller);
        if (token != OP_LPAREN)
        {
            ThrowInvalidTokenError_(token, "`(` expected before controlling-expression of `do` iteration-statement");
        }

        shared_ptr<ASTNode> controllingExpression = ParseExpression_(caller);

        token = WaitForTokenReady_(caller);
        if (token != OP_RPAREN)
        {
            ThrowInvalidTokenError_(token, "`)` expected after controlling-expression of `do` iteration-statement");
        }

        token = WaitForTokenReady_(caller);
        if (token != OP_SEMICOLON)
        {
            ThrowInvalidTokenError_(token, "`;` at the end of `do` iteration-statement");
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

        Token token = WaitForTokenReady_(caller);
        if (token != OP_LPAREN)
        {
            ThrowInvalidTokenError_(token, "`(` expected before controlling-expression of `while` iteration-statement");
        }

        shared_ptr<ASTNode> controllingExpression = ParseExpression_(caller);

        token = WaitForTokenReady_(caller);
        if (token != OP_RPAREN)
        {
            ThrowInvalidTokenError_(token, "`)` expected after controlling-expression of `while` iteration-statement");
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
        Token token = WaitForTokenReady_(caller);
        shared_ptr<JumpStatement> jumpStatement = make_shared<JumpStatement>(token);
        if (token == KW_RETURN)
        {
            token = WithdrawTokenIf_(caller, OP_SEMICOLON);
            if (token != OP_SEMICOLON)
            {
                shared_ptr<ASTNode> expression = ParseExpression_(caller);
                jumpStatement->SetReturnExpression(expression);
                token = WaitForTokenReady_(caller);
            }
        }
        else
        {
            if (iterationStatementStack_.empty())
            {
                ThrowError_("`break` or `continue` statement must belong to loop");
            }
            jumpStatement->SetRefLoopStatement(iterationStatementStack_.back());
            token = WaitForTokenReady_(caller);
        }

        if (token != OP_SEMICOLON)
        {
            ThrowInvalidTokenError_(token, "semicolon `;` expected at the end of jump-statement");
        }
        return jumpStatement;
    }

//------------------------------------------------------------------------------
    shared_ptr<ExpressionStatement> Parser::ParseExpressionStatement_(CallerType& caller)
    {
        shared_ptr<ExpressionStatement> expressionStatement = make_shared<ExpressionStatement>();
        Token token = WithdrawTokenIf_(caller, OP_SEMICOLON);
        if (token != OP_SEMICOLON)
        {
            shared_ptr<ASTNode> expression = ParseExpression_(caller);
            expressionStatement->SetExpression(expression);
            token = WithdrawTokenIf_(caller, OP_SEMICOLON);
            if (token != OP_SEMICOLON)
            {
                ThrowInvalidTokenError_(token, "semicolon expected at the end of expression-statement");
            }
        }
        return expressionStatement;
    }

//------------------------------------------------------------------------------
    shared_ptr<Statement> Parser::ParseStatement_(CallerType& caller)
    {
        Token token = WithdrawTokenIf_(caller, OP_LBRACE);
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
    void Parser::ThrowInvalidTokenError_(const Token &token, const std::string& descriptionText)
    {
        FlushOutput_();
        std::stringstream ss;
        ss << "unexpected token " << TokenTypeToString(token.type) << " : \""
           << token.text << "\" at " << token.line << "-" << token.column;
        if (!descriptionText.empty())
        {
            ss << ", " << descriptionText;
        }
        throw std::logic_error(ss.str());
    }

//------------------------------------------------------------------------------
    void Parser::ThrowError_(const std::string& descriptionText)
    {
        FlushOutput_();
        throw std::logic_error(descriptionText);
    }

//------------------------------------------------------------------------------
    void Parser::ResumeParse_(const Token& token)
    {
        parseCoroutine_(token);
    }

//------------------------------------------------------------------------------
    Token Parser::WaitForTokenReady_(CallerType& caller)
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
    Token Parser::WithdrawTokenIf_(Parser::CallerType& caller, const ETokenType& tokenType)
    {
        Token token = WaitForTokenReady_(caller);
        if (token != tokenType)
        {
            tokenStack_.push_back(token);
        }
        return token;
    }

//------------------------------------------------------------------------------
    Token Parser::WithdrawTokenIf_(Parser::CallerType& caller, bool condition)
    {
        Token token = WaitForTokenReady_(caller);
        if (!condition)
        {
            tokenStack_.push_back(token);
        }
        return token;
    }

//------------------------------------------------------------------------------
    void Parser::FlushOutput_()
    {
        PrintSymbolTable(symTables_[1].get());
    }

//------------------------------------------------------------------------------
    bool Parser::IsStartsDeclaration_(const Token& token) const
    {
        if (token == TT_IDENTIFIER)
        {
            return LookupType_(token.text) != NULL;
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
    shared_ptr<SymbolType> Parser::LookupType_(const std::string& name) const
    {
        return LookupSymbolHelper_(name, &SymbolTable::LookupType);
    }

//------------------------------------------------------------------------------
    shared_ptr<SymbolVariable> Parser::LookupVariable_(const std::string& name) const
    {
        return LookupSymbolHelper_(name, &SymbolTable::LookupVariable);
    }

//------------------------------------------------------------------------------
    shared_ptr<SymbolVariable> Parser::LookupFunction_(const std::string& name) const
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

        switch (symType->GetSymbolType())
        {
            case ESymbolType::TYPE_STRUCT:
                if (symbols->GetScopeType() == EScopeType::PARAMETERS)
                {
                    ThrowError_("type declarations not allowed in parameter list");
                }
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

        if (symVar->GetTypeSymbol()->GetSymbolType() == ESymbolType::TYPE_STRUCT)
        {
            shared_ptr<SymbolStruct> symStruct = static_pointer_cast<SymbolStruct>(symVar->GetTypeSymbol());
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
                shared_ptr<SymbolFunctionType> symFunTypePresent = static_pointer_cast<SymbolFunctionType>(symFunPresent->GetTypeSymbol());
                shared_ptr<SymbolFunctionType> symFunTypeAdding = static_pointer_cast<SymbolFunctionType>(symFun->GetTypeSymbol());

                if (symFunTypeAdding->GetBody() != NULL)
                {
                    if (symFunTypePresent->GetBody() != NULL)
                    {
                        ThrowError_("redefinition of " + symFun->GetQualifiedName());
                    }
                    else
                    {
                        symFunTypePresent->SetBody(symFunTypeAdding->GetBody());
                        // TODO: delete symFun
                    }
                }
                else
                {
                    // TODO: assure signatures are the same
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
        std::unordered_map<EFundamentalType, ETokenType> ftToTtMap =
        {
            {FT_INT, TT_LITERAL_INT},
            {FT_FLOAT, TT_LITERAL_FLOAT},
            {FT_CHAR, TT_LITERAL_CHAR},
        };

        assert(ftToTtMap.find(type) != ftToTtMap.end());

        Token token(ftToTtMap[type], source, line, column);

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
        assert(type == FT_CHAR);
        Token token(TT_LITERAL_CHAR_ARRAY, "\"" + source + "\"", line, column);
        token.charValue = new char [nbytes];
        memcpy(token.charValue, data, nbytes);
        ResumeParse_(token);
    }

//------------------------------------------------------------------------------
    void Parser::EmitEof(const int line, const int column)
    {
        Token token(TT_EOF, "", line, column);
        ResumeParse_(token);
    }

} // namespace Compiler
