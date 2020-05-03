/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */
#include "ioutils.h"
#include <fstream>
#include <locale>
#include <codecvt>

using namespace std;
using namespace stdext;
using namespace ioutils;

text_reader::text_reader(stream_t& stream)
    : m_stream(&stream)
{ }

text_reader::text_reader(stream_t* const stream)
    : m_stream(stream)
{ }

text_reader::text_reader(const std::wstring file_name, const file_encoding enc, const char* locale_name)
{
    m_owns_stream = true;
    m_stream = new wifstream(file_name, std::ios::binary);
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

text_reader::~text_reader()
{
    if (m_owns_stream && m_stream != nullptr)
        delete m_stream;
}

bool ioutils::text_reader::next_char(char_type& c)
{
    c = m_stream->get();
    return m_stream->good();
}

bool text_reader::is_next_char(std::initializer_list<char_type> chars) const
{
    for (char_type c : chars)
    {
        if (m_stream->peek() == c)
            return true;
    }
    return false;
}
