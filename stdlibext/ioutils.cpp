/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */
#include "ioutils.h"
#include <fstream>
#include <sstream>
#include <locale>
#include <codecvt>

using namespace std;
using namespace stdext;
using namespace ioutils;

bool ioutils::is_high_surrogate(const wchar_t c)
{
    return c >= 0xD800 && c <= 0xDBFF;
}

bool ioutils::is_low_surrogate(const wchar_t c)
{
    return c >= 0xDC00 && c <= 0xDFFF;
}

bool ioutils::is_noncharacter(const wchar_t c)
{
    // Noncharacters are code points that are permanently reserved in the Unicode Standard forinternal use.
    // They are forbidden for use in open interchange of Unicode text data.
    // http://www.unicode.org/versions/Unicode5.2.0/ch16.pdf#G19635
    return
        (c >= 0xFDD0 && c <= 0xFDEF)
        || c == 0xFEFF // BOM BE
        || c == 0xFFFE // BOM LE
        || c == 0xFFFF
        ;
}

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

void text_reader::read_all(std::wstring& s)
{
    wstringstream wss;
    wss << m_stream->rdbuf();
    s = wss.str();
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
    switch (enc)
    {
    case file_encoding::ansi:
        if (locale_name != nullptr)
            m_stream->imbue(std::locale(locale_name));
        break;
    case file_encoding::utf8:
        //stream->imbue(std::locale(std::locale("C"), new std::codecvt_utf8<wchar_t, 0x10ffffUL, std::consume_header>));
        m_stream->imbue(std::locale(m_stream->getloc(), new std::codecvt_utf8<wchar_t, 0x10ffffUL, std::consume_header>));
        break;
    case file_encoding::utf16:
        m_stream->imbue(std::locale(std::locale::empty(), new std::codecvt_utf16<wchar_t, 0x10ffffUL, std::generate_header>));
        //m_stream->imbue(std::locale(m_stream->getloc(), new std::codecvt_utf16<wchar_t, 0x10ffffUL, std::generate_header>));
        break;
    }
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
