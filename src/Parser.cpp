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
        SymbolTable* internalSymbols = new SymbolTable(EScopeType::INTERNAL);
        internalSymbols->AddType(new SymbolChar);
        internalSymbols->AddType(new SymbolInt);
        internalSymbols->AddType(new SymbolFloat);
        internalSymbols->AddType(new SymbolVoid);
        symTables_.push_back(internalSymbols);
        SymbolTable* globalSymbols = new SymbolTable(EScopeType::GLOBAL);
        symTables_.push_back(globalSymbols);
        parseCoroutine_ = boost::move(Coroutine(boost::bind(&Parser::ParseTranslationUnit_, this, _1), Token()));
    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParseTopLevelExpression_(CallerType& caller)
    {
        while (true)
        {
            PrintAST(ParseExpression_(caller));
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
    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParsePrimaryExpression_(CallerType& caller)
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
                return new ASTNode(token);
                break;
            }

            case OP_LPAREN:
            {
                ASTNode* r = ParseExpression_(caller);

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
    ASTNode* Parser::ParseBinaryOperator_(CallerType& caller, int priority)
    {
        ASTNode* left = NULL;

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
            left = new ASTNodeBinaryOperator(token, left, ParseBinaryOperator_(caller, binaryOperatorTypeToPrecedence.at(token.type) + 1));
            token = WaitForTokenReady_(caller);
        }

        tokenStack_.push_back(token);
        return left;
    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParseExpression_(CallerType& caller)
    {
        ASTNode* node = ParseAssignmentExpression_(caller);
        Token token = WaitForTokenReady_(caller);
        while (token == OP_COMMA)
        {
            node = new ASTNodeBinaryOperator(token, node, ParseAssignmentExpression_(caller));
            token = WaitForTokenReady_(caller);
        }

        tokenStack_.push_back(token);
        return node;
    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParseAssignmentExpression_(CallerType& caller)
    {
        ASTNode* node = ParseUnaryExpression_(caller);
        Token token = WaitForTokenReady_(caller);
        if (IsAssignmentOperator(token.type))
        {
            return new ASTNodeAssignment(token, node, ParseAssignmentExpression_(caller));
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
    ASTNode* Parser::ParseUnaryExpression_(CallerType& caller)
    {
        Token token = WaitForTokenReady_(caller);

        if (IsUnaryOperator(token.type)
            || token == OP_INC
            || token == OP_DEC)
        {
            ASTNode* node = new ASTNodeUnaryOperator(token);
            ASTNode* root = node;
            token = WaitForTokenReady_(caller);
            while (IsUnaryOperator(token.type)
                   || token == OP_INC
                   || token == OP_DEC)
            {
                static_cast<ASTNodeUnaryOperator*>(node)->SetOperand(new ASTNodeUnaryOperator(token));
                node = static_cast<ASTNodeUnaryOperator*>(node)->GetOperand();
                token = WaitForTokenReady_(caller);
            }
            tokenStack_.push_back(token);
            static_cast<ASTNodeUnaryOperator*>(node)->SetOperand(ParsePostfixExpression_(caller));
            return root;
        }
        else
        {
            tokenStack_.push_back(token);
            return ParsePostfixExpression_(caller);
        }
    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParseConditionalExpression_(CallerType& caller)
    {
        ASTNode* conditionNode = ParseBinaryOperator_(caller, 0);
        Token token = WaitForTokenReady_(caller);

        if (token == OP_QMARK)
        {
            ASTNode* thenExpressionNode = ParseExpression_(caller);
            Token colonToken = WaitForTokenReady_(caller);

            if (colonToken == OP_COLON)
            {
                return new ASTNodeConditionalOperator(token, conditionNode,
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
    }

//------------------------------------------------------------------------------
    ASTNode* Parser::ParsePostfixExpression_(CallerType& caller)
    {
        ASTNode* node = ParsePrimaryExpression_(caller);
        Token token = WaitForTokenReady_(caller);

        while (true)
        {
            switch (token.type)
            {
                case OP_LSQUARE:
                {
                    // postfix-expression '[' expression ']'
                    node = new ASTNodeArraySubscript(token, node, ParseExpression_(caller));
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
                        ASTNodeFunctionCall* fcallNode = new ASTNodeFunctionCall(prevToken, node);
                        node = fcallNode;

                        do
                        {
                            ASTNode* nodeAssExpr = ParseAssignmentExpression_(caller);
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
                        node = new ASTNodeFunctionCall(token, node);
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
                        node = new ASTNodeStructureAccess(token, node, new ASTNode(identifierToken));
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
                        node = new ASTNodeUnaryOperator(token, node);
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
    std::tuple<SymbolType*, SymbolType*> Parser::ParsePointer_(Parser::CallerType& caller, SymbolType* refType /*= NULL*/)
    {
        SymbolType* tail = new SymbolPointer();
        SymbolType* head = refType != NULL ? refType : tail;
        SymbolType* temp = refType;
        Token token;

        do
        {
            if (temp != NULL)
            {
                tail->SetTypeSymbol(temp);
            }

            token = WaitForTokenReady_(caller);

            while (token == KW_CONST)
            {
                tail->constant = true;
                token = WaitForTokenReady_(caller);
            }

            if (token == OP_STAR)
            {
                temp = tail;
                tail = new SymbolPointer();
            }

        } while (token == OP_STAR);

        tokenStack_.push_back(token);

        return std::make_tuple(head, tail);
    }

//------------------------------------------------------------------------------
    Symbol* Parser::ParseDeclaration_(Parser::CallerType& caller)
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
            return ParseInitDeclaratorList_(caller, declSpec);
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

        bool identifierFound = false;

        while (token != OP_SEMICOLON
               && token != OP_LPAREN
               && token != OP_STAR
               && !identifierFound)
        {
            switch (token)
            {
                case KW_CONST:
                    declSpec.isConst = true;
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

        return declSpec;
    }

//------------------------------------------------------------------------------
    Symbol* Parser::ParseInitDeclaratorList_(Parser::CallerType& caller, DeclarationSpecifiers& declSpec)
    {
        SymbolVariable* declarator = ParseOutermostDeclarator_(caller, declSpec);
        Token token = WaitForTokenReady_(caller);

        if (declarator->GetSymbolType() == ESymbolType::VARIABLE
            && declarator ->GetTypeSymbol()->GetSymbolType() == ESymbolType::TYPE_FUNCTION
            && token == OP_LBRACE)
        {
            return declarator;
        }

        while (token != OP_SEMICOLON)
        {
            if (token == OP_ASS)
            {
                // Parse initializer
                token = WaitForTokenReady_(caller);
            }
            else if (token == OP_COMMA)
            {
                ParseOutermostDeclarator_(caller, declSpec);
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
    SymbolVariable* Parser::ParseOutermostDeclarator_(Parser::CallerType& caller, DeclarationSpecifiers& declSpec)
    {
        SymbolVariable* declaratorVariable = NULL;
        Token token = WaitForTokenReady_(caller);

        SymbolType* leftmostPointer = declSpec.typeSymbol;
        SymbolType* rightmostPointer = NULL;
        Symbol* centralType = NULL;

        // pointer part
        if (token == OP_STAR)
        {
            std::tie(leftmostPointer, rightmostPointer) = ParsePointer_(caller, leftmostPointer);
            token = WaitForTokenReady_(caller);
        }
        else
        {
            rightmostPointer = leftmostPointer;
        }

        // direct declarator
        if (token == TT_IDENTIFIER)
        {
            // TODO: more complex check
            if (symTables_.back()->LookupVariable(token.text) != NULL)
            {
                ThrowInvalidTokenError_(token, "redeclaration of identifier");
            }
            declaratorVariable = new SymbolVariable(token.text);
            centralType = declaratorVariable;
        }
        else if (token == OP_LPAREN)
        {
            centralType = ParseInnerDeclarator_(caller, declaratorVariable);
            token = WaitForTokenReady_(caller);
            if (token != OP_RPAREN)
            {
                ThrowInvalidTokenError_(token, "`)` expected to close in declarator");
            }
        }
        else
        {
            ThrowInvalidTokenError_(token, "identifier or `(` expected");
        }

        token = WaitForTokenReady_(caller);

        Symbol* leftmostType = centralType;
        Symbol* rightmostType = centralType;
        SymbolType* temp = NULL;

        while (token == OP_LPAREN
               || token == OP_LSQUARE)
        {
            if (token == OP_LPAREN)
            {
                SymbolTableWithOrder* parametersSymTable = new SymbolTableWithOrder(EScopeType::PARAMETERS);
                SymbolFunctionType* type = new SymbolFunctionType(parametersSymTable);
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
                SymbolArray* type = new SymbolArray();
                temp = type;
                ASTNode* initializer = ParseAssignmentExpression_(caller);
                type->SetInitializer(initializer);
                token = WaitForTokenReady_(caller);
                if (token != OP_RSQUARE)
                {
                    ThrowInvalidTokenError_(token, "closing `)` expected for parameter-list in declarator");
                }
            }

            rightmostType->SetTypeSymbol(temp);
            rightmostType = temp;

            token = WaitForTokenReady_(caller);
        }
        rightmostType->SetTypeSymbol(rightmostPointer);
        tokenStack_.push_back(token);

        //typedef int a()[];
//        void* a;

        if (declSpec.isTypedef)
        {
//            AddType_();
        }
        else
        {
            if (declaratorVariable->GetTypeSymbol()->GetSymbolType() == ESymbolType::TYPE_FUNCTION)
            {
                AddFunction_(declaratorVariable);
            }
            else
            {
                if (declaratorVariable->GetTypeSymbol()->GetSymbolType() == ESymbolType::TYPE_VOID)
                {
                    ThrowError_("variable " + declaratorVariable->name + " declared as void");
                }
                AddVariable_(declaratorVariable);
            }
        }

        return declaratorVariable;
    }

//------------------------------------------------------------------------------
    Symbol* Parser::ParseInnerDeclarator_(Parser::CallerType& caller, SymbolVariable*& declaratorVariable)
    {
        Token token = WaitForTokenReady_(caller);

        SymbolType* leftmostPointer = NULL;
        SymbolType* rightmostPointer = NULL;
        Symbol* centralType = NULL;
        Symbol* leftmostFirstType = NULL;
        Symbol* rightmostFirstType = NULL;

        // pointer part
        if (token == OP_STAR)
        {
            std::tie(leftmostPointer, rightmostPointer) = ParsePointer_(caller);
            token = WaitForTokenReady_(caller);
            leftmostFirstType = leftmostPointer;
            rightmostFirstType = rightmostPointer;
        }

        // direct declarator
        if (token == TT_IDENTIFIER)
        {
            // TODO: more complex check
            if (symTables_.back()->LookupVariable(token.text) != NULL)
            {
                ThrowInvalidTokenError_(token, "redeclaration of identifier");
            }
            declaratorVariable = new SymbolVariable(token.text);
            centralType = declaratorVariable;
        }
        else if (token == OP_LPAREN)
        {
            centralType = ParseInnerDeclarator_(caller, declaratorVariable);
            token = WaitForTokenReady_(caller);
            if (token != OP_RPAREN)
            {
                ThrowInvalidTokenError_(token, "`)` expected to close in declarator");
            }
        }
        else
        {
            ThrowInvalidTokenError_(token, "identifier or `(` expected");
        }

        token = WaitForTokenReady_(caller);

        Symbol* leftmostType = centralType;
        Symbol* rightmostType = centralType;
        SymbolType* temp = NULL;

        while (token == OP_LPAREN
               || token == OP_LSQUARE)
        {
            if (token == OP_LPAREN)
            {
                SymbolTableWithOrder* parametersSymTable = new SymbolTableWithOrder(EScopeType::PARAMETERS);
                SymbolFunctionType* type = new SymbolFunctionType(parametersSymTable);
                temp = type;
                symTables_.push_back(parametersSymTable);

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
                SymbolArray* type = new SymbolArray();
                temp = type;
                ASTNode* initializer = ParseAssignmentExpression_(caller);
                type->SetInitializer(initializer);
                token = WaitForTokenReady_(caller);
                if (token != OP_RSQUARE)
                {
                    ThrowInvalidTokenError_(token, "closing `)` expected for parameter-list in declarator");
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

            ParseOutermostDeclarator_(caller, declSpec);

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
    SymbolStruct* Parser::ParseStructSpecifier_(Parser::CallerType& caller)
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

        SymbolTableWithOrder* fieldsSymTable = new SymbolTableWithOrder(EScopeType::STRUCTURE);
        SymbolStruct* symStruct = new SymbolStruct(fieldsSymTable, tagToken.text);
        AddType_(symStruct);
        symTables_.push_back(fieldsSymTable);

        token = WaitForTokenReady_(caller);

        if (token == OP_LBRACE)
        {
            while (token != OP_RBRACE)
            {
                DeclarationSpecifiers declSpec = ParseDeclarationSpecifiers_(caller);
                if (declSpec.isTypedef)
                {
                    ThrowError_("struct declaration can not contain typedef");
                }

                while (token != OP_SEMICOLON)
                {
                    ParseOutermostDeclarator_(caller, declSpec);
                    token = WaitForTokenReady_(caller);
                    if (token != OP_COMMA
                        && token != OP_SEMICOLON)
                    {
                        token = WaitForTokenReady_(caller);
                    }
                }

                token = WithdrawTokenIf_(caller, OP_RBRACE);
            }
        }

        symTables_.pop_back();
        return symStruct;
    }

//------------------------------------------------------------------------------
    void Parser::ParseTranslationUnit_(CallerType& caller)
    {
        Symbol* symbol = NULL;
        Token token = WithdrawTokenIf_(caller, TT_EOF);

        while (token != TT_EOF)
        {
            symbol = ParseDeclaration_(caller);
            if (symbol != NULL)
            {
                SymbolVariable* symFun = static_cast<SymbolVariable*>(symbol);
                SymbolFunctionType* symType = static_cast<SymbolFunctionType*>(symFun->GetTypeSymbol());
                symTables_.push_back(symType->GetSymbolTable());
                // at this point symbol has ESymbolType::VARIABLE of type ESymbolType::TYPE_FUNCTION
                // and `{` already eaten
                symType->SetBody(ParseCompoundStatement_(caller));
                symTables_.pop_back();
            }

            token = WithdrawTokenIf_(caller, TT_EOF);
        }
        // globals
        FlushOutput_();
    }

//------------------------------------------------------------------------------
    CompoundStatement* Parser::ParseCompoundStatement_(CallerType& caller)
    {
        SymbolTable* symTable = new SymbolTable(EScopeType::BLOCK);
        symTables_.push_back(symTable);
        CompoundStatement* compoundStatement = new CompoundStatement(Token(OP_LBRACE), symTable);

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
    SelectionStatement* Parser::ParseSelectionStatement_(CallerType& caller)
    {
        return NULL;
    }

//------------------------------------------------------------------------------
    IterationStatement* Parser::ParseIterationStatement_(CallerType& caller)
    {
        return NULL;
    }

//------------------------------------------------------------------------------
    JumpStatement* Parser::ParseJumpStatement_(CallerType& caller)
    {
        Token token = WaitForTokenReady_(caller);
        JumpStatement* jumpStatement = new JumpStatement(token);
        if (token == KW_RETURN)
        {
            token = WithdrawTokenIf_(caller, OP_SEMICOLON);
            if (token != OP_SEMICOLON)
            {
                ASTNode* expression = ParseExpression_(caller);
                jumpStatement->SetReturnExpression(expression);
                token = WaitForTokenReady_(caller);
            }
        }
        else
        {
            token = WaitForTokenReady_(caller);
        }

        if (token != OP_SEMICOLON)
        {
            ThrowInvalidTokenError_(token, "semicolon `;` expected at the end of jump-statement");
        }
        return jumpStatement;
    }

//------------------------------------------------------------------------------
    ExpressionStatement* Parser::ParseExpressionStatement_(CallerType& caller)
    {
        ExpressionStatement* expressionStatement = new ExpressionStatement();
        Token token = WithdrawTokenIf_(caller, OP_SEMICOLON);
        if (token != OP_SEMICOLON)
        {
            ASTNode* expression = ParseExpression_(caller);
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
    Statement* Parser::ParseStatement_(CallerType& caller)
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
    void Parser::FlushOutput_()
    {
        PrintSymbolTable(symTables_[1]);
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
    SymbolType* Parser::LookupType_(const std::string& name) const
    {
        return LookupSymbolHelper_(name, &SymbolTable::LookupType);
    }

//------------------------------------------------------------------------------
    SymbolVariable* Parser::LookupVariable_(const std::string& name) const
    {
        return LookupSymbolHelper_(name, &SymbolTable::LookupVariable);
    }

//------------------------------------------------------------------------------
    SymbolVariable* Parser::LookupFunction_(const std::string& name) const
    {
        return LookupSymbolHelper_(name, &SymbolTable::LookupFunction);
    }

//------------------------------------------------------------------------------
    void Parser::AddType_(SymbolType* symType)
    {
        SymbolTable* symbols = symTables_.back();
        switch (symType->GetSymbolType())
        {
            case ESymbolType::TYPE_STRUCT:
                if (symbols->GetScopeType() == EScopeType::PARAMETERS)
                {
                    ThrowError_("type declarations not allowed in parameter list");
                }
                break;

        }

        if (symbols->LookupType(symType->name) != NULL)
        {
            ThrowError_("redefinition of type " + symType->name);
        }

        symbols->AddType(symType);
    }

//------------------------------------------------------------------------------
    void Parser::AddVariable_(SymbolVariable* symVar)
    {
        SymbolTable* symbols = symTables_.back();
        if (symbols->LookupVariable(symVar->name) != NULL)
        {
            ThrowError_("redeclaration of " + symVar->GetQualifiedName());
        }

        symbols->AddVariable(symVar);
    }

//------------------------------------------------------------------------------
    void Parser::AddFunction_(SymbolVariable* symFun)
    {
        SymbolTable* symbols = symTables_.back();
        if (symbols->LookupVariable(symFun->name) != NULL)
        {
            ThrowError_("redeclaration of " + symFun->GetQualifiedName());
        }

        symbols->AddFunction(symFun);
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
