/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */
#include "ioutils.h"
#include "strutils.h"
#include <fstream>
#include <sstream>
#include <clocale>

using namespace std;
using namespace stdext;
using namespace ioutils;

/*
 * text_io_options_ansi class
 */
void text_io_options_ansi::set_imbue_read(std::wistream& stream) const
{
#if defined(__STDEXT_USE_ICONV)
    stream.imbue(std::locale(std::locale(""), new locutils::codecvt_ansi_utf16_wchar_t(m_cvt_mode)));
#elif defined(__STDEXT_WINDOWS)
    if (m_cvt_mode.is_encoding_name_assigned())
        stream.imbue(std::locale(m_cvt_mode.encoding_name()));
#endif
}

void text_io_options_ansi::set_imbue_write(std::wostream& stream) const
{
#if defined(__STDEXT_USE_ICONV)
    stream.imbue(std::locale(std::locale(""), new locutils::codecvt_ansi_utf16_wchar_t(m_cvt_mode)));
#elif defined(__STDEXT_WINDOWS)
    if (m_cvt_mode.is_encoding_name_assigned())
        stream.imbue(std::locale(m_cvt_mode.encoding_name()));
#endif
}

/*
 * text_io_options_utf8 class
 */
void text_io_options_utf8::set_imbue_read(std::wistream& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_utf8_wchar_t(m_cvt_mode)));
}

void text_io_options_utf8::set_imbue_write(std::wostream& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_utf8_wchar_t(m_cvt_mode)));
}

/*
 * text_io_options_utf16 class
 */
void text_io_options_utf16::set_imbue_read(std::wistream& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_utf16_wchar_t(m_cvt_mode)));
}

void text_io_options_utf16::set_imbue_write(std::wostream& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_utf16_wchar_t(m_cvt_mode)));
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

text_reader::text_reader(const std::wstring& file_name, const text_io_options& options)
    : m_source_name(file_name)
{
    m_owns_stream = true;
#if defined(__STDEXT_WINDOWS)
    m_stream = new wifstream(file_name, std::ios::binary);
#else
    m_stream = new wifstream(strutils::to_utf8string(file_name), std::ios::binary);
#endif
    options.set_imbue_read(*m_stream);
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
        if (c == (wchar_t)(m_stream->peek()))
            return true;
    }
    return false;
}

void text_reader::read_all(std::wstring& s)
{
//    wstringstream wss;
//    wss << m_stream->rdbuf();
//    s = wss.str();
    s.clear();
    wchar_t c;
    while (next_char(c))
        s += c;
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

text_writer::text_writer(const std::wstring file_name, const text_io_options& options)
{
    m_owns_stream = true;
#if defined(__STDEXT_WINDOWS)
    m_stream = new wofstream(file_name, ios::binary | ios::trunc);
#else
    m_stream = new wofstream(strutils::to_utf8string(file_name), ios::binary | ios::trunc);
#endif
    options.set_imbue_write(*m_stream);
}

text_writer::~text_writer()
{
    if (m_owns_stream && m_stream != nullptr)
        delete m_stream;
}

text_writer& text_writer::write(const std::wstring& s)
{
    return write(s.c_str());
}

text_writer& text_writer::write(const wchar_t* s)
{
    *m_stream << s;
    return *this; 
}

text_writer& text_writer::write_endl()
{
    *m_stream << endl;
    return *this;
}

