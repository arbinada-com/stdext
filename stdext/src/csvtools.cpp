/*
 C++ standard library extensions
 (c) 2001-2021 Serguei Tarassov (see license.txt)
 */
#include "csvtools.h"

using namespace std;

namespace stdext::csv
{

std::wstring to_wstring(const reader_msg_kind value)
{
    switch(value)
    {
    case reader_msg_kind::io_error:
        return L"IO error";
    case reader_msg_kind::expected_separator:
        return L"Separator expected";
    case reader_msg_kind::row_field_count_different_from_header:
        return L"Row field count is defferent from header column count";
    default:
        return L"Unknown";
    }
}

/*
 * CSV parser and reader
 */
reader::reader(ioutils::text_reader* const rd)
    : m_reader(rd), m_owns_reader(false)
{}

reader::reader(std::wistream& stream)
    : m_reader(new ioutils::text_reader(stream)), m_owns_reader(true)
{}

reader::reader(std::wistream& stream, const ioutils::text_io_policy& policy)
    : m_reader(new ioutils::text_reader(stream, policy)), m_owns_reader(true)
{}

reader::reader(const wstring& file_name, const ioutils::text_io_policy& policy)
    : m_reader(new ioutils::text_reader(file_name, policy)), m_owns_reader(true)
{}

reader::reader(std::wifstream& stream, const ioutils::text_io_policy& policy)
    : m_reader(new ioutils::text_reader(stream, policy)), m_owns_reader(true)
{}

reader::~reader()
{
    if (m_owns_reader && m_reader != nullptr)
        delete m_reader;
}

void reader::add_error(const reader_msg_kind kind, const wstring& text)
{
    add_error(kind, m_pos, text);
}

void reader::add_error(const reader_msg_kind kind, parsers::textpos pos, const std::wstring& text)
{
    m_messages.add_error(parsers::msg_origin::lexer, kind, pos, m_reader->source_name(), text);
}

bool reader::next_char(wchar_t& wc)
{
    if (m_reader->next_char(wc))
    {
        ++m_pos;
        return true;
    }
    return false;
}

bool reader::next_row(row& r)
{
    r.clear();
    if (m_reader->eof())
        return false;
    bool row_accepted = false;
    bool value_accepted = false;
    bool accepting_quoted_value = false;
    std::wstring value;
    parsers::textpos lastpos; // last position before accepting row
    while (!m_reader->eof() && !row_accepted)
    {
        lastpos = m_pos;
        wchar_t c = 0;
        if (!next_char(c))
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
                {
                    next_char(c);
                    value += c;
                }
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
                next_char(c);
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

bool reader::read_header()
{
    bool result = next_row(m_header);
    if (result)
        m_row_num--;
    return result;
}

}
