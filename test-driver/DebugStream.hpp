#pragma once

#include <streambuf>

#include <QPlainTextEdit>

class DebugStream : public std::basic_streambuf<char>
{
public:
    DebugStream(std::ostream& stream, QPlainTextEdit* text_edit);
    virtual ~DebugStream();

protected:
    virtual int_type overflow(int_type v);

    virtual std::streamsize xsputn(const char* p, std::streamsize n);

private:
    std::ostream& stream_;
    std::streambuf* oldBuffer_;
    std::string string_;
    QPlainTextEdit* logWindow_;
};
