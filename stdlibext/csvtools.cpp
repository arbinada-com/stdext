/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */
#include "csvtools.h"


using namespace std;
using namespace stdext;
using namespace parsers;

/**
 * CSV parser and reader
 */

csv::reader::reader(ioutils::text_reader* const rd)
    : m_owns_reader(false), m_reader(rd)
{ }

csv::reader::reader(std::wistream& stream)
    : m_owns_reader(true), m_reader(new ioutils::text_reader(stream))
{ }

csv::reader::reader(std::wistream* const stream)
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

void csv::reader::add_error(const reader_msg_kind kind, const std::wstring text)
{
    add_error(kind, m_pos, text);
}

void csv::reader::add_error(const reader_msg_kind kind, parsers::textpos pos, const std::wstring text)
{
    m_messages.add_error(msg_origin::lexer, kind, pos, m_reader->source_name(), text);
}

wchar_t csv::reader::next_char()
{
    wchar_t c;
    if (m_reader->next_char(c))
        m_pos++;
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
    textpos lastpos; // last position before accepting row
    while (!m_reader->eof() && !row_accepted)
    {
        lastpos = m_pos;
        wchar_t c = next_char();
        if (!m_reader->good())
        {
            if (!m_reader->eof())
                add_error(reader_msg_kind::io_error, L"I/O error");
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
                        add_error(reader_msg_kind::expected_separator, L"Separator expected");
                        return false;
                    }
                }
            }
            break;
        case L'\r':
        case L'\n':
        {
            wstring nl;
            nl += c;
            if (c == L'\r' && m_reader->is_next_char(L'\n'))
            {
                c = next_char();
                nl += c;
            }
            if (accepting_quoted_value)
                value += nl;
            else
            {
                value_accepted = true;
                row_accepted = true;
            }
            m_pos.newline();
            break;
        }
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
        add_error(reader_msg_kind::row_field_count_different_from_header, lastpos, L"Row field count is different from header one");
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
