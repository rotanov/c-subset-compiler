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
        OP_DOTS,
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
        TT_LITERAL,
        TT_INVALID,
        TT_LITERAL_ARRAY,
        TT_EOF,
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
    const unordered_map<string, ETokenType> StringToKeywordTypeMap =
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
    };

    const unordered_map<string, ETokenType> StringToPunctuationTypeMap =
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
    const map<ETokenType, string> KeywordTypeToStringMap =
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

    const map<ETokenType, string> PunctuationTypeToStringMap =
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
        {OP_DOTS, "OP_DOTS"},
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

    // convert EFundamentalType to a source code
    const unordered_map<EFundamentalType, string> FundamentalTypeToStringMap
    {
        {FT_CHAR, "char"},
        {FT_INT, "int"},
        {FT_FLOAT, "float"},
        {FT_DOUBLE, "double"},
    };

    const unordered_set<ETokenType> TokenTypeToRightAssociativity =
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

    const unordered_set<ETokenType> UnaryOperators =
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

    const unordered_map<ETokenType, int> TokenTypeToPrecedence =
    {
        {OP_LPAREN, 0},
        {OP_RPAREN, 0},
        {OP_LSQUARE, 0},
        {OP_RSQUARE, 0},
        {OP_DOT, 0},
        {OP_ARROW, 0},
        // Unary ++  --  +  -  !  ~  &  *  (type_name)  C++: sizeof new delete
        {OP_STAR, 2},
        {OP_DIV, 2},
        {OP_MOD, 2},
        //
        {OP_PLUS, 3},
        {OP_MINUS, 3},
        //
        {OP_LSHIFT, 4},
        {OP_RSHIFT, 4},
        //
        {OP_LT, 5},
        {OP_GT, 5},
        {OP_LE, 5},
        {OP_GE, 5},
        //
        {OP_EQ, 6},
        {OP_NE, 6},
        //
        {OP_AMP, 7},
        //
        {OP_XOR, 8},
        //
        {OP_BOR, 9},
        //
        {OP_LAND, 10},
        //
        {OP_LOR, 11},
        // conditional
        {OP_QMARK, 12},
        {OP_COLON, 12},
        //
        {OP_ASS, 13},
        {OP_STARASS, 13},
        {OP_DIVASS, 13},
        {OP_MODASS, 13},
        {OP_PLUSASS, 13},
        {OP_MINUSASS, 13},
        {OP_LSHIFTASS, 13},
        {OP_RSHIFTASS, 13},
        {OP_BANDASS, 13},
        {OP_XORASS, 13},
        {OP_BORASS, 13},
        //
        {OP_COMMA, 14},
    };

} // namespace Compiler
