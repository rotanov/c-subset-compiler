#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stdexcept>

namespace Compiler
{
    using std::string;
    using std::vector;
    using std::unordered_set;
    using std::unordered_map;
    using std::map;

    constexpr int EndOfFile = -1;

    const unordered_set<int> simpleEscapeSequence_CodePoints =
    {
        '\'', '"', '?', '\\', 'a', 'b', 'f', 'n', 'r', 't', 'v',
    };

    const unordered_map<int, int> simpleEscapeSequence_Replacements =
    {
        {'\'', '\''},
        {'"', '"'},
        {'?', '?'},
        {'\\', '\\'},
        {'a', '\a'},
        {'b', '\b'},
        {'f', '\f'},
        {'n', '\n'},
        {'r', '\r'},
        {'t', '\t'},
        {'v', '\v'},
    };

    const unordered_set<int> hexadecimalCharachters =
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd',
        'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F',
    };

    const unordered_map<int, int> trigraphReplacement =
    {
        {'=', '#',},
        {'/', '\\',},
        {'\'', '^',},
        {'(', '[',},
        {')', ']',},
        {'!', '|',},
        {'<', '{',},
        {'>', '}',},
        {'-', '~',},
    };

    const unordered_set<string> punctuation4 =
    {
        "%:%:",
    };

    const unordered_set<string> punctuation3 =
    {
        "<<=", ">>=",
    };

    const unordered_set<string> punctuation2 =
    {
        "+=", "-=", "*=", "/=", "%=", "^=", "&=", "|=", "<<", ">>", "<=", ">=",
        "&&", "==", "!=", "||", "++", "--", "->", "##", "<:", ":>", "<%", "%>",
        "%:",
    };

    const unordered_set<string> punctuation1 =
    {
        "{", "}", "[", "]", "#", "(", ")", ";", ":", "?", ".", "+", "-", "*",
        "/", "%", "^", "&", "|", "~", "!", "=", "<", ">", ",",
    };

    const vector<unordered_set<string>> punctuation =
    {
        punctuation1, punctuation2, punctuation3, punctuation4,
    };

    // token type enum for
    enum ETokenType
    {
        // keywords
        KW_AUTO,
        KW_BREAK,
        KW_CASE,
        KW_CHAR,
        KW_CONST,
        KW_CONTINUE,
        KW_DEFAULT,
        KW_DO,
        KW_DOUBLE,
        KW_ELSE,
        KW_ENUM,
        KW_EXTERN,
        KW_FLOAT,
        KW_FOR,
        KW_GOTO,
        KW_IF,
        KW_INLINE,
        KW_INT,
        KW_LONG,
        KW_REGISTER,
        KW_RETURN,
        KW_SHORT,
        KW_SIGNED,
        KW_SIZEOF,
        KW_STATIC,
        KW_STRUCT,
        KW_SWITCH,
        KW_TYPEDEF,
        KW_UNION,
        KW_UNSIGNED,
        KW_VOID,
        KW_VOLATILE,
        KW_WHILE,

        // operators/punctuation
        OP_LBRACE,
        OP_RBRACE,
        OP_LSQUARE,
        OP_RSQUARE,
        OP_LPAREN,
        OP_RPAREN,
        OP_BOR,
        OP_XOR,
        OP_COMPL,
        OP_AMP,
        OP_LNOT,
        OP_SEMICOLON,
        OP_COLON,
        OP_QMARK,
        OP_DOT,
        OP_PLUS,
        OP_MINUS,
        OP_STAR,
        OP_DIV,
        OP_MOD,
        OP_ASS,
        OP_LT,
        OP_GT,
        OP_PLUSASS,
        OP_MINUSASS,
        OP_STARASS,
        OP_DIVASS,
        OP_MODASS,
        OP_XORASS,
        OP_BANDASS,
        OP_BORASS,
        OP_LSHIFT,
        OP_RSHIFT,
        OP_RSHIFTASS,
        OP_LSHIFTASS,
        OP_EQ,
        OP_NE,
        OP_LE,
        OP_GE,
        OP_LAND,
        OP_LOR,
        OP_INC,
        OP_DEC,
        OP_COMMA,
        OP_ARROW,

        // for consistency
        TT_IDENTIFIER,
        TT_LITERAL_INT,
        TT_LITERAL_FLOAT,
        TT_LITERAL_CHAR,
        TT_LITERAL_CHAR_ARRAY,
        TT_INVALID,
        TT_EOF,
        TT_EXPRESSION_STATEMENT,
    };

    enum EFundamentalType
    {
        FT_INVALID,
        FT_CHAR,
        FT_INT,
        FT_FLOAT,
        FT_DOUBLE,
    };

} // namespace Compiler

namespace std
{
    template<>
    struct hash<Compiler::ETokenType>
    {
        typedef Compiler::ETokenType argument_type;
        typedef std::underlying_type<argument_type>::type underlying_type;
        typedef std::hash<underlying_type>::result_type result_type;

        result_type operator ()(const argument_type& arg) const
        {
            std::hash<underlying_type> hasher;
            return hasher(static_cast<underlying_type>(arg));
        }
    };

    template<>
    struct hash<Compiler::EFundamentalType>
    {
        typedef Compiler::EFundamentalType argument_type;
        typedef std::underlying_type<argument_type>::type underlying_type;
        typedef std::hash<underlying_type>::result_type result_type;

        result_type operator ()(const argument_type& arg) const
        {
            std::hash<underlying_type> hasher;
            return hasher(static_cast<underlying_type>(arg));
        }
    };

}

namespace Compiler
{
    const unordered_map<string, ETokenType> stringToKeywordTypeMap =
    {
        // keywords
        {"break", KW_BREAK},
        {"char", KW_CHAR},
        {"const", KW_CONST},
        {"continue", KW_CONTINUE},
        {"do", KW_DO},
        {"else", KW_ELSE},
        {"float", KW_FLOAT},
        {"for", KW_FOR},
        {"if", KW_IF},
        {"int", KW_INT},
        {"return", KW_RETURN},
        {"sizeof", KW_SIZEOF},
        {"struct", KW_STRUCT},
        {"typedef", KW_TYPEDEF},
        {"void", KW_VOID},
        {"while", KW_WHILE},
    };

    const unordered_map<string, ETokenType> stringToPunctuationTypeMap =
    {
        // operators/punctuation
        {"{", OP_LBRACE},
        {"<%", OP_LBRACE},
        {"}", OP_RBRACE},
        {"%>", OP_RBRACE},
        {"[", OP_LSQUARE},
        {"<:", OP_LSQUARE},
        {"]", OP_RSQUARE},
        {":>", OP_RSQUARE},
        {"(", OP_LPAREN},
        {")", OP_RPAREN},
        {"|", OP_BOR},
        {"^", OP_XOR},
        {"~", OP_COMPL},
        {"&", OP_AMP},
        {"!", OP_LNOT},
        {";", OP_SEMICOLON},
        {":", OP_COLON},
        {"?", OP_QMARK},
        {".", OP_DOT},
        {"+", OP_PLUS},
        {"-", OP_MINUS},
        {"*", OP_STAR},
        {"/", OP_DIV},
        {"%", OP_MOD},
        {"=", OP_ASS},
        {"<", OP_LT},
        {">", OP_GT},
        {"+=", OP_PLUSASS},
        {"-=", OP_MINUSASS},
        {"*=", OP_STARASS},
        {"/=", OP_DIVASS},
        {"%=", OP_MODASS},
        {"^=", OP_XORASS},
        {"&=", OP_BANDASS},
        {"|=", OP_BORASS},
        {"<<", OP_LSHIFT},
        {">>", OP_RSHIFT},
        {">>=", OP_RSHIFTASS},
        {"<<=", OP_LSHIFTASS},
        {"==", OP_EQ},
        {"!=", OP_NE},
        {"<=", OP_LE},
        {">=", OP_GE},
        {"&&", OP_LAND},
        {"||", OP_LOR},
        {"++", OP_INC},
        {"--", OP_DEC},
        {",", OP_COMMA},
        {"->", OP_ARROW},
    };

    // map of enum to string
    const map<ETokenType, string> keywordTypeToStringMap =
    {
        {KW_AUTO, "KW_AUTO"},
        {KW_BREAK, "KW_BREAK"},
        {KW_CASE, "KW_CASE"},
        {KW_CHAR, "KW_CHAR"},
        {KW_CONST, "KW_CONST"},
        {KW_CONTINUE, "KW_CONTINUE"},
        {KW_DEFAULT, "KW_DEFAULT"},
        {KW_DO, "KW_DO"},
        {KW_DOUBLE, "KW_DOUBLE"},
        {KW_ELSE, "KW_ELSE"},
        {KW_ENUM, "KW_ENUM"},
        {KW_EXTERN, "KW_EXTERN"},
        {KW_FLOAT, "KW_FLOAT"},
        {KW_FOR, "KW_FOR"},
        {KW_GOTO, "KW_GOTO"},
        {KW_IF, "KW_IF"},
        {KW_INLINE, "KW_INLINE"},
        {KW_INT, "KW_INT"},
        {KW_LONG, "KW_LONG"},
        {KW_REGISTER, "KW_REGISTER"},
        {KW_RETURN, "KW_RETURN"},
        {KW_SHORT, "KW_SHORT"},
        {KW_SIGNED, "KW_SIGNED"},
        {KW_SIZEOF, "KW_SIZEOF"},
        {KW_STATIC, "KW_STATIC"},
        {KW_STRUCT, "KW_STRUCT"},
        {KW_SWITCH, "KW_SWITCH"},
        {KW_TYPEDEF, "KW_TYPEDEF"},
        {KW_UNION, "KW_UNION"},
        {KW_UNSIGNED, "KW_UNSIGNED"},
        {KW_VOID, "KW_VOID"},
        {KW_VOLATILE, "KW_VOLATILE"},
        {KW_WHILE, "KW_WHILE"},
    };

    const map<ETokenType, string> punctuationTypeToStringMap =
    {
        {OP_LBRACE, "OP_LBRACE"},
        {OP_RBRACE, "OP_RBRACE"},
        {OP_LSQUARE, "OP_LSQUARE"},
        {OP_RSQUARE, "OP_RSQUARE"},
        {OP_LPAREN, "OP_LPAREN"},
        {OP_RPAREN, "OP_RPAREN"},
        {OP_BOR, "OP_BOR"},
        {OP_XOR, "OP_XOR"},
        {OP_COMPL, "OP_COMPL"},
        {OP_AMP, "OP_AMP"},
        {OP_LNOT, "OP_LNOT"},
        {OP_SEMICOLON, "OP_SEMICOLON"},
        {OP_COLON, "OP_COLON"},
        {OP_QMARK, "OP_QMARK"},
        {OP_DOT, "OP_DOT"},
        {OP_PLUS, "OP_PLUS"},
        {OP_MINUS, "OP_MINUS"},
        {OP_STAR, "OP_STAR"},
        {OP_DIV, "OP_DIV"},
        {OP_MOD, "OP_MOD"},
        {OP_ASS, "OP_ASS"},
        {OP_LT, "OP_LT"},
        {OP_GT, "OP_GT"},
        {OP_PLUSASS, "OP_PLUSASS"},
        {OP_MINUSASS, "OP_MINUSASS"},
        {OP_STARASS, "OP_STARASS"},
        {OP_DIVASS, "OP_DIVASS"},
        {OP_MODASS, "OP_MODASS"},
        {OP_XORASS, "OP_XORASS"},
        {OP_BANDASS, "OP_BANDASS"},
        {OP_BORASS, "OP_BORASS"},
        {OP_LSHIFT, "OP_LSHIFT"},
        {OP_RSHIFT, "OP_RSHIFT"},
        {OP_RSHIFTASS, "OP_RSHIFTASS"},
        {OP_LSHIFTASS, "OP_LSHIFTASS"},
        {OP_EQ, "OP_EQ"},
        {OP_NE, "OP_NE"},
        {OP_LE, "OP_LE"},
        {OP_GE, "OP_GE"},
        {OP_LAND, "OP_LAND"},
        {OP_LOR, "OP_LOR"},
        {OP_INC, "OP_INC"},
        {OP_DEC, "OP_DEC"},
        {OP_COMMA, "OP_COMMA"},
        {OP_ARROW, "OP_ARROW"},
    };

    const map<ETokenType, string> otherTokenTypeToStringMap =
    {
        {TT_IDENTIFIER, "TT_IDENTIFIER"},
        {TT_LITERAL_INT, "TT_LITERAL_INT"},
        {TT_LITERAL_FLOAT, "TT_LITERAL_FLOAT"},
        {TT_LITERAL_CHAR, "TT_LITERAL_CHAR"},
        {TT_INVALID, "TT_INVALID"},
        {TT_LITERAL_CHAR_ARRAY, "TT_LITERAL_CHAR_ARRAY"},
        {TT_EOF, "TT_EOF"},
    };

    // convert EFundamentalType to a source code
    const unordered_map<EFundamentalType, string> fundamentalTypeToStringMap
    {
        {FT_CHAR, "char"},
        {FT_INT, "int"},
        {FT_FLOAT, "float"},
        {FT_DOUBLE, "double"},
    };

    const unordered_set<ETokenType> tokenTypeToRightAssociativity =
    {
        OP_ASS,
        OP_STARASS,
        OP_DIVASS,
        OP_MODASS,
        OP_PLUSASS,
        OP_MINUSASS,
        OP_LSHIFTASS,
        OP_RSHIFTASS,
        OP_BANDASS,
        OP_XORASS,
        OP_BORASS,
        OP_QMARK,
        OP_COLON,
        // also Unary 	 right to left 	 ++  --  +  -  !  ~  &  *  //(type_name)  C++: sizeof new delete
    };

    const unordered_set<ETokenType> unaryOperators =
    {
        OP_INC,
        OP_DEC,
        OP_PLUS,
        OP_MINUS,
        OP_LNOT,
        OP_COMPL,
        OP_AMP,
        OP_STAR,
    };

    const unordered_map<ETokenType, int> binaryOperatorTypeToPrecedence =
    {
        {OP_STAR, 9},
        {OP_DIV, 9},
        {OP_MOD, 9},
        //
        {OP_PLUS, 8},
        {OP_MINUS, 8},
        //
        {OP_LSHIFT, 7},
        {OP_RSHIFT, 7},
        //
        {OP_LT, 6},
        {OP_GT, 6},
        {OP_LE, 6},
        {OP_GE, 6},
        //
        {OP_EQ, 5},
        {OP_NE, 5},
        //
        {OP_AMP, 4},
        //
        {OP_XOR, 3},
        //
        {OP_BOR, 2},
        //
        {OP_LAND, 1},
        //
        {OP_LOR, 0},
    };

} // namespace Compiler
