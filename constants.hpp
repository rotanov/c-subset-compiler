#pragma once

#include <vector>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stdexcept>

namespace Compiler
{
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
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F',
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
        "<<=", ">>=", "...", "->*", /* surrogate */ "<::",
    };

    const unordered_set<string> Punctuation2 =
    {
        "+=", "-=", "*=", "/=", "%=", "^=", "&=", "|=", "<<", ">>", "<=", ">=", "&&",
        "==", "!=", "||", "++", "--", "->", ".*", "::", "##", "<:", ":>", "<%", "%>", "%:"
    };

    const unordered_set<string> Punctuation1 =
    {
        "{", "}", "[", "]", "#", "(", ")", ";", ":", "?", ".", "+", "-", "*", "/",
        "%", "^", "&", "|", "~", "!", "=", "<", ">", ",",
    };

    const vector<unordered_set<string>> Punctuation =
    {
        Punctuation1, Punctuation2, Punctuation3, Punctuation4,
    };

} // namespace Compiler
