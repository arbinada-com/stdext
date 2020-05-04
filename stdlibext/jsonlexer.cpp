/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "jsonlexer.h"

using namespace std;
using namespace stdext;

bool json::lexer::next_lexeme(lexeme& lex)
{
    return false;
}

bool json::lexer::has_error()
{
    return false;
}

bool json::lexer::eof()
{
    return m_reader == nullptr || m_reader->eof();
}

bool json::lexer::is_whitespace(const wchar_t c)
{
    return c == L' ' || c == L'\t';
}

bool json::lexer::next_char()
{
    wchar_t prev = m_c;
    if (m_reader->next_char(m_c))
    {
        if (m_c == L'\n')
            m_pos.newline();
        else if (prev != L'\n' && prev != L'\r')
            m_pos++;
        return true;
    }
    else
    {
        m_c = 0;
        if (m_reader->eof())
            ; // I/O error
    }
    return false;
}

void json::lexer::skip_whitespaces()
{
    while (!eof() && is_whitespace(m_c))
        next_char();
}
