#pragma once

#include <iostream>
#include <exception>
#include <cassert>
#include <sstream>

#include "ITokenStream.hpp"
#include "constants.hpp"
#include "utils.hpp"

namespace Compiler
{
using std::cout;
using std::endl;
using std::logic_error;
using std::stringstream;

struct DebugTokenOutputStream : public ITokenStream
{
  // output: <line>-<column>: invalid <source>
  void EmitInvalid(const string& source, const int line, const int column)
  {
    stringstream ss;
    ss << "invalid token at " << FormatPos_(line, column) << source;
    //            throw logic_error(ss.str());
    cout << FormatPos_(line, column) << "invalid " << source << endl;
  }

  // output: <line>-<column>: keyword <token_type> <source>
  void EmitKeyword(const string& source, ETokenType token_type,
                   const int line, const int column)
  {
    cout << FormatPos_(line, column) << "keyword "
         << keywordTypeToStringMap.at(token_type) << " " << source << endl;
  }

  // output: <line>-<column>: punctuation <token_type> <source>
  void EmitPunctuation(const string& source, ETokenType token_type,
                       const int line, const int column)
  {
    cout << FormatPos_(line, column) << "punctuation "
         << punctuationTypeToStringMap.at(token_type) << " " << source <<  endl;
  }

  // output: <line>-<column>: identifier <source>
  void EmitIdentifier(const string& source, const int line, const int column)
  {
    cout << FormatPos_(line, column) << "identifier " << " " << source << endl;
  }

  // output: <line>-<column>: literal <type> <source> <hexdump(data,nbytes)>
  void EmitLiteral(const string& source, EFundamentalType type, const void* data,
                   size_t nbytes, const int line, const int column)
  {
    cout << FormatPos_(line, column) << "literal " << fundamentalTypeToStringMap.at(type)
         << " " << source << " " << HexDump(data, nbytes) << endl;
  }

  // output: <line>-<column>: literal <source> array of <num_elements> <type> <hexdump(data,nbytes)>
  void EmitLiteralArray(const string& source, size_t num_elements,
                        EFundamentalType type, const void* data, size_t nbytes,
                        const int line, const int column)
  {
    cout << FormatPos_(line, column) << "literal " << source << " array of "
         << num_elements << " " << fundamentalTypeToStringMap.at(type)
         << " " << HexDump(data, nbytes) << endl;
  }

  // output : eof
  void EmitEof(const int /*line*/, const int /*column*/)
  {
    cout << "eof" << endl;
  }

private:
  inline std::string FormatPos_(const int line, const int column)
  {
    std::string lineString = std::to_string(line);
    std::string columnString = std::to_string(column);
    assert(lineString.size() <= 4);
    assert(columnString.size() <= 3);
    return std::string(4 - lineString.size(), '.')
        + lineString
        + '-'
        + std::string(3 - columnString.size(), '.')
        + columnString
        + ": ";
  }
};

} // namespace Compiler
