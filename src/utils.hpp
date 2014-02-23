#pragma once

#include <string>
#include <vector>

#include "constants.hpp"

#define UNUSED(expression) do { (void)(expression); } while (0)

namespace Compiler
{
    using std::vector;
    using std::string;

    vector<string>& Split(const string& s, char delimiter, vector<string>& elems);
    vector<string> Split(const string& s, char delimiter);

    string UTF8CodePointToString(int c);

    // decodes codepoints sequence c from start index to end index inclusive
    string UTF8CodePointToString(const int* c, int start, int end);
    string UTF8CodePointToString(const int *source, size_t size);
    std::wstring UTF8CodePointToWString(const int *c, int start, int end);

    bool IsWhiteSpace(const int c);
    bool IsDigit(const int c);
    bool IsNonDigit(const int c);

    // given hex digit character c, return its value
    int HexCharToValue(int c);

    // convert integer [0,15] to hexadecimal digit
    char ValueToHexChar(const int c);

    float DecodeFloat(const string& s);
    double DecodeDouble(const string& s);
    long double DecodeLongDouble(const string& s);

    // hex dump memory range
    string HexDump(const void* pdata, const size_t nbytes);

    int MatchEscapeSequence(int* where);
    int ReplaceEscapeSequences(int* where);

    int MatchHexQuad(int* where, int& out);

    // returns if prefix symbol-matches "where" per symbol
    // caller must ensure [where, where + prefix.size()) are accessible
    // same applies to any other function below with similar semantic
    bool MatchPrefix(const std::string& prefix, int* where);

    bool MatchSubsequence(int* source, int from, int begin, int end);

    // splits string by |, if we want to match | then we're at fail
    // returns true if at least one prefix matched
    // if s == "u|ul|uLL" then first match returned keep that in mind
    int MatchPrefixes(const string& s, int* where);

    std::string TokenTypeToString(const ETokenType tokenType);

    bool IsUnaryOperator(const ETokenType& tokenType);
    bool IsAssignmentOperator(const ETokenType& tokenType);
    bool IsBinaryOperator(const ETokenType& tokenType);
    bool IsLiteral(const ETokenType& tokenType);
    bool IsStatement(const ETokenType& tokenType);

} // namespace Compiler
