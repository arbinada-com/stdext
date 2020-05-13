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

void ioutils::set_imbue(std::wios* stream, const file_encoding enc, const char* locale_name)
{
    switch (enc)
    {
    case file_encoding::ansi:
        if (locale_name != nullptr)
            stream->imbue(std::locale(locale_name));
        break;
    case file_encoding::utf8:
        stream->imbue(std::locale(stream->getloc(), new std::codecvt_utf8<wchar_t, 0x10ffffUL, std::consume_header>));
        break;
    case file_encoding::utf16:
        stream->imbue(std::locale(stream->getloc(), new std::codecvt_utf16<wchar_t, 0x10ffffUL, std::consume_header>));
        break;
    }
}

/*
 * test_reader class
 */
text_reader::text_reader(std::wistream& stream)
    : text_reader(&stream, L"")
{ }

text_reader::text_reader(std::wistream& stream, const std::wstring source_name)
    : text_reader(&stream, source_name)
{ }

text_reader::text_reader(std::wistream* const stream)
    : text_reader(stream, L"")
{ }

text_reader::text_reader(std::wistream* const stream, const std::wstring source_name)
    : m_stream(stream), m_source_name(source_name)
{ }

text_reader::text_reader(const std::wstring file_name, const file_encoding enc, const char* locale_name)
    :m_source_name(file_name)
{
    m_owns_stream = true;
    m_stream = new wifstream(file_name, std::ios::binary);
    set_imbue(m_stream, enc, locale_name);
}

text_reader::~text_reader()
{
    if (m_owns_stream && m_stream != nullptr)
        delete m_stream;
}

bool ioutils::text_reader::next_char(wchar_t& c)
{
    c = m_stream->get();
    return m_stream->good();
}

bool text_reader::is_next_char(std::initializer_list<wchar_t> chars) const
{
    for (wchar_t c : chars)
    {
        if (m_stream->peek() == c)
            return true;
    }
    return false;
}


/*
 * test_writer class
 */
text_writer::text_writer(std::wostream& stream)
    : text_writer(&stream)
{ }

text_writer::text_writer(std::wostream* const stream)
    : m_stream(stream)
{ }

text_writer::text_writer(const std::wstring file_name, const file_encoding enc, const char* locale_name)
{
    m_owns_stream = true;
    m_stream = new wofstream(file_name, ios::binary | ios::trunc);
    set_imbue(m_stream, enc, locale_name);
}

text_writer::~text_writer()
{
    if (m_owns_stream && m_stream != nullptr)
        delete m_stream;
}

text_writer& text_writer::operator <<(const std::wstring s) 
{
    *m_stream << s.c_str(); 
    return *this; 
}
