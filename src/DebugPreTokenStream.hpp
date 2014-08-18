#pragma once

#include <string>
#include <iostream>

#include "utils.hpp"
#include "ipretokenstream.hpp"

namespace Compiler
{
using std::string;
using std::cout;
using std::endl;

struct DebugPreTokenStream : IPreTokenStream
{
  void EmitWhitespaceSequence(const int /*rowOffset*/)
  {
    cout << "whitespace-sequence 0 " << endl;
  }

  void EmitNewLine()
  {
    cout << "new-line 0 " << endl;
  }

  void EmitIdentifier(const int* data, size_t size)
  {
    string utf8Data = UTF8CodePointToString(data, size);
    write_token("identifier", utf8Data);
  }

  void EmitPpNumber(const string& data)
  {
    write_token("pp-number", data);
  }

  void EmitCharacterLiteral(const string& data)
  {
    write_token("character-literal", data);
  }

  void EmitStringLiteral(const string& data)
  {
    write_token("string-literal", data);
  }

  void EmitPunctuation(const string& data)
  {
    write_token("preprocessing-op-or-punc", data);
  }

  void EmitNonWhitespaceChar(const string& data)
  {
    write_token("non-whitespace-character", data);
  }

  void EmitEof()
  {
    cout << "eof" << endl;
  }

  virtual void Flush()
  {

  }

private:
  void write_token(const string& type, const string& data)
  {
    cout << type << " " << data.size() << " ";
    cout.write(data.data(), data.size());
    cout << endl;
  }
};

} // namespace Compiler
