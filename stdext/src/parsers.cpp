/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "parsers.h"
#include "strutils.h"

using namespace std;

namespace stdext::parsers
{

std::wstring to_wstring(const msg_severity value)
{
    switch (value)
    {
    case msg_severity::error: return L"error";
    case msg_severity::hint: return L"hint";
    case msg_severity::info: return L"info";
    case msg_severity::warning: return L"warning";
    default:
        return L"unknown severity";
    }
}

/*
 * textpos class
 */
void textpos::operator ++() noexcept
{
    m_col++;
}
textpos& textpos::operator ++(pos_t) noexcept
{
    m_col++;
    return *this;
}

bool textpos::operator ==(const textpos& rhs) const noexcept
{
    return m_col == rhs.m_col && m_line == rhs.m_line;
}

void textpos::newline() noexcept
{
    m_line++;
    m_col = 1;
}

void textpos::reset() noexcept
{
    m_line = 1;
    m_col = 1;
}

std::string textpos::to_string() const
{
    return str::format("(%d,%d)", m_line, m_col);
}
std::wstring textpos::to_wstring() const
{
    return str::to_wstring(to_string());
}

/*
 * numeric_parser implementation
 */
numeric_parser::numeric_parser(const std::wstring& s)
{
    read_string(s);
}

void numeric_parser::accept_char(const wchar_t c)
{
    m_char_count++;
    m_value += c;
}

bool numeric_parser::read_string(const std::wstring& s)
{
    for (wchar_t c : s)
    {
        if (!read_char(c))
        {
            m_type = numeric_type::nt_unknown;
            break;
        }
    }
    return is_valid_number();
}

bool numeric_parser::read_char(const wchar_t c)
{
    switch (c)
    {
    case L'-':
        if (m_char_count == 0)
            accept_char(c);
        break;
    case L'+':
        if (m_accepting_type == numeric_type::nt_float && m_char_count == 0)
            accept_char(c);
        else
            return false;
        break;
    case L'.':
        if (m_accepting_type == numeric_type::nt_integer && m_digit_count > 0)
        {
            accept_char(c);
            m_accepting_type = numeric_type::nt_decimal;
            m_type = numeric_type::nt_unknown;
        }
        else
            return false;
        break;
    case L'0':
    case L'1':
    case L'2':
    case L'3':
    case L'4':
    case L'5':
    case L'6':
    case L'7':
    case L'8':
    case L'9':
        if (m_accepting_type == numeric_type::nt_integer && m_digit_count == 1 && m_value == L"0")
        {
            m_type = numeric_type::nt_unknown;
            return false;
        }
        accept_char(c);
        m_digit_count++;
        m_type = m_accepting_type;
        break;
    case L'e':
    case L'E':
        if (m_accepting_type == numeric_type::nt_integer || m_accepting_type == numeric_type::nt_decimal)
        {
            accept_char(c);
            m_char_count = 0;
            m_digit_count = 0;
            m_type = numeric_type::nt_unknown;
            m_accepting_type = numeric_type::nt_float;
        }
        else
        {
            m_type = numeric_type::nt_unknown;
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}


bool is_number(const std::wstring& s)
{
    numeric_parser parser(s);
    return parser.is_valid_number();
}

}


