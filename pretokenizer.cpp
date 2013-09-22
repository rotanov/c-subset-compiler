#include "pretokenizer.hpp"

#include <exception>

#include "constants.hpp"
#include "utils.hpp"
#include "unicode.hpp"

namespace Compiler
{
    using std::logic_error;
//------------------------------------------------------------------------------
    PreTokenizer::PreTokenizer(const std::vector<char> &input, IPreTokenStream &output)
        : state_(TS_START)
        , sourceSize_(input.size() + 1)
        , output_(output)
        , outStart_(0)
        , pos_(0)
    {
        source_ = new int [sourceSize_];
        for (unsigned i = 0; i < input.size(); i++)
        {
            source_[i] = static_cast<unsigned char>(input[i]);
        }
        source_[sourceSize_ - 1] = EndOfFile;

        DecodeUTF_();
        ProcessTrigraphs_();
// No Universal Character Names
//        ProcessUCN_();
        ProcessSplicing_();
        Process_();
    }

//------------------------------------------------------------------------------
    void PreTokenizer::DecodeUTF_()
    {
        int codePoint = 0;
        int* i = source_;
        int* j = source_;

        while (i[0] != EndOfFile)
        {
            i += UTF8Decode(i, codePoint);
            j[0] = codePoint;
            j++;
        }

        j[0] = EndOfFile;
        sourceSize_ = j - source_ + 1;
    }

//------------------------------------------------------------------------------
    void PreTokenizer::ProcessTrigraphs_()
    {
            int* i = source_;
            int* j = source_;
            while (i[0] != EndOfFile)
            {
                if (MatchPrefix("??", i))
                {
                    if (TrigraphReplacement.find(i[2]) != TrigraphReplacement.end())
                    {

                        j[0] = TrigraphReplacement.find(i[2])->second;
                        i += 3;
                        j++;
                        continue;
                    }
                }
                j[0] = i[0];
                i++;
                j++;
            }
            j[0] = EndOfFile;
        }

//------------------------------------------------------------------------------
    void PreTokenizer::ProcessSplicing_()
    {
        int* i = source_;
        int* j = source_;
        while (i[0] != EndOfFile)
        {
            if (MatchPrefix("\\\n", i))
            {
                i += 2;
                continue;
            }
            j[0] = i[0];
            i++;
            j++;
        }
        j[0] = EndOfFile;
    }

//------------------------------------------------------------------------------
    void PreTokenizer::Process_()
    {
        while (state_ != TS_FINISHED)
        {
            int* where = source_ + pos_;
            int c = where[0];

            switch (state_)
            {
////////////////////////////////////////////////////////////////////////////////
            case TS_START:
//------------------------------------------------------------------------------
                if (IsWhiteSpace(c))
                {
                    state_ = TS_WHITESPACE;
                    pos_++;
                }
//------------------------------------------------------------------------------
                else if (MatchPrefix("//", where))
                {
                    pos_ += 2;
                    state_ = TS_COMMENT;
                }
//------------------------------------------------------------------------------
                else if (MatchPrefix("/*", where))
                {
                    state_ = TS_INLINE_COMMENT;
                    pos_ += 2;
                }
//------------------------------------------------------------------------------
                else if (c == '\n')
                {
                    output_.emit_new_line();
                    pos_++;
                }
//------------------------------------------------------------------------------
                else if (c == '\'')
                {
                    outStart_ = pos_;
                    pos_++;
                    state_ = TS_MATCHING_CHARACTER_LITERAL;
                }
//------------------------------------------------------------------------------
                else if (IsDigit(c))
                {
                    outStart_ = pos_;
                    pos_++;
                    state_ = TS_MATCHING_PP_NUMBER;
                }
//------------------------------------------------------------------------------
                else if (c == '.' && IsDigit((where + 1)[0]))
                {
                    outStart_ = pos_;
                    pos_ += 2;
                    state_ = TS_MATCHING_PP_NUMBER;
                }
//------------------------------------------------------------------------------
                else if (c == '"')
                {
                    outStart_ = pos_;
                    pos_++;
                    state_ = TS_MATCHING_S_CHAR_SEQUENCE;
                }
//------------------------------------------------------------------------------
                else if (c == '\'')
                {
                    outStart_ = pos_;
                    pos_++;
                    state_ = TS_MATCHING_CHARACTER_LITERAL;
                }
//------------------------------------------------------------------------------
                else if (IsNonDigit(c))
                {
                    outStart_ = pos_;
                    pos_++;
                    state_ = TS_MATCHING_IDENTIFIER;
                }
//------------------------------------------------------------------------------
                else if (c == EndOfFile)
                {
                    state_ = TS_FINISHED;
                }
//------------------------------------------------------------------------------
                else
                {
                    int spaceUntilEof = 1;
                    while (source_[pos_ + spaceUntilEof] != EndOfFile
                           && spaceUntilEof < 4)
                    {
                        spaceUntilEof++;
                    }

                    bool puncFound = false;

                    for (int i = spaceUntilEof; i > 0; i--)
                    {
                        string s = UTF8CodePointToString(source_, pos_, pos_ + i - 1);
                        if (Punctuation[i - 1].count(s) > 0)
                        {
                            output_.emit_preprocessing_op_or_punc(s);
                            pos_ += i;
                            puncFound = true;
                            break;
                        }
                    }

                    if (puncFound)
                    {
                        break;
                    }

                    output_.emit_non_whitespace_char(UTF8CodePointToString(c));
                    pos_++;
                }
                break;
////////////////////////////////////////////////////////////////////////////////
            case TS_COMMENT:
//------------------------------------------------------------------------------
                if (c == '\n')
                {
                    output_.emit_whitespace_sequence();
                    output_.emit_new_line();
                    pos_++;
                    state_ = TS_START;
                }
//------------------------------------------------------------------------------
                else if (c == EndOfFile)
                {
                    output_.emit_whitespace_sequence();
                    state_ = TS_FINISHED;
                }
//------------------------------------------------------------------------------
                else
                {
                    pos_++;
                }
                break;
////////////////////////////////////////////////////////////////////////////////
            case TS_INLINE_COMMENT:
//------------------------------------------------------------------------------
                if (MatchPrefix("*/", where))
                {
                    output_.emit_whitespace_sequence();
                    pos_ += 2;
                    state_ = TS_START;
                }
//------------------------------------------------------------------------------
                else if (c == EndOfFile)
                {
                    throw logic_error("Partial inline comment.");
                }
//------------------------------------------------------------------------------
                else
                {
                    pos_++;
                }
                break;
////////////////////////////////////////////////////////////////////////////////
            case TS_WHITESPACE:
//------------------------------------------------------------------------------
                if (c == ' '
                        || c == '\t')
                {
                    pos_++;
                }
//------------------------------------------------------------------------------
                else if (MatchPrefix("/*", where)
                         || MatchPrefix("//", where))
                {
                    state_ = TS_START;
                }
//------------------------------------------------------------------------------
                else if (c == '\n')
                {
                    output_.emit_whitespace_sequence();
                    output_.emit_new_line();
                    pos_++;
                    state_ = TS_START;
                }
//------------------------------------------------------------------------------
                else
                {
                    output_.emit_whitespace_sequence();
                    state_ = TS_START;
                    if (c == EndOfFile)
                    {
                        state_ = TS_FINISHED;
                    }
                }
                break;
////////////////////////////////////////////////////////////////////////////////
            case TS_MATCHING_IDENTIFIER:
//------------------------------------------------------------------------------
                if (IsDigit(c)
                        || IsNonDigit(c))
                {
                    pos_++;
                }
//------------------------------------------------------------------------------
                else
                {
                    string identifier = UTF8CodePointToString(source_, outStart_, pos_ - 1);
                    output_.emit_identifier(identifier);
                    state_ = TS_START;
                    if (c == EndOfFile)
                    {
                        state_ = TS_FINISHED;
                    }
                }
                break;
////////////////////////////////////////////////////////////////////////////////
            case TS_MATCHING_CHARACTER_LITERAL:
//------------------------------------------------------------------------------
                // ignoring escape sequences and stuff for now
                if (c == '\'')
                {
                    output_.emit_character_literal(UTF8CodePointToString(source_, outStart_, pos_));
                    state_ = TS_START;
                    pos_++;
                }
//------------------------------------------------------------------------------
                else if (c == '\n')
                {
                    throw logic_error("Newline in character literal");
                }
//------------------------------------------------------------------------------
                else if (c == '\\')
                {
                    int escapeSequenceMatched = MatchEscapeSequence(where + 1);
                    if (escapeSequenceMatched == 0)
                    {
                        throw logic_error("Invalid escape sequence.");
                    }
                    else
                    {
                        pos_ += escapeSequenceMatched + 1;
                    }
                }
//------------------------------------------------------------------------------
                else
                {
                    if (c == EndOfFile)
                    {
                        state_ = TS_FINISHED;
                    }
                    else
                    {
                        pos_++;
                    }
                }
                break;
////////////////////////////////////////////////////////////////////////////////
            case TS_MATCHING_PP_NUMBER:
            {
                int matched = MatchPrefixes("e-|e+|E-|E+", where);
//------------------------------------------------------------------------------
                if (IsDigit(c)
                        || IsNonDigit(c)
                        || matched > 0
                        || c == '.')
                {
                    matched = matched == 0 ? 1 : matched;
                    pos_ += matched;
                }
//------------------------------------------------------------------------------
                else
                {
                    output_.emit_pp_number(UTF8CodePointToString(source_, outStart_, pos_ - 1));
                    if (c == EndOfFile)
                    {
                        state_ = TS_FINISHED;
                    }
                    else
                    {
                        state_ = TS_START;
                    }
                }
            }
            break;
////////////////////////////////////////////////////////////////////////////////
            case TS_MATCHING_S_CHAR_SEQUENCE:
//------------------------------------------------------------------------------
                if (c == '"')
                {
                    output_.emit_string_literal(UTF8CodePointToString(source_, outStart_, pos_));
                    state_ = TS_START;
                    pos_++;
                }
                else if (c == '\n')
                {
                    throw logic_error("Newline in string literal");
                }
                else if (c == '\\')
                {
                    int escapeSequenceMatched = MatchEscapeSequence(where + 1);
                    if (escapeSequenceMatched == 0)
                    {
                        throw logic_error("Invalid escape sequence.");
                    }
                    else
                    {
                        pos_ += escapeSequenceMatched + 1;
                    }
                }
                else if (c == EndOfFile)
                {
                    throw logic_error("Unfinished string literal");
                }
                else
                {
                    pos_++;
                }
                break;
            }
        }

        if (sourceSize_ > 1)
        {
            if (sourceSize_ > 2)
            {
                if (*(source_ + pos_ - 2) == '\\')
                {
                    if (*(source_ + pos_ - 1) == '\n')
                    {
                        output_.emit_new_line();
                    }
                }
                if (*(source_ + pos_ - 1) != '\n')
                {
                    output_.emit_new_line();
                }
            }
            else
            {
                if (*(source_ + pos_ - 1) != '\n')
                {
                    output_.emit_new_line();
                }
            }
        }

        output_.emit_eof();
    }

} // namespace Compiler
