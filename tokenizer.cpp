#include "tokenizer.hpp"

#include <iostream>

#include "unicode.hpp"

namespace Compiler
{
    using std::cerr;

//------------------------------------------------------------------------------
    Tokenizer::Tokenizer(DebugTokenOutputStream &output)
        : output_(output)
        , codePoints_(NULL)
        , codePointsAllocated_(NULL)
        , codePointsCount_(0)
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
    void Tokenizer::emit_whitespace_sequence()
    {

    }

//------------------------------------------------------------------------------
    void Tokenizer::emit_new_line()
    {

    }

//------------------------------------------------------------------------------
    void Tokenizer::emit_identifier(const string &data)
    {
        FlushAdjacentStringLiterals_();
        if (StringToTokenTypeMap.find(data) != StringToTokenTypeMap.end())
        {
            output_.emit_simple(data, StringToTokenTypeMap.find(data)->second);
        }
        else
        {
            output_.emit_identifier(data);
        }
    }

//------------------------------------------------------------------------------
    void Tokenizer::emit_pp_number(const string &data)
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
        EFundamentalType type = FT_INVALID;
        int* floatPartEnd = 0;
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
                        output_.emit_invalid(data);
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
                    output_.emit_invalid(data);
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
                    output_.emit_invalid(data);
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
                    output_.emit_invalid(data);
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
            output_.emit_invalid(data);
            return;
        }

        string floatString = "";
        if (floatPartEnd != 0)
        {
            floatString = UTF8CodePointToString(codePoints_, 0, floatPartEnd - codePoints_);
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
                output_.emit_invalid(data);
                return;
            }
            if (intValue <= INT_MAX)
            {
                int t = intValue;
                output_.emit_literal(data, FT_INT, &t, sizeof(t));
            }
            else
            {
                output_.emit_invalid(data);
                return;
            }
        }
        else
        {
            floatValue = DecodeFloat(floatString);
            output_.emit_literal(data, FT_FLOAT, &floatValue, sizeof(floatValue));
        }
    }

//------------------------------------------------------------------------------
    void Tokenizer::emit_character_literal(const string &data)
    {
        FlushAdjacentStringLiterals_();
        DecodeInput_(data);
        codePointsCount_ = ReplaceEscapeSequences(codePoints_);

        if (codePointsCount_ > 3 && codePoints_[0] == '\'' || codePointsCount_ > 4)
        {
            output_.emit_invalid(data);
            cerr << "ERROR: multi code point character literals not supported: " << data << endl;
        }
        else if (codePointsCount_ <= 2 || codePointsCount_ == 3 && codePoints_[0] != '\'')
        {
            output_.emit_invalid(data);
            cerr << "ERROR: Empty character literal." << data << endl;
        }
        else
        {
            if (codePoints_[0] == '\'')
            {
                if (codePoints_[1] < 0x80)
                {
                    char t = codePoints_[1];
                    output_.emit_literal(data, FT_CHAR, &t, 1);
                }
                else if (codePoints_[1] <= 0x10FFFF)
                {
                    output_.emit_literal(data, FT_INT, codePoints_ + 1, 4);
                }
                else
                {
                    output_.emit_invalid(data);
                }
            }
        }
    }

//------------------------------------------------------------------------------
    void Tokenizer::emit_string_literal(const string &data)
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
    }

//------------------------------------------------------------------------------
    void Tokenizer::emit_preprocessing_op_or_punc(const string &data)
    {
        FlushAdjacentStringLiterals_();
        if (StringToTokenTypeMap.find(data) != StringToTokenTypeMap.end())
        {
            output_.emit_simple(data, StringToTokenTypeMap.find(data)->second);
        }
        else
        {
            output_.emit_invalid(data);
        }
    }

//------------------------------------------------------------------------------
    void Tokenizer::emit_non_whitespace_char(const string &data)
    {
        FlushAdjacentStringLiterals_();
        output_.emit_invalid(data);
    }

//------------------------------------------------------------------------------
    void Tokenizer::emit_eof()
    {
        FlushAdjacentStringLiterals_();
        output_.emit_eof();
    }

//------------------------------------------------------------------------------
    void Tokenizer::DecodeInput_(const string &data)
    {
        int i = 0;
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
        for (int i = 0; i < stringLiterals_.size(); i++)
        {
            commonData += stringLiterals_[i].data;
            if (i != stringLiterals_.size() - 1)
            {
                commonData += ' ';
            }
        }

        vector<int> codePoints;
        for (int i = 0; i < stringLiterals_.size(); i++)
        {
            StringLiteralRecord& l = stringLiterals_[i];
            codePoints.insert(codePoints.end(), l.codePoints.begin(), l.codePoints.end());
        }

        int j = 0;
        char* data = new char [codePoints.size() * 4 + 1];
        int i = 0;
        while (i < codePoints.size())
        {
            j += UTF8Encode(codePoints[i], data + j);
            i++;
        }
        data[j] = 0;
        j++;
        output_.emit_literal_array(data, j, FT_CHAR, data, j);
        delete [] data;
        stringLiterals_.clear();
    }

} // namespace Compiler
