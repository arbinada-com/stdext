/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "parsers.h"
#include "strutils.h"

using namespace std;
using namespace stdext;
using namespace parsers;

std::wstring parsers::to_wstring(const msg_severity value)
{
    switch (value)
    {
    case msg_severity::error: return L"error";
    case msg_severity::hint: return L"hint";
    case msg_severity::none: return L"";
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
    return strutils::format("(%d,%d)", m_line, m_col);
}
std::wstring textpos::to_wstring() const 
{
    return strutils::to_wstring(to_string()); 
}
