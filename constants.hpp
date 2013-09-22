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

    const unordered_set<int> SimpleEscapeSequence_CodePoints =
    {
        '\'', '"', '?', '\\', 'a', 'b', 'f', 'n', 'r', 't', 'v',
    };

    const unordered_map<int, int> SimpleEscapeSequence_Replacements =
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

    const unordered_set<int> HexadecimalCharachters =
    {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd',
        'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F',
    };

    const unordered_map<int, int> TrigraphReplacement =
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

    const unordered_set<string> Punctuation4 =
    {
        "%:%:",
    };

    const unordered_set<string> Punctuation3 =
    {
        "<<=", ">>=", "...",
    };

    const unordered_set<string> Punctuation2 =
    {
        "+=", "-=", "*=", "/=", "%=", "^=", "&=", "|=", "<<", ">>", "<=", ">=",
        "&&", "==", "!=", "||", "++", "--", "->", "##", "<:", ":>", "<%", "%>",
        "%:",
    };

    const unordered_set<string> Punctuation1 =
    {
        "{", "}", "[", "]", "#", "(", ")", ";", ":", "?", ".", "+", "-", "*",
        "/", "%", "^", "&", "|", "~", "!", "=", "<", ">", ",",
    };

    const vector<unordered_set<string>> Punctuation =
    {
        Punctuation1, Punctuation2, Punctuation3, Punctuation4,
    };

    // token type enum for `simples`
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
        OP_DOTS,
        OP_QMARK,
        OP_COLON2,
        OP_DOT,
        OP_DOTSTAR,
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
        OP_ARROWSTAR,
        OP_ARROW,
    };

    // StringToETokenTypeMap map of `simple` `preprocessing-tokens` to ETokenType
    const unordered_map<string, ETokenType> StringToTokenTypeMap =
    {
        // keywords
        {"auto", KW_AUTO},
        {"break", KW_BREAK},
        {"case", KW_CASE},
        {"char", KW_CHAR},
        {"const", KW_CONST},
        {"continue", KW_CONTINUE},
        {"default", KW_DEFAULT},
        {"do", KW_DO},
        {"double", KW_DOUBLE},
        {"else", KW_ELSE},
        {"enum", KW_ENUM},
        {"extern", KW_EXTERN},
        {"float", KW_FLOAT},
        {"for", KW_FOR},
        {"goto", KW_GOTO},
        {"if", KW_IF},
        {"inline", KW_INLINE},
        {"int", KW_INT},
        {"long", KW_LONG},
        {"register", KW_REGISTER},
        {"return", KW_RETURN},
        {"short", KW_SHORT},
        {"signed", KW_SIGNED},
        {"sizeof", KW_SIZEOF},
        {"static", KW_STATIC},
        {"struct", KW_STRUCT},
        {"switch", KW_SWITCH},
        {"typedef", KW_TYPEDEF},
        {"union", KW_UNION},
        {"unsigned", KW_UNSIGNED},
        {"void", KW_VOID},
        {"volatile", KW_VOLATILE},
        {"while", KW_WHILE},

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
        {"bitor", OP_BOR},
        {"^", OP_XOR},
        {"xor", OP_XOR},
        {"~", OP_COMPL},
        {"compl", OP_COMPL},
        {"&", OP_AMP},
        {"bitand", OP_AMP},
        {"!", OP_LNOT},
        {"not", OP_LNOT},
        {";", OP_SEMICOLON},
        {":", OP_COLON},
        {"...", OP_DOTS},
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
        {"xor_eq", OP_XORASS},
        {"&=", OP_BANDASS},
        {"and_eq", OP_BANDASS},
        {"|=", OP_BORASS},
        {"or_eq", OP_BORASS},
        {"<<", OP_LSHIFT},
        {">>", OP_RSHIFT},
        {">>=", OP_RSHIFTASS},
        {"<<=", OP_LSHIFTASS},
        {"==", OP_EQ},
        {"!=", OP_NE},
        {"not_eq", OP_NE},
        {"<=", OP_LE},
        {">=", OP_GE},
        {"&&", OP_LAND},
        {"and", OP_LAND},
        {"||", OP_LOR},
        {"or", OP_LOR},
        {"++", OP_INC},
        {"--", OP_DEC},
        {",", OP_COMMA},
        {"->*", OP_ARROWSTAR},
        {"->", OP_ARROW}
    };

    // map of enum to string
    const map<ETokenType, string> TokenTypeToStringMap =
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
        {OP_DOTS, "OP_DOTS"},
        {OP_QMARK, "OP_QMARK"},
        {OP_COLON2, "OP_COLON2"},
        {OP_DOT, "OP_DOT"},
        {OP_DOTSTAR, "OP_DOTSTAR"},
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
        {OP_ARROWSTAR, "OP_ARROWSTAR"},
        {OP_ARROW, "OP_ARROW"},
    };

    enum EFundamentalType
    {
        FT_INVALID,
        FT_CHAR,
        FT_INT,
        FT_FLOAT,
    };

    // convert EFundamentalType to a source code
    const map<EFundamentalType, string> FundamentalTypeToStringMap
    {
        {FT_CHAR, "char"},
        {FT_INT, "int"},
        {FT_FLOAT, "float"},
    };

} // namespace Compiler
