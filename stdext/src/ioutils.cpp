/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */
#include "ioutils.h"
#include "platforms.h"
#include "strutils.h"
#include <fstream>
#include <sstream>
#include <clocale>

using namespace std;
using namespace stdext;
using namespace ioutils;
using namespace locutils;


/*
 * text_io_policy class
 */

bool text_io_policy::read_string(std::wistream& stream, std::string& buf, const size_t max_len) const
{
    buf.clear();
    buf.reserve(max_len);
    while (!stream.eof() && buf.length() <= max_len)
        buf += (char)stream.get();
    return (buf.length() > 0);
}

void text_io_policy::push_back_chars(const std::wstring& ws, text_buffer_t& buf) const
{
    for (const wchar_t& wc : ws)
        buf.push_back(wc);
}


/*
 * text_io_policy_plain class
 */
void text_io_policy_plain::do_read_chars(mbstate_t&, std::wistream& stream, text_buffer_t& buf) const
{
    if (!buf.empty())
        return;
    while (!stream.eof() && buf.size() <= m_max_buf_size)
        buf.push_back((wchar_t)stream.get());
}

/*
 * text_io_policy_ansi class
 */
text_io_policy_ansi::text_io_policy_ansi()
    : text_io_policy_ansi(locutils::codecvt_mode_ansi())
{}

text_io_policy_ansi::text_io_policy_ansi(const locutils::ansi_encoding encoding)
    : text_io_policy_ansi(locutils::codecvt_mode_ansi(encoding))
{}

text_io_policy_ansi::text_io_policy_ansi(const char* encoding_name)
    : text_io_policy_ansi(locutils::codecvt_mode_ansi(encoding_name))
{}

text_io_policy_ansi::text_io_policy_ansi(const locutils::codecvt_mode_ansi& cvt_mode)
    : base_t(),
      m_cvt_mode(cvt_mode)
{
    m_cvt = new locutils::codecvt_ansi_utf16_wchar_t(m_cvt_mode);
}

text_io_policy_ansi::~text_io_policy_ansi()
{
    if (m_cvt != nullptr)
        delete m_cvt;
}

void text_io_policy_ansi::set_imbue_read(std::wistream& stream) const
{
#if defined(__STDEXT_USE_ICONV)
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_ansi_utf16_wchar_t(m_cvt_mode)));
#elif defined(__STDEXT_WINDOWS)
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_ansi_utf16_wchar_t(m_cvt_mode)));
//    if (!use_default_encoding())
//        stream.imbue(std::locale(m_cvt_mode.encoding_name_windows()));
#endif
}

void text_io_policy_ansi::set_imbue_write(std::wostream& stream) const
{
#if defined(__STDEXT_USE_ICONV)
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_ansi_utf16_wchar_t(m_cvt_mode)));
#elif defined(__STDEXT_WINDOWS)
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_ansi_utf16_wchar_t(m_cvt_mode)));
//    if (!use_default_encoding())
//        stream.imbue(std::locale(m_cvt_mode.encoding_name_windows()));
#endif
}

void text_io_policy_ansi::do_read_chars(mbstate_t&, std::wistream& stream, text_buffer_t& buf) const
{
    string ansi;
    if (read_string(stream, ansi, 1024))
    {
        wstring ws;
        if (m_cvt->ansi_to_utf16(ansi, ws) == m_cvt->ok) // TODO use m_mbstate
            push_back_chars(ws, buf);
    }
}

/*
 * text_io_policy_utf8 class
 */
text_io_policy_utf8::text_io_policy_utf8()
    : text_io_policy_utf8(locutils::codecvt_mode_utf8())
{}

text_io_policy_utf8::text_io_policy_utf8(const locutils::codecvt_mode_utf8& cvt_mode)
    : base_t(), m_cvt_mode(cvt_mode)
{
    m_cvt = new locutils::codecvt_utf8_wchar_t(m_cvt_mode);
}

text_io_policy_utf8::~text_io_policy_utf8()
{
    if (m_cvt != nullptr)
        delete m_cvt;
}

void text_io_policy_utf8::set_imbue_read(std::wistream& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_utf8_wchar_t(m_cvt_mode)));
}

void text_io_policy_utf8::set_imbue_write(std::wostream& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_utf8_wchar_t(m_cvt_mode)));
}

void text_io_policy_utf8::do_read_chars(mbstate_t&, std::wistream& stream, text_buffer_t& buf) const
{
    string u8s;
    if (read_string(stream, u8s, 1024)) //TO DO loop until m_cvt->ok
    {
        wstring ws;
        if (m_cvt->to_utf16(u8s, ws) == m_cvt->ok)
            push_back_chars(ws, buf);
    }
}


/*
 * text_io_policy_utf16 class
 */

text_io_policy_utf16::text_io_policy_utf16()
    : text_io_policy_utf16(locutils::codecvt_mode_utf16())
{}

text_io_policy_utf16::text_io_policy_utf16(const locutils::codecvt_mode_utf16& cvt_mode)
    : base_t(), m_cvt_mode(cvt_mode)
{
    m_cvt = new locutils::codecvt_utf16_wchar_t(m_cvt_mode);
}

text_io_policy_utf16::~text_io_policy_utf16()
{
    if (m_cvt != nullptr)
        delete m_cvt;
}

void text_io_policy_utf16::set_imbue_read(std::wistream& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_utf16_wchar_t(m_cvt_mode)));
}

void text_io_policy_utf16::set_imbue_write(std::wostream& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_utf16_wchar_t(m_cvt_mode)));
}

void text_io_policy_utf16::do_read_chars(mbstate_t&, std::wistream& stream, text_buffer_t& buf) const
{
    string mbs;
    if (read_string(stream, mbs, 1024))
    {
        wstring ws;
        if (m_cvt->mb_to_utf16(mbs, ws) == m_cvt->ok)
            push_back_chars(ws, buf);
    }
}


/*
 * text_reader class
 */
text_reader::text_reader(std::wistream& stream)
    : m_stream(&stream)
{}

text_reader::text_reader(std::wistream& stream, const text_io_policy& policy)
    : m_stream(&stream),
      m_policy(policy)
{}

text_reader::text_reader(const std::wstring& file_name, const text_io_policy& policy)
    : m_source_name(file_name),
      m_policy(policy)
{
    m_use_file_io = true;
    m_owns_stream = true;
#if defined(__STDEXT_WINDOWS)
    m_stream = new wifstream(file_name, std::ios::binary);
#else
    m_wstream = new wifstream(utf16::to_utf8string(file_name), std::ios::binary);
#endif
    m_policy.set_imbue_read(*m_stream);
}

text_reader::text_reader(std::wifstream& stream, const text_io_policy& policy)
    : m_stream(&stream),
      m_policy(policy)
{
    m_use_file_io = true;
    // stream should be open in binary mode but there is probably no way to check it
    m_policy.set_imbue_read(*m_stream);
}

text_reader::~text_reader()
{
    if (m_owns_stream && m_stream != nullptr)
        delete m_stream;
}

wchar_t text_reader::next_char()
{
    wchar_t wc = 0;
    if (m_chars.empty())
        read_chars();
    if (!m_chars.empty())
    {
        wc = m_chars.front();
        m_chars.pop_front();
    }
    return wc;
}

bool text_reader::next_char(wchar_t& c)
{
    c = next_char();
    return good();
}

void text_reader::read_chars()
{
    if (m_stream == nullptr)
        return;
    if (m_use_file_io) // imbue() codecvt works only with file streams
    {
        if (!m_stream->eof() && m_stream->good())
            m_chars.push_back((wchar_t)m_stream->get());
    }
    else
        m_policy.do_read_chars(m_mbstate, *m_stream, m_chars);
}

wchar_t text_reader::peek()
{
    if (m_chars.empty())
    {
        if (m_stream != nullptr)
            m_stream->peek(); // Set flags before
        read_chars();
    }
    if (!m_chars.empty())
        return m_chars.front();
    return 0;
}

bool text_reader::is_next_char(wchar_t c)
{
    return c == this->peek();
}

bool text_reader::is_next_char(std::initializer_list<wchar_t> chars)
{
    for (wchar_t c : chars)
    {
        if (c == this->peek())
            return true;
    }
    return false;
}

void text_reader::read_all(std::wstring& ws)
{
    wchar_t c;
    while (next_char(c))
        ws += c;
}

std::streamsize text_reader::count() const
{
    if (m_stream != nullptr)
        return m_stream->gcount();
    return -1;
}

bool text_reader::eof() const
{
    if (!m_chars.empty())
        return false;
    if (m_stream != nullptr)
        return m_stream->eof();
    return true;
}

bool text_reader::good() const
{
    if (!m_chars.empty())
        return true;
    if (m_stream != nullptr)
        return m_stream->good();
    return false;
}

int text_reader::rdstate() const
{
    if (m_stream != nullptr)
        return m_stream->rdstate();
    return ios_base::failbit;
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

text_writer::text_writer(const std::wstring file_name, const text_io_policy& policy)
{
    m_owns_stream = true;
#if defined(__STDEXT_WINDOWS)
    m_stream = new wofstream(file_name, ios::binary | ios::trunc);
#else
    m_stream = new wofstream(utf16::to_utf8string(file_name), ios::binary | ios::trunc);
#endif
    policy.set_imbue_write(*m_stream);
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

