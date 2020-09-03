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
#include <algorithm>

using namespace std;
using namespace stdext;
using namespace ioutils;
using namespace locutils;


/*
 * text_io_policy class
 */
void text_io_policy::push_back_chars(const std::wstring& ws, text_buffer_t& buf) const
{
    for (const wchar_t& wc : ws)
        buf.push_back(wc);
}


/*
 * text_io_policy_plain class
 */
void text_io_policy_plain::read_chars(mbstate_t&, text_reader_stream_adapter_base& stream, text_buffer_t& buf) const
{
    if (!buf.empty())
        return;
    while (buf.size() < m_max_text_buf_size)
    {
        text_reader_stream_adapter_base::int_type wc = stream.get();
        if (!stream.good() || stream.is_eof(wc))
            break;
        buf.push_back((wchar_t)wc);
    }
}

void text_io_policy_plain::write_chars(mbstate_t&, text_writer_stream_adapter_base& stream, const std::wstring& ws) const
{
    for(const wchar_t& wc : ws)
        stream.put(wc);
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

void text_io_policy_ansi::set_imbue_read(text_reader_stream_adapter_base& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_ansi_utf16_wchar_t(m_cvt_mode)));
}

void text_io_policy_ansi::set_imbue_write(text_writer_stream_adapter_base& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_ansi_utf16_wchar_t(m_cvt_mode)));
}

void text_io_policy_ansi::read_chars(mbstate_t& state, text_reader_stream_adapter_base& stream, text_buffer_t& buf) const
{
    string ansi;
    if (read_bytes(stream, ansi, m_max_text_buf_size))
    {
        wstring ws;
        if (m_cvt->ansi_to_utf16(state, ansi, ws) == m_cvt->ok)
            push_back_chars(ws, buf);
    }
}

bool text_io_policy_ansi::read_bytes(text_reader_stream_adapter_base& stream, std::string& bytes, const size_t max_len) const
{
    bytes.clear();
    bytes.reserve(max_len);
    while (bytes.length() < max_len)
    {
        text_reader_stream_adapter_base::int_type wc = stream.get();
        if (!stream.good() || stream.is_eof(wc))
            break;
        bytes += (unsigned char)wc;
    }
    return (bytes.length() > 0);
}

void text_io_policy_ansi::write_chars(mbstate_t& state, text_writer_stream_adapter_base& stream, const std::wstring& ws) const
{
    string bytes;
    if (m_cvt->utf16_to_ansi(state, ws, bytes) == m_cvt->ok)
    {
        for(const char& c : bytes)
            stream.put((unsigned char)c);
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

void text_io_policy_utf8::set_imbue_read(text_reader_stream_adapter_base& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_utf8_wchar_t(m_cvt_mode)));
}

void text_io_policy_utf8::set_imbue_write(text_writer_stream_adapter_base& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_utf8_wchar_t(m_cvt_mode)));
}

void text_io_policy_utf8::read_chars(mbstate_t& state, text_reader_stream_adapter_base& stream, text_buffer_t& buf) const
{
    string bytes;
    if (read_bytes(stream, bytes, m_max_text_buf_size))  // min length of UTF-8 bytes is equal to UTF-16 string length (in characters)
    {
        codecvt_utf8_wchar_t::result_t result = m_cvt->error;
        do
        {
            wstring ws;
            size_t converted_bytes = 0;
            result = m_cvt->to_utf16(state, bytes, ws, converted_bytes);
            if (result == m_cvt->ok)
            {
                bytes.clear();
                push_back_chars(ws, buf);
            }
            else if (result == m_cvt->partial)
            {
                bytes.erase(0, converted_bytes);
                push_back_chars(ws, buf);
                if (!read_bytes(stream, bytes, 1))
                    break;
            }
            else
                break;
        } while (result != m_cvt->ok);
    }
}

bool text_io_policy_utf8::read_bytes(text_reader_stream_adapter_base& stream, std::string& bytes, const size_t count) const
{
    size_t i = 0;
    for (; i < count; i++)
    {
        text_reader_stream_adapter_base::int_type wc = stream.get();
        if (!stream.good() || stream.is_eof(wc))
            break;
        bytes += (unsigned char)wc;
    }
    return i > 0;
}

void text_io_policy_utf8::write_chars(mbstate_t& state, text_writer_stream_adapter_base& stream, const std::wstring& ws) const
{
    string bytes;
    if (m_cvt->to_utf8(state, ws, bytes) == m_cvt->ok)
    {
        for(const char& c : bytes)
            stream.put((unsigned char)c);
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

void text_io_policy_utf16::set_imbue_read(text_reader_stream_adapter_base& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_utf16_wchar_t(m_cvt_mode)));
}

void text_io_policy_utf16::set_imbue_write(text_writer_stream_adapter_base& stream) const
{
    stream.imbue(std::locale(stream.getloc(), new locutils::codecvt_utf16_wchar_t(m_cvt_mode)));
}

void text_io_policy_utf16::read_chars(mbstate_t& state, text_reader_stream_adapter_base& stream, text_buffer_t& buf) const
{
    string mbs;
    if (read_bytes(stream, mbs, m_max_text_buf_size * utf16::bytes_per_character))
    {
        wstring ws;
        if (m_cvt->mb_to_utf16(state, mbs, ws) == m_cvt->ok)
            push_back_chars(ws, buf);
    }
}

bool text_io_policy_utf16::read_bytes(text_reader_stream_adapter_base& stream, std::string& bytes, const size_t max_len) const
{
    bytes.clear();
    bytes.reserve(max_len);
    while (bytes.length() < max_len)
    {
        text_reader_stream_adapter_base::int_type wc = stream.get();
        if (!stream.good() || stream.is_eof(wc))
            break;
        bytes += (unsigned char)wc;
    }
    // Other way is copy_n() function but copy_if() is required too to check eof()
    // Note that 'streambuf' operations doesn't set stream state bits
    // std::istreambuf_iterator<wchar_t> it(stream);
    // std::copy_n(it, max_len, std::back_inserter(buf));
    return (bytes.length() > 0);
}

void text_io_policy_utf16::write_chars(mbstate_t& state, text_writer_stream_adapter_base& stream, const std::wstring& ws) const
{
    string bytes;
    if (m_cvt->utf16_to_mb(state, ws, bytes) == m_cvt->ok)
    {
        for(const char& c : bytes)
            stream.put((unsigned char)c);
    }
}



/*
 * text_reader class
 */
text_reader::text_reader(std::wistream& stream)
    : m_stream(new text_wistream_adapter(&stream, false))
{}

text_reader::text_reader(std::wistream& stream, const text_io_policy& policy)
    : m_stream(new text_wistream_adapter(&stream, false)),
      m_policy(policy)
{}

text_reader::text_reader(std::istream& stream)
    : m_stream(new text_istream_adapter(&stream, false))
{}

text_reader::text_reader(std::istream& stream, const text_io_policy& policy)
    : m_stream(new text_istream_adapter(&stream, false)),
      m_policy(policy)
{}

text_reader::text_reader(const std::wstring& file_name, const text_io_policy& policy)
    : m_source_name(file_name),
      m_policy(policy)
{
    m_use_file_io = true;
#if defined(__STDEXT_WINDOWS)
    m_stream = new text_wistream_adapter(new wifstream(file_name, std::ios::binary), true);
#else
    m_stream = new text_wistream_adapter(new wifstream(utf16::to_utf8string(file_name), std::ios::binary), true);
#endif
    m_policy.set_imbue_read(*m_stream);
}

text_reader::text_reader(std::wifstream& stream, const text_io_policy& policy)
    : m_stream(new text_wistream_adapter(&stream, false)),
      m_policy(policy)
{
    m_use_file_io = true;
    // stream should be open in binary mode but there is probably no way to check it
    m_policy.set_imbue_read(*m_stream);
}

text_reader::~text_reader()
{
    if (m_stream != nullptr)
        delete m_stream;
}

bool text_reader::next_char(wchar_t& wc)
{
    if (m_chars.empty())
        read_chars();
    if (!m_chars.empty())
    {
        wc = m_chars.front();
        m_chars.pop_front();
        return true;
    }
    return false;
}

void text_reader::read_chars()
{
    if (m_use_file_io) // imbue() codecvt works only with file streams
    {
        while (m_chars.size() < m_policy.max_text_buf_size())
        {
            text_reader_stream_adapter_base::int_type wc = m_stream->get();
            if (!m_stream->good() || m_stream->is_eof(wc))
                break;
            m_chars.push_back((wchar_t)wc);
        }
    }
    else
        m_policy.read_chars(m_mbstate, *m_stream, m_chars);
}

bool text_reader::peek(wchar_t& wc)
{
    if (m_chars.empty())
        read_chars();
    if (!m_chars.empty())
    {
        wc = m_chars.front();
        return true;
    }
    return false;
}

bool text_reader::is_next_char(wchar_t wc)
{
    wchar_t next;
    if (this->peek(next))
        return wc == next;
    return false;
}

bool text_reader::is_next_char(std::initializer_list<wchar_t> wchars)
{
    wchar_t next;
    if (this->peek(next))
    {
        for (wchar_t c : wchars)
        {
            if (c == next)
                return true;
        }
    }
    return false;
}

void text_reader::read_all(std::wstring& ws)
{
    wchar_t c;
    while (next_char(c))
        ws += c;
}

void text_reader::read_line(std::wstring& ws)
{
    wchar_t c;
    while (next_char(c))
    {
        ws += c;
        if (c == '\n')
            break;
    }
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
    {
        if (m_stream->gcount() == 0)
            m_stream->peek();
        return m_stream->eof();
    }
    return true;
}


/*
 * test_writer class
 */
text_writer::text_writer(std::wostream& stream)
    : m_stream(new text_wostream_adapter(&stream, false))
{}

text_writer::text_writer(std::wostream& stream, const text_io_policy& policy)
    : m_stream(new text_wostream_adapter(&stream, false)), m_policy(policy)
{}

text_writer::text_writer(std::ostream& stream)
    : m_stream(new text_ostream_adapter(&stream, false))
{}

text_writer::text_writer(std::ostream& stream, const text_io_policy& policy)
    : m_stream(new text_ostream_adapter(&stream, false)), m_policy(policy)
{}

text_writer::text_writer(const std::wstring file_name, const text_io_policy& policy)
    : m_policy(policy)
{
    m_use_file_io = true;
#if defined(__STDEXT_WINDOWS)
    m_stream = new text_wostream_adapter(new wofstream(file_name, ios::binary | ios::trunc), true);
#else
    m_stream = new text_wostream_adapter(new wofstream(utf16::to_utf8string(file_name), ios::binary | ios::trunc), true);
#endif
    policy.set_imbue_write(*m_stream);
}

text_writer::text_writer(std::wofstream& stream, const text_io_policy& policy)
    : m_stream(new text_wostream_adapter(&stream, false)), m_policy(policy)
{
    m_use_file_io = true;
    policy.set_imbue_write(*m_stream);
}

text_writer::~text_writer()
{
    if (m_stream != nullptr)
        delete m_stream;
}

text_writer& text_writer::write(const std::wstring& ws)
{
    if (m_use_file_io)
    {
        for (const wchar_t& wc : ws)
            m_stream->put(wc);
    }
    else
        m_policy.write_chars(m_mbstate, *m_stream, ws);
    return *this;
}

text_writer& text_writer::write(const wchar_t wc)
{
    if (m_use_file_io)
        m_stream->put(wc);
    else
        m_policy.write_chars(m_mbstate, *m_stream, wstring(1, wc));
    return *this;
}

text_writer& text_writer::write_endl()
{
    return write(L'\n');
}

