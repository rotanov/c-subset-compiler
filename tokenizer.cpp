#include "tokenizer.hpp"

#include <iostream>
#include <climits>

#include "unicode.hpp"

namespace Compiler
{
    using std::cerr;

//------------------------------------------------------------------------------
    Tokenizer::Tokenizer(DebugTokenOutputStream &output)
        : output_(output)
        , codePoints_(NULL)
        , codePointsCount_(0)
        , codePointsAllocated_(NULL)
        , line_(1)
        , column_(1)
    {
        codePointsAllocated_ = 512;
        codePoints_ = new int[codePointsAllocated_];
    }

//------------------------------------------------------------------------------
    Tokenizer::~Tokenizer()
    {
        delete [] codePoints_;
    }

//------------------------------------------------------------------------------
    void Tokenizer::EmitWhitespaceSequence(const int rowOffset)
    {
        column_ += rowOffset;
    }

//------------------------------------------------------------------------------
    void Tokenizer::EmitNewLine()
    {
        line_++;
        column_ = 1;
    }

//------------------------------------------------------------------------------
    void Tokenizer::EmitIdentifier(const int* data, size_t size)
    {
        FlushAdjacentStringLiterals_();
        string utf8Data = UTF8CodePointToString(data, size);
        if (StringToKeywordTypeMap.find(utf8Data) != StringToKeywordTypeMap.end())
        {
            output_.EmitKeyword(utf8Data, StringToKeywordTypeMap.find(utf8Data)->second, line_, column_);
        }
        else if (StringToPunctuationTypeMap.find(utf8Data) != StringToPunctuationTypeMap.end())
        {
            output_.EmitPunctuation(utf8Data, StringToPunctuationTypeMap.find(utf8Data)->second, line_, column_);
        }
        else
        {
            output_.EmitIdentifier(utf8Data, line_, column_);
        }
        column_ += utf8Data.size();
    }

//------------------------------------------------------------------------------
    void Tokenizer::EmitPpNumber(const string &data)
    {
        FlushAdjacentStringLiterals_();
        DecodeInput_(data);

        enum
        {
            START,
            FINISH,
            HEX_LITERAL,
            FRACTIONAL_PART,
            EXPONENT,
        };

        int state = START;
        unsigned long long intValue = 0;
        float floatValue = 0.0;
        int base = 0;

        int i = 0;
        while (state != FINISH)
        {
            int* where = codePoints_ + i;
            int c = codePoints_[i];

            switch (state)
            {
    //------------------------------------------------------------------------------
            case START:
                if (MatchPrefixes("0x|0X", where))
                {
                    state = HEX_LITERAL;
                    i += 2;
                }
                else if (c == '.')
                {
                    if (!IsDigit(where[1]))
                    {
                        output_.EmitInvalid(data, line_, column_);
                        return;
                    }
                    i++;
                    state = FRACTIONAL_PART;
                }
                else if (IsDigit(c))
                {
                    while (IsDigit(c))
                    {
                        i++;
                        c = codePoints_[i];
                    }
                    if (c == '.')
                    {
                        i++;
                        state = FRACTIONAL_PART;
                    }
                    else if (c == 'e' || c == 'E')
                    {
                        i++;
                        state = EXPONENT;
                    }
                    else
                    {
                        if (codePoints_[0] == '0')
                        {
                            base = 8;
                            i = 0;
                            while('0' <= codePoints_[i]
                                  && codePoints_[i] <= '7')
                            {
                                i++;
                            }
                            state = FINISH;
                        }
                        else
                        {
                            base = 10;
                            i = 0;
                            while('0' <= codePoints_[i]
                                  && codePoints_[i] <= '9')
                            {
                                i++;
                            }
                            state = FINISH;
                        }
                    }
                }
                else
                {
                    output_.EmitInvalid(data, line_, column_);
                    return;
                }
                break;
    //------------------------------------------------------------------------------
            case HEX_LITERAL:
            {
                base = 16;
                int j = i;
                while(HexadecimalCharachters.count(where[0]) == 1)
                {
                    i++;
                    where = codePoints_ + i;
                }
                if (i == j)
                {
                    output_.EmitInvalid(data, line_, column_);
                    return;
                }
                state = FINISH;
            }
                break;
    //------------------------------------------------------------------------------
            case FRACTIONAL_PART:
                while(IsDigit(c))
                {
                    i++;
                    c = codePoints_[i];
                }

                if (c == 'e' || c == 'E')
                {
                    i++;
                    state = EXPONENT;
                }
                else
                {
                    state = FINISH;
                }
                break;

    //------------------------------------------------------------------------------
            case EXPONENT:
                if (c == '+' || c == '-')
                {
                    i++;
                }
                else if (IsDigit(c))
                {
                    while(IsDigit(c))
                    {
                        i++;
                        c = codePoints_[i];
                    }
                    state = FINISH;
                }
                else
                {
                    output_.EmitInvalid(data, line_, column_);
                    return;
                }
                break;

    //------------------------------------------------------------------------------
            case FINISH:
                break;
            }
        }
        if (i != codePointsCount_)
        {
            output_.EmitInvalid(data, line_, column_);
            return;
        }

        if (base != 0)
        {
            errno = 0;
            int* where = codePoints_;
            if (base == 16)
            {
                where += 2;
            }
            int* whereWas = where;
            intValue = wcstoull(reinterpret_cast<wchar_t*>(where),
                                reinterpret_cast<wchar_t**>(&where), base);
            if (errno == ERANGE || whereWas == where)
            {
                output_.EmitInvalid(data, line_, column_);
                return;
            }
            if (intValue <= INT_MAX)
            {
                int t = intValue;
                output_.EmitLiteral(data, FT_INT, &t, sizeof(t), line_, column_);
            }
            else
            {
                output_.EmitInvalid(data, line_, column_);
                return;
            }
        }
        else
        {
            string floatString = "";
            floatString = UTF8CodePointToString(codePoints_, 0, i);//where - codePoints_);
            floatValue = DecodeFloat(floatString);
            output_.EmitLiteral(data, FT_FLOAT, &floatValue, sizeof(floatValue), line_, column_);
//            double doubleValue = DecodeDouble(floatString);
//            output_.EmitLiteral(data, FT_DOUBLE, &doubleValue, sizeof(doubleValue), line_, row_);
        }
        column_ += codePointsCount_;
    }

//------------------------------------------------------------------------------
    void Tokenizer::EmitCharacterLiteral(const string &data)
    {
        FlushAdjacentStringLiterals_();
        DecodeInput_(data);
        codePointsCount_ = ReplaceEscapeSequences(codePoints_);

        if ((codePointsCount_ > 3 && codePoints_[0] == '\'')
                || codePointsCount_ > 4)
        {
            output_.EmitInvalid(data, line_, column_);
            cerr << "ERROR: multi code point character literals not supported: " << data << endl;
        }
        else if ((codePointsCount_ == 3 && codePoints_[0] != '\'')
                 || codePointsCount_ <= 2)
        {
            output_.EmitInvalid(data, line_, column_);
            cerr << "ERROR: Empty character literal." << data << endl;
        }
        else
        {
            if (codePoints_[0] == '\'')
            {
                if (codePoints_[1] < 0x80)
                {
                    char t = codePoints_[1];
                    output_.EmitLiteral(data, FT_CHAR, &t, 1, line_, column_);
                }
                else if (codePoints_[1] <= 0x10FFFF)
                {
                    output_.EmitLiteral(data, FT_INT, codePoints_ + 1, 4, line_, column_);
                }
                else
                {
                    output_.EmitInvalid(data, line_, column_);
                }
            }
        }
        column_ += codePointsCount_;
    }

//------------------------------------------------------------------------------
    void Tokenizer::EmitStringLiteral(const string &data)
    {
        DecodeInput_(data);
        StringLiteralRecord r;
        r.data = data;

        codePointsCount_ = ReplaceEscapeSequences(codePoints_);
        int* begin = codePoints_;
        int* end = codePoints_ + codePointsCount_ - 1;

        while (begin[0] != '"')
        {
            begin++;
        }
        begin++;
        while (end[0] != '"')
        {
            end--;
        }
        end--;

        r.codePoints.insert(r.codePoints.end(), begin, end + 1);
        PushStringRecord_(r);
        column_ += codePointsCount_;
    }

//------------------------------------------------------------------------------
    void Tokenizer::EmitPunctuation(const string &data)
    {
        FlushAdjacentStringLiterals_();
        if (StringToPunctuationTypeMap.find(data) != StringToPunctuationTypeMap.end())
        {
            output_.EmitPunctuation(data, StringToPunctuationTypeMap.find(data)->second, line_, column_);
        }
        else
        {
            output_.EmitInvalid(data, line_, column_);
        }
        column_ += data.size();
    }

//------------------------------------------------------------------------------
    void Tokenizer::EmitNonWhitespaceChar(const string &data)
    {
        FlushAdjacentStringLiterals_();
        output_.EmitInvalid(data, line_, column_);
        column_++;
    }

//------------------------------------------------------------------------------
    void Tokenizer::EmitEof()
    {
        FlushAdjacentStringLiterals_();
        output_.EmitEof();
    }

//------------------------------------------------------------------------------
    void Tokenizer::Flush()
    {
        FlushAdjacentStringLiterals_();
    }

//------------------------------------------------------------------------------
    void Tokenizer::DecodeInput_(const string &data)
    {
        unsigned i = 0;
        codePointsCount_ = 0;
        int* utf8Data = new int [data.size()];

        if (data.size() >= codePointsAllocated_)
        {
            while (codePointsAllocated_ <= data.size())
            {
                codePointsAllocated_ *= 2;
            }
            delete [] codePoints_;
            codePoints_ = new int[codePointsAllocated_];
        }

        while (i < data.size())
        {
            utf8Data[i] = static_cast<unsigned char>(data[i]);
            i++;
        }

        i = 0;
        while (i < data.size())
        {
            i += UTF8Decode(utf8Data + i, codePoints_[codePointsCount_]);
            codePointsCount_++;
        }

        delete [] utf8Data;
        codePoints_[codePointsCount_] = 0;
    }

//------------------------------------------------------------------------------
    void Tokenizer::PushStringRecord_(const Tokenizer::StringLiteralRecord &record)
    {
        stringLiterals_.push_back(record);
    }

//------------------------------------------------------------------------------
    void Tokenizer::FlushAdjacentStringLiterals_()
    {
        if (stringLiterals_.empty())
        {
            return;
        }

        string commonData = "";
        for (unsigned i = 0; i < stringLiterals_.size(); i++)
        {
            commonData += stringLiterals_[i].data;
            if (i != stringLiterals_.size() - 1)
            {
                commonData += ' ';
            }
        }

        vector<int> codePoints;
        for (unsigned i = 0; i < stringLiterals_.size(); i++)
        {
            StringLiteralRecord& l = stringLiterals_[i];
            codePoints.insert(codePoints.end(), l.codePoints.begin(), l.codePoints.end());
        }

        int j = 0;
        char* data = new char [codePoints.size() * 4 + 1];
        unsigned i = 0;
        while (i < codePoints.size())
        {
            j += UTF8Encode(codePoints[i], data + j);
            i++;
        }
        data[j] = 0;
        j++;
        output_.EmitLiteralArray(data, j, FT_CHAR, data, j, line_, column_);
        delete [] data;
        stringLiterals_.clear();
    }

} // namespace Compiler
