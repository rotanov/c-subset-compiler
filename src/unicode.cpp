#include "unicode.hpp"

int UTF8Encode(int codePoint, char* out)
{
    if (codePoint <= 0x7F)
    {
        out[0] = codePoint;
        return 1;
    }
    else if (codePoint <= 0x7FF)
    {
        out[0] = ((codePoint & 0x7C0) >> 6) | 0xC0;
        out[1] = (codePoint & 0x3F) | 0x80;
        return 2;
    }
    else if (codePoint <= 0xFFFF)
    {
        out[0] = ((codePoint & 0xF000) >> 12) | 0xE0;
        out[1] = ((codePoint & 0xFC0) >> 6) | 0x80;
        out[2] = (codePoint & 0x3F) | 0x80;
        return 3;
    }
    else if (codePoint <= 0x10FFFF)
    {
        out[0] = ((codePoint & 0x1C0000) >> 18) | 0xF0;
        out[1] = ((codePoint & 0x3F000) >> 12) | 0x80;
        out[2] = ((codePoint & 0xFC0) >> 6) | 0x80;
        out[3] = (codePoint & 0x3F) | 0x80;
        return 4;
    }
    throw std::runtime_error("invalid UTF8 code point");
}

int UTF8Decode(int* in, int& out)
{
    out = 0;

    // FSA
    // TODO: handle invalid UTF-8 sequences e.g. overlongs, surrogates
    static unsigned char table[] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 000-015
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 016-031
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 032-047
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 048-063
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 064-079
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 080-095
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 096-111
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 112-127  // first 128 of ASCII ends here

        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 128-143
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 144-159
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 161-175
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 176-191  // 10xx xxxx

        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 192-227
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 208-223  // 110x xxxx

        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 224-239  // 1110 xxxx

        4, 4, 4, 4, 4, 4, 4, 4, // 240-247  // 1111 0xxx
        8, 8, 8, 8, 8, 8, 8, 8, // 248-255  // invalid

    };

    if (table[in[0]] == 0)
    {
        out = in[0];
        return 1;
    }
    else if (table[in[0]] == 2)
    {
        if (in[1] == -1)
        {
            throw std::runtime_error("invalid UTF8 octet sequence");
        }

        out |= (in[0] ^ 0xC0) << 6;
        out |= (in[1] ^ 0x80);
        return 2;
    }
    else if (table[in[0]] == 3)
    {
        if (in[1] == -1 || in[2] == -1)
        {
            throw std::runtime_error("invalid UTF8 octet sequence");
        }

        out |= (in[0] ^ 0xE0) << 12;
        out |= (in[1] ^ 0x80) << 6;
        out |= (in[2] ^ 0x80);
        return 3;
    }
    else if (table[in[0]] == 4)
    {
        if (in[1] == -1 || in[2] == -1 || in[3] == -1)
        {
            throw std::runtime_error("invalid UTF8 octet sequence");
        }

        out |= (in[0] ^ 0xF0) << 18;
        out |= (in[1] ^ 0x80) << 12;
        out |= (in[2] ^ 0x80) << 6;
        out |= (in[3] ^ 0x80);
        return 4;
    }

    throw std::runtime_error("invalid UTF8 octet sequence");
}

int UTF16Encode(int codePoint, char16_t* out)
{
    if (codePoint < 0x10000)
    {
        out[0] = codePoint;
        return 1;
    }
    else if (0x10000 <= codePoint && codePoint <= 0x10FFFF)
    {
        out[0] = 0xD800;
        out[1] = 0xDC00;
        codePoint -= 0x10000;
        out[0] |= codePoint >> 10;
        out[1] |= codePoint & 0x3FF;
        return 2;
    }
    else
    {
        throw std::runtime_error("unicode codepoint doesn't fit UTF16 range");
    }
}

int UTF16Decode(int* /*in*/, int& /*out*/)
{
    throw std::runtime_error("not implemented");
}

int UTF32Encode(int codePoint, char32_t* out)
{
    out[0] = codePoint;
    return 1;
}

int UTF32Decode(int* in, int& out)
{
    out = in[0];
    return 1;
}
