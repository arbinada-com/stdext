/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */
#include "csvtools.h"

#include <fstream>
#include <locale>
#include <codecvt>

using namespace std;
using namespace stdext;

/**
 * CSV parser and reader
 */
csv::reader::reader(stream_t& stream)
    : m_stream(&stream)
{ }

csv::reader::reader(stream_t* const stream)
    : m_stream(stream)
{ }

csv::reader::reader(const std::wstring filename, const file_encoding enc, const char* locale_name)
{
    m_owns_stream = true;
    m_stream = new wifstream(filename, std::ios::binary);
    switch (enc)
    {
    case file_encoding::ansi:
        if (locale_name != nullptr)
            m_stream->imbue(std::locale(locale_name));
        break;
    case file_encoding::utf8:
        m_stream->imbue(std::locale(m_stream->getloc(), new std::codecvt_utf8<wchar_t, 0x10ffffUL, std::consume_header>));
        break;
    case file_encoding::utf16:
        m_stream->imbue(std::locale(m_stream->getloc(), new std::codecvt_utf16<wchar_t, 0x10ffffUL, std::consume_header>));
        break;
    }
}

csv::reader::~reader()
{
    if (m_owns_stream && m_stream != nullptr)
        delete m_stream;
}


bool csv::reader::next_row(csv::row& r)
{
    r.clear();
    if (eof())
        return false;
    bool row_accepted = false;
    bool value_accepted = false;
    bool accepting_quoted_value = false;
    std::wstring value;
    m_col_num = 0;
    while (!eof() && !row_accepted)
    {
        wchar_t c = next_char();
        if (!good())
        {
            if (!eof())
                m_error = reader_error::IOError;
            return false;
        }
        switch (c)
        {
        case '"':
            if (!accepting_quoted_value)
                accepting_quoted_value = true;
            else
            {
                if (is_next_char(c))
                    value += next_char();
                else
                {
                    accepting_quoted_value = false;
                    if (!eof() && !is_next_char({ m_separator, L'\r', L'\n' }))
                    {
                        m_error = reader_error::expectedSeparator;
                        return false;
                    }
                }
            }
            break;
        case L'\r':
        case L'\n':
            if (accepting_quoted_value)
                value += c;
            else
            {
                value_accepted = true;
                row_accepted = true;
                m_line_num++;
                m_col_num = 0;
                if (c == L'\r' && is_next_char(L'\n'))
                    next_char();
            }
            break;
        default:
            if (c != m_separator || accepting_quoted_value)
                value += c;
            else
                value_accepted = true;
            break;
        }
        if (value_accepted || eof())
        {
            value_accepted = false;
            r.append_value(value);
            value.clear();
        }
    }
    m_row_num++;
    if (has_header() && m_header.field_count() != r.field_count())
    {
        m_error = reader_error::rowFieldCountIsDifferentFromHeader;
        return false;
    }
    return true;
}

bool csv::reader::read_header()
{
    bool result = next_row(m_header);
    if (result)
        m_row_num--;
    return result;
}

wchar_t csv::reader::next_char()
{
    wchar_t c = m_stream->get();
    if (m_stream->good())
        m_col_num++;
    return c;
}

bool csv::reader::is_next_char(std::initializer_list<wchar_t> chars) const
{
    for (wchar_t c : chars)
    {
        if (m_stream->peek() == c)
            return true;
    }
    return false;
}
