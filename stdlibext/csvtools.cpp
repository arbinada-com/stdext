/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */
#include "csvtools.h"


using namespace std;
using namespace stdext;

/**
 * CSV parser and reader
 */

csv::reader::reader(ioutils::text_reader* const rd)
    : m_owns_reader(false), m_reader(rd)
{ }

csv::reader::reader(ioutils::text_reader::stream_t& stream)
    : m_owns_reader(true), m_reader(new ioutils::text_reader(stream))
{ }

csv::reader::reader(ioutils::text_reader::stream_t* const stream)
    : m_owns_reader(true), m_reader(new ioutils::text_reader(stream))
{ }

csv::reader::reader(const std::wstring file_name, const ioutils::file_encoding enc, const char* locale_name)
    : m_owns_reader(true), m_reader(new ioutils::text_reader(file_name, enc, locale_name))
{ }

csv::reader::~reader()
{
    if (m_owns_reader && m_reader != nullptr)
        delete m_reader;
}

ioutils::text_reader::char_type csv::reader::next_char()
{
    ioutils::text_reader::char_type c;
    if (m_reader->next_char(c))
        m_col_num++;
    return c;
}

bool csv::reader::next_row(csv::row& r)
{
    r.clear();
    if (m_reader->eof())
        return false;
    bool row_accepted = false;
    bool value_accepted = false;
    bool accepting_quoted_value = false;
    std::wstring value;
    m_col_num = 0;
    while (!m_reader->eof() && !row_accepted)
    {
        ioutils::text_reader::char_type c = next_char();
        if (!m_reader->good())
        {
            if (!m_reader->eof())
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
                if (m_reader->is_next_char(c))
                    value += next_char();
                else
                {
                    accepting_quoted_value = false;
                    if (!m_reader->eof() && !m_reader->is_next_char({ m_separator, L'\r', L'\n' }))
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
                if (c == L'\r' && m_reader->is_next_char(L'\n'))
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
        if (value_accepted || m_reader->eof())
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
