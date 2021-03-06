/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "locutils.h"
#include <algorithm>
#include <functional>
#include <cwctype>
#if defined(__STDEXT_USE_ICONV)
    #include <iconv.h>
#endif

using namespace std;

namespace stdext::locutils
{

/*
 * utf16 utils
 */
const char utf16::bom_le[] = { '\xFF', '\xFE' };
const char utf16::bom_be[] = { '\xFE', '\xFF' };
const uint16_t utf16::bom_value;
const uint16_t utf16::high_surrogate_min;
const uint16_t utf16::high_surrogate_max;
const uint16_t utf16::low_surrogate_min;
const uint16_t utf16::low_surrogate_max;
const char32_t utf16::max_char;
const wchar_t utf16::replacement_character;
const int utf16::bytes_per_character;

bool utf16::is_bom(wchar_t c)
{
    return (c == utf16::bom_value);
}

bool utf16::is_bom(char byte0, char byte1, endianess::byte_order order)
{
    if (order == endianess::byte_order::big_endian)
        return (byte0 == utf16::bom_be[0] && byte1 == utf16::bom_be[1]);
    return (byte0 == utf16::bom_le[0] && byte1 == utf16::bom_le[1]);
}

void utf16::add_bom(wstring& ws)
{
    if (ws.length() == 0 || !utf16::is_bom(ws[0]))
    {
        ws.insert(0, 1, utf16::bom_value);
    }
}

void utf16::add_bom(string& bytes, endianess::byte_order order)
{
    if (bytes.length() < 2 || !utf16::is_bom(bytes[0], bytes[1], order))
    {
        if (order == endianess::byte_order::big_endian)
            bytes.insert(0, utf16::bom_be, 2);
        else
            bytes.insert(0, utf16::bom_le, 2);
    }
}

bool utf16::is_high_surrogate(const wchar_t c)
{
    return c >= utf16::high_surrogate_min && c <= utf16::high_surrogate_max;
}

bool utf16::is_low_surrogate(const wchar_t c)
{
    return c >= utf16::low_surrogate_min && c <= utf16::low_surrogate_max;
}

bool utf16::is_noncharacter(const wchar_t c)
{
    // Noncharacters are code points that are permanently reserved in the Unicode Standard forinternal use.
    // They are forbidden for use in open interchange of Unicode text data.
    // http://www.unicode.org/versions/Unicode5.2.0/ch16.pdf#G19635
    return
        (c >= (wchar_t)0xFDD0u && c <= (wchar_t)0xFDEFu)
        || c == (wchar_t)utf16::bom_value
        || c == (wchar_t)0xFFFEu
        || c == (wchar_t)0xFFFFu
#if __STDEXT_WCHAR_SIZE > 2
        || c > (wchar_t)utf16::max_char
#endif
        ;
}

bool utf16::is_surrogate_pair(const char32_t c)
{
    return c > 0xFFFF && c <= max_char;
}

bool utf16::is_surrogate_pair(const wchar_t high, const wchar_t low)
{
    return utf16::is_high_surrogate(high) && utf16::is_low_surrogate(low);
}

bool utf16::to_surrogate_pair(const char32_t c, wchar_t& high, wchar_t& low)
{
    if (is_surrogate_pair(c))
    {
        high = (uint16_t)(((c - 0x10000) >> 10) + utf16::high_surrogate_min);
        low = (uint16_t)((c & 0x3FF) + utf16::low_surrogate_min);
        return true;
    }
    return false;
}

bool utf16::from_surrogate_pair(wchar_t high, wchar_t low, char32_t& c)
{
    if (is_high_surrogate(high) && is_low_surrogate(low))
    {
        c = (((char32_t)(high - utf16::high_surrogate_min)) << 10)
            + ((low - utf16::low_surrogate_min) + 0x10000);
        return true;
    }
    return false;
}

char16_t utf16::swap_byte_order16(const char16_t c)
{
    return ((c & 0xFF00) >> 8) |
           ((c & 0x00FF) << 8);
}

char32_t utf16::swap_byte_order32(const char32_t c)
{
    return ((c & 0xFF000000) >> 24) |
           ((c & 0x00FF0000) >> 8) |
           ((c & 0x0000FF00) << 8) |
           ((c & 0x000000FF) << 24);
}

std::wstring utf16::swap_byte_order(const std::wstring& ws)
{
    wstring s2;
    s2.resize(ws.length());
    for (size_t i = 0; i < ws.length(); i++)
    {
        s2[i] = utf16::swap_byte_order(ws[i]);
    }
    return s2;
}


bool utf16::try_detect_byte_order(const char* str, const size_t len, endianess::byte_order& order)
{
    if (len < 2)
        return false;
    if (*str == utf16::bom_le[0] && *(str + 1) == utf16::bom_le[1])
    {
        order = endianess::byte_order::little_endian;
        return true;
    }
    else if (*str == utf16::bom_be[0] && *(str + 1) == utf16::bom_be[1])
    {
        order = endianess::byte_order::big_endian;
        return true;
    }
    // Statistic based detection
    // If more even bytes are null, then it is little-endian.
    int null_count = 0;
    size_t scan_depth = 100;
    if (scan_depth > len)
        scan_depth = len;
    for (size_t i = 0; i < scan_depth; i++)
    {
        if (i % 2 == 0 && *(str + i) == 0)
            null_count++;
    }
    if (null_count > (int)scan_depth / 4)
        order = endianess::byte_order::little_endian;
    else
        order = endianess::platform_value();
    return true;
}

std::string utf16::wchar_to_multibyte(const std::wstring& ws,
                                      const endianess::byte_order to_order)
{
    return utf16::wchar_to_multibyte(ws.c_str(), ws.length(), to_order);
}

std::string utf16::wchar_to_multibyte(const std::wstring& ws)
{
    return utf16::wchar_to_multibyte(ws, endianess::platform_value());
}

std::string utf16::wchar_to_multibyte(const wchar_t* ws, const size_t len,
                                      const endianess::byte_order to_order)
{
    string s;
    s.reserve(len * utf16::bytes_per_character);
    for (size_t i = 0; i < len; i++)
    {
#if __STDEXT_WCHAR_SIZE == 2
        char16_t c = (char16_t)ws[i];
#else
        char16_t c = (char16_t)(ws[i] < 0xFFFF ? ws[i] : L'?');
#endif
        unsigned char* p = (unsigned char*)&c;
        if (endianess::platform_value() == to_order)
        {
            for (size_t j = 0; j < utf16::bytes_per_character; j++)
                s += p[j];
        }
        else
        {
            for (int j = utf16::bytes_per_character - 1; j >= 0; j--)
                s += p[j];
        }
    }
    return s;
}

std::string utf16::wchar_to_multibyte(const wchar_t* ws, const size_t len)
{
    return utf16::wchar_to_multibyte(ws, len, endianess::platform_value());
}

std::wstring utf16::to_lower(const std::wstring& ws)
{
    locale_guard lg(LC_ALL, "");
    wstring ws2(ws.length(), 0);
    std::transform(ws.begin(), ws.end(), ws2.begin(), std::towlower);
    return ws2;
}

std::wstring utf16::to_upper(const std::wstring& ws)
{
    locale_guard lg(LC_ALL, "");
    wstring ws2(ws.length(), 0);
    transform(ws.begin(), ws.end(), ws2.begin(), std::towupper);
    return ws2;
}

std::string utf16::to_utf8string(const std::wstring& ws)
{
    locutils::codecvt_utf8_wchar_t cvt;
    string s;
    cvt.to_utf8(ws, s);
    return s;
}

bool utf16::equal_ci(const std::wstring& ws1, const std::wstring& ws2)
{
    return utf16::to_lower(ws1) == utf16::to_lower(ws2);
}


/*
 * utf8 utils
 */
const char utf8::bom[] = { '\xEF', '\xBB', '\xBF' };
const int utf8::max_seq_length;
const unsigned char utf8::chk_seq1;
const unsigned char utf8::chk_seq2;
const unsigned char utf8::chk_seq3;
const unsigned char utf8::chk_seq4;
const unsigned char utf8::chk_seq5;
const unsigned char utf8::chk_seq6;
// code points
const char32_t utf8::code_point1;
const char32_t utf8::code_point2;
const char32_t utf8::code_point3;
const char32_t utf8::code_point4;
const char32_t utf8::code_point5;

bool utf8::is_bom(char byte0, char byte1, char byte2)
{
    return byte0 == utf8::bom[0] && byte1 == utf8::bom[1] && byte2 == utf8::bom[2];
}

void utf8::add_bom(string& bytes)
{
    if (bytes.length() < 3 || !utf8::is_bom(bytes[0], bytes[1], bytes[2]))
    {
        bytes.insert(0, utf8::bom, sizeof(utf8::bom));
    }
}

bool utf8::is_noncharacter(const unsigned char c)
{
    switch (c)
    {
    case 0xC0:
    case 0xC1:
    case 0xF5:
    case 0xFF:
        return true;
    }
    return false;
}

std::wstring utf8::to_utf16string(const std::string& s)
{
    locutils::codecvt_utf8_wchar_t cvt;
    wstring ws;
    cvt.to_utf16(s, ws);
    return ws;
}



/*
 * locale_guard class
 */
locale_guard::locale_guard(int category, const char* new_locale_name)
    : m_category(category)
{
    m_old_locale_name = std::setlocale(m_category, nullptr);
    std::setlocale(m_category, new_locale_name);
}

locale_guard::~locale_guard()
{
    std::setlocale(m_category, m_old_locale_name.c_str());
}


/*
 * codecvt_utf8_wchar_t class
 */
codecvt_utf8_wchar_t::result_t
codecvt_utf8_wchar_t::to_utf16(mbstate_t& state, const std::string& utf8s, std::wstring& utf16s, std::size_t& converted_bytes)
{
    const char* first1 = utf8s.data();
    const char* last1 = first1 + utf8s.length();
    const char* next1 = nullptr;
    utf16s.resize(utf8s.length()); // potential maximum
    wchar_t* first2 = (wchar_t*)utf16s.data();
    wchar_t* last2 = first2 + utf16s.length();
    wchar_t* next2 = nullptr;
    result_t result = do_in(state, first1, last1, next1, first2, last2, next2);
    converted_bytes = next1 - first1;
    if (next2 != nullptr && next2 >= first2)
        utf16s.resize(next2 - first2);
    return result;
}

codecvt_utf8_wchar_t::result_t
codecvt_utf8_wchar_t::to_utf16(const std::string& utf8s, std::wstring& utf16s)
{
    mbstate_t state = {};
    std::size_t converted_bytes;
    return to_utf16(state, utf8s, utf16s, converted_bytes);
}

codecvt_utf8_wchar_t::result_t
codecvt_utf8_wchar_t::to_utf8(mbstate_t& state, const std::wstring& utf16s, std::string& utf8s)
{
    const wchar_t* first1 = utf16s.data();
    const wchar_t* last1 = first1 + utf16s.length();
    const wchar_t* next1 = nullptr;
    utf8s.resize(utf16s.length() * utf8::max_seq_length + sizeof(utf8::bom)); // potential maximum
    char* first2 = (char*)utf8s.data();
    char* last2 = first2 + utf8s.length();
    char* next2 = nullptr;
    result_t result = do_out(state, first1, last1, next1, first2, last2, next2);
    if (next2 != nullptr && next2 >= first2)
        utf8s.resize(next2 - first2);
    return result;
}

codecvt_utf8_wchar_t::result_t
codecvt_utf8_wchar_t::to_utf8(const std::wstring& utf16s, std::string& utf8s)
{
    mbstate_t state = {};
    return to_utf8(state, utf16s, utf8s);
}
// UTF-8 char* -> UTF-16 wchar_t*
// Bitwise operators abstract away the endianness
codecvt_utf8_wchar_t::result_t
codecvt_utf8_wchar_t::do_in(
    mbstate_t& state,
    const extern_type* first1, const extern_type* last1, const extern_type*& next1,
    intern_type* first2, intern_type* last2, intern_type*& next2) const
{
    auto put_next2 = [&](const intern_type c)
    {
        *next2++ = c;
    };
    mbstate_adapter state_adapter(state);
    next1 = first1;
    next2 = first2;
    while (next1 != last1 && next2 != last2)
    {
        unsigned char c1 = static_cast<unsigned char>(*next1);
        char32_t c2;
        int n = 1;
        if (c1 < utf8::chk_seq1)
            c2 = c1;
        else if (c1 < utf8::chk_seq2) // 0x80-0xDF are not first byte
        {
            ++next1;
            return codecvt_base_t::error;
        }
        else if (c1 < utf8::chk_seq3)
        {
            n = 2;
            c2 = c1 & 0x1F;
        }
        else if (c1 < utf8::chk_seq4)
        {
            n = 3;
            c2 = c1 & 0x0F;
        }
        else if (c1 < utf8::chk_seq5)
        {
            n = 4;
            c2 = c1 & 0x07;
        }
        else
        {
            n = c1 < utf8::chk_seq6 ? 5 : 6;
            c2 = c1 & 0x03;
        }
        if (n == 1)
            ++next1;
        else if (last1 - next1 < n)
            break;
        else
        {
            ++next1;
            while (n > 1)
            {
                c1 = (unsigned char)*next1++;
                if (c1 < utf8::chk_seq1 || c1 >= utf8::chk_seq2)
                    return codecvt_base_t::error;
                else
                    c2 = c2 << 6 | (c1 & 0x3F);
                --n;
            }
        }
        if (state_adapter.codecvt_state() == codecvt_state::initial)
        {
            state_adapter.codecvt_state(codecvt_state::passed_once_or_more);
            if (utf16::is_bom((intern_type)c2))
            {
                if (!m_cvt_mode.consume_header())
                    put_next2((intern_type)utf16::bom_value);
                continue;
            }
            else if (m_cvt_mode.generate_header())
                put_next2((intern_type)utf16::bom_value);

        }
        if (c2 > utf16::max_char)
            return codecvt_base_t::error;
        if (utf16::is_surrogate_pair(c2))
        {
            if (last2 - next2 < 2)
                return codecvt_base::error;
            wchar_t high, low;
            if (!utf16::to_surrogate_pair(c2, high, low))
                return codecvt_base_t::error;
            put_next2(high);
            put_next2(low);
        }
        else
            put_next2((intern_type)c2);
    }
    return (next1 < last1 ? codecvt_base_t::partial : codecvt_base_t::ok);
}

// UTF-16 wchar_t* -> UTF-8 char*
codecvt_utf8_wchar_t::result_t
codecvt_utf8_wchar_t::do_out(
    mbstate_t& state,
    const intern_type* first1, const intern_type* last1, const intern_type*& next1,
    extern_type* first2, extern_type* last2, extern_type*& next2) const
{
    mbstate_adapter state_adapter(state);
    next1 = first1;
    next2 = first2;
    while (next1 != last1 && next2 != last2)
    {	// convert and put a wide char
        extern_type c2;
        int n = 1;
        char32_t c1 = *next1;
        if (utf16::is_high_surrogate(*next1))
        {
            if (last1 - next1 > 0)
            {
                wchar_t high = *next1;
                wchar_t low = *(next1 + 1);
                if (utf16::from_surrogate_pair(high, low, c1))
                    ++next1;
            }
        }
        if (c1 > utf16::max_char)
            return codecvt_base_t::error;
        if (c1 < utf8::code_point1)
            c2 = (extern_type)c1;
        else if (c1 < utf8::code_point2)
        {
            c2 = (extern_type)(utf8::chk_seq2 | c1 >> 6);
            n = 2;
        }
        else if (c1 < utf8::code_point3)
        {
            c2 = (extern_type)(utf8::chk_seq3 | c1 >> 12);
            n = 3;
        }
        else if (c1 < utf8::code_point4)
        {
            c2 = (extern_type)(utf8::chk_seq4 | c1 >> 18);
            n = 4;
        }
        else if (c1 < utf8::code_point5)
        {
            c2 = (extern_type)(utf8::chk_seq5 | c1 >> 24);
            n = 5;
        }
        else
        {
            c2 = (extern_type)(utf8::chk_seq6 | (c1 >> 30 & 0x03));
            n = 6;
        }
        if (state_adapter.codecvt_state() == codecvt_state::initial)
        {
            state_adapter.codecvt_state(codecvt_state::passed_once_or_more);
            if (m_cvt_mode.generate_header())
            {
                if (last2 - next2 < (int)sizeof(utf8::bom) + n)
                    return codecvt_base_t::error;
                // generate header
                *next2++ = (extern_type)utf8::bom[0];
                *next2++ = (extern_type)utf8::bom[1];
                *next2++ = (extern_type)utf8::bom[2];
            }
            if (utf16::is_bom((intern_type)c1))
            {
                ++next1;
                continue;
            }
        }
        if (last2 - next2 < n)
            return codecvt_base_t::partial;
        *next2++ = c2;
        --n;
        while (n > 0)
        {
            *next2++ = (extern_type)((c1 >> 6 * --n & 0x3F) | 0x80);
        }
        ++next1;
    }
    return (first1 == next1 ? codecvt_base_t::partial : codecvt_base_t::ok);
}

int codecvt_utf8_wchar_t::do_max_length() const noexcept
{
    return (m_cvt_mode.consume_header() || m_cvt_mode.generate_header() ? 7 : 4);
}

int codecvt_utf8_wchar_t::do_encoding() const noexcept
{
    // -1 => state dependent, 0 => varying length
    return (m_cvt_mode.consume_header() || m_cvt_mode.generate_header() ? -1 : 0);
}

// generate bytes to return to default shift state
codecvt_utf8_wchar_t::result_t
codecvt_utf8_wchar_t::do_unshift(mbstate_t&, extern_type* first2, extern_type*, extern_type*& next2) const noexcept
{
    next2 = first2;
    return codecvt_base_t::ok;
}

// return min(len2, converted length of bytes [first1, last1))
int codecvt_utf8_wchar_t::do_length(mbstate_t& state, const extern_type* first1, const extern_type* last1, size_t len2) const noexcept
{
    size_t curr_length = 0;
    mbstate_t curr_state = state;
    while (curr_length < len2 && first1 != last1)
    {	// convert another wide character
        const extern_type* next1;
        intern_type* next2;
        intern_type buf2[2];
        // test result of single wide-char conversion
        switch (do_in(curr_state, first1, last1, next1, buf2, buf2 + 1, next2))
        {
        case codecvt_base_t::noconv:
            return (int)(curr_length + (last1 - first1));
        case codecvt_base_t::ok:
            if (next2 == buf2 + 1)
            {
                ++curr_length;	// replacement do_in might not convert one
            }
            first1 = next1;
            break;
        default:
            return (int)curr_length;	// error or partial
        }
    }
    return (int)curr_length;
}


/*
 * codecvt_utf16_wchar_t class
 */
codecvt_utf16_wchar_t::result_t
codecvt_utf16_wchar_t::mb_to_utf16(mbstate_t& state, const std::string& mbs, std::wstring& ws)
{
    const char* first1 = mbs.data();
    const char* last1 = first1 + mbs.length();
    const char* next1 = nullptr;
    size_t len2 = // mbstr length should be an even number (binary stream of UTF-16 bytes)
        (mbs.length() % 2 == 0 ? mbs.length() : mbs.length() + 1) / bytes_per_character
        + 1;  // BOM
    ws.resize(len2);
    wchar_t* first2 = (wchar_t*)ws.data();
    wchar_t* last2 = first2 + ws.length();
    wchar_t* next2 = nullptr;
    result_t result = do_in(state, first1, last1, next1, first2, last2, next2);
    if (next2 != nullptr && next2 >= first2)
        ws.resize(next2 - first2);
    return result;
}

codecvt_utf16_wchar_t::result_t
codecvt_utf16_wchar_t::mb_to_utf16(const std::string& mbs, std::wstring& ws)
{
    mbstate_t state = {};
    return mb_to_utf16(state, mbs, ws);
}

codecvt_utf16_wchar_t::result_t
codecvt_utf16_wchar_t::utf16_to_mb(mbstate_t& state, const std::wstring& ws, std::string& mbs)
{
    const wchar_t* first1 = ws.data();
    const wchar_t* last1 = first1 + ws.length();
    const wchar_t* next1 = nullptr;
    size_t len2 = (ws.length() + 1 /*BOM*/) * bytes_per_character;
    mbs.resize(len2);
    char* first2 = (char*)mbs.data();
    char* last2 = first2 + mbs.length();
    char* next2 = nullptr;
    result_t result = do_out(state, first1, last1, next1, first2, last2, next2);
    if (next2 != nullptr && next2 >= first2)
        mbs.resize(next2 - first2);
    return result;
}

codecvt_utf16_wchar_t::result_t
codecvt_utf16_wchar_t::utf16_to_mb(const std::wstring& ws, std::string& mbs)
{
    mbstate_t state = {};
    return utf16_to_mb(state, ws, mbs);
}

// multibyte char* (extern) sequence --> UTF-16 wchar_t* (intern) sequence
codecvt_utf16_wchar_t::result_t
codecvt_utf16_wchar_t::do_in(
    mbstate_t& state,
    const extern_type* first1, const extern_type* last1, const extern_type*& next1,
    intern_type* first2, intern_type* last2, intern_type*& next2) const
{
    auto get_next1 = [&](intern_type& c2, endianess::byte_order source_order)
    {
        // Source byte order may be different from internal one (cf. "endianess")
        // every byte pair should be converted to wchar_t (16/32 bit)
        // Examples:
        // Source LE "\x41\x04" (0x0441) --> Target 16 bit 0x0441 | 32 bit 0x00000441
        // Source BE "\x04\x41" (0x0441) --> Target 16 bit 0x0441 | 32 bit 0x00000441
        char16_t c = source_order == endianess::byte_order::little_endian ?
                    (((unsigned char)(*(next1 + 1))) << 8) | (unsigned char)(*next1)
                  :
                    (((unsigned char)(*next1)) << 8) | (unsigned char)(*(next1 + 1));
        c2 = (wchar_t)c;
        next1 += bytes_per_character;
    };
    auto put_next2 = [&](const intern_type& c2)
    {
        *next2++ = c2;
    };
    //
    mbstate_adapter state_adapter(state);
    next1 = first1;
    next2 = first2;
    while (last1 - next1 >= bytes_per_character && next2 != last2)
    {
        intern_type c2;
        if (state_adapter.codecvt_state() == codecvt_state::initial)
        {
            state_adapter.byte_order(m_cvt_mode.byte_order());
            state_adapter.codecvt_state(codecvt_state::passed_once_or_more);
            if (!m_cvt_mode.is_byte_order_assigned())
            {
                endianess::byte_order order;
                if (utf16::try_detect_byte_order(first1, last1 - first1, order))
                    state_adapter.byte_order(order);
            }
            get_next1(c2, state_adapter.byte_order());
            if (m_cvt_mode.generate_header())
            {
                if (next2 != last2)
                    put_next2(utf16::bom_value);
            }
            if (utf16::is_bom(c2))
                continue;
        }
        else
            get_next1(c2, state_adapter.byte_order());
        put_next2(c2);
    }
    return (first1 == next1 ? codecvt_base_t::partial : codecvt_base_t::ok);
}

// UTF-16 wchar_t* (intern) sequence --> multibyte char* (extern) sequence
codecvt_utf16_wchar_t::result_t
codecvt_utf16_wchar_t::do_out(mbstate_t& state,
                              const intern_type* first1, const intern_type* last1, const intern_type*& next1,
                              extern_type* first2, extern_type* last2, extern_type*& next2) const
{
    auto put_char2 = [&](const intern_type& c2)
    {
        if (m_cvt_mode.byte_order() == endianess::byte_order::little_endian)
        {
            *next2++ = (unsigned char)(c2 & 0xFF);
            *next2++ = (unsigned char)((c2 >> 8) & 0xFF);
        }
        else
        {
            *next2++ = (unsigned char)((c2 >> 8) & 0xFF);
            *next2++ = (unsigned char)(c2 & 0xFF);
        }
    };
    mbstate_adapter state_adapter(state);
    next1 = first1;
    next2 = first2;
    if (state_adapter.codecvt_state() == codecvt_state::initial)
    {
        state_adapter.codecvt_state(codecvt_state::passed_once_or_more);
        if (m_cvt_mode.generate_header())
        {
            if (last2 - next2 < bytes_per_character * 3)
                return codecvt_base_t::partial;
            if (m_cvt_mode.byte_order() == endianess::byte_order::big_endian)
            {
                *next2++ = utf16::bom_be[0];
                *next2++ = utf16::bom_be[1];
            }
            else
            {
                *next2++ = utf16::bom_le[0];
                *next2++ = utf16::bom_le[1];
            }
        }
    }
    while (next1 != last1 && last2 - next2 >= bytes_per_character)
    {
        char32_t c2 = (char32_t)*next1++; // case of 4-byte wchar_t
        if (c2 > utf16::max_char)
            return codecvt_base_t::error;
        if (utf16::is_bom((wchar_t)c2))
            continue;
        if (c2 <= utf16::max_char)
        {
            put_char2((intern_type)c2);
        }
        else
        {
            if (last2 - next2 < bytes_per_character * 2)
            {
                --next1;
                return codecvt_base_t::partial;
            }
            wchar_t c21, c22;
            utf16::to_surrogate_pair(c2, c21, c22);
            put_char2(c21);
            put_char2(c22);
        }
    }
    return (first1 == next1 ? codecvt_base_t::partial : codecvt_base_t::ok);
}

codecvt_utf16_wchar_t::result_t
codecvt_utf16_wchar_t::do_unshift(mbstate_t&, extern_type* first2, extern_type*, extern_type*& next2) const noexcept
{
    next2 = first2;
    return codecvt_base_t::ok;
}

// return min(len2, converted length of bytes [first1, last1))
int codecvt_utf16_wchar_t::do_length(mbstate_t& state, const extern_type* first1, const extern_type* last1, size_t len2) const noexcept
{
    size_t curr_length = 0;
    mbstate_t curr_state = state;
    while (curr_length < len2 && first1 != last1)
    {
        const extern_type* next1;
        intern_type* next2;
        intern_type buf2[2];
        switch (do_in(curr_state, first1, last1, next1, buf2, buf2 + 1, next2))
        {
        case codecvt_base_t::noconv:
            return (int)(curr_length + (last1 - first1));
        case codecvt_base_t::ok:
            if (next2 == buf2 + 1)
                ++curr_length;
            first1 = next1;
            break;
        default:
            return (int)curr_length;
        }
    }
    return (int)curr_length;
}

int codecvt_utf16_wchar_t::do_max_length() const noexcept
{
    return (m_cvt_mode.consume_header() || m_cvt_mode.generate_header() ? 2 * bytes_per_character : bytes_per_character);
}

int codecvt_utf16_wchar_t::do_encoding() const noexcept
{
    return (m_cvt_mode.consume_header() || m_cvt_mode.generate_header() ? -1 : 0);
}


/*
 * codecvt_ansi_utf16_wchar_t class
 */
std::string to_encoding_name(const ansi_encoding encoding, const ansi_encoding_naming naming)
{
    switch (naming)
    {
    case ansi_encoding_naming::iconv:
    {
        switch (encoding)
        {
        case ansi_encoding::cp1250: return "CP1250";
        case ansi_encoding::cp1251: return "CP1251";
        case ansi_encoding::cp1252: return "CP1252";
        default: return "";
        }
    }
    case ansi_encoding_naming::windows:
        switch (encoding)
        {
        case ansi_encoding::cp1250: return ".1250";
        case ansi_encoding::cp1251: return ".1251";
        case ansi_encoding::cp1252: return ".1252";
        default: return "";
        }
    default:
        return "";
    }
}


codecvt_ansi_utf16_wchar_t::result_t
codecvt_ansi_utf16_wchar_t::ansi_to_utf16(mbstate_t& state, const std::string& s, std::wstring& ws) const
{
    const char* first1 = s.data();
    const char* last1 = first1 + s.length();
    const char* next1 = nullptr;
    size_t len2 = s.length() + (m_cvt_mode.generate_header() ? 1 : 0);
    ws.resize(len2);
    wchar_t* first2 = (wchar_t*)ws.data();
    wchar_t* last2 = first2 + ws.length();
    wchar_t* next2 = nullptr;
    result_t result = do_in(state, first1, last1, next1, first2, last2, next2);
    if (next2 != nullptr && next2 >= first2)
        ws.resize(next2 - first2);
    return result;
}

codecvt_ansi_utf16_wchar_t::result_t
codecvt_ansi_utf16_wchar_t::ansi_to_utf16(const std::string& s, std::wstring& ws) const
{
    mbstate_t state = {};
    return ansi_to_utf16(state, s, ws);
}


codecvt_ansi_utf16_wchar_t::result_t
codecvt_ansi_utf16_wchar_t::utf16_to_ansi(mbstate_t& state, const std::wstring& ws, std::string& s) const
{
    const wchar_t* first1 = ws.data();
    const wchar_t* last1 = first1 + ws.length();
    const wchar_t* next1 = nullptr;
    size_t len2 = ws.length() + 1 /*BOM*/;
    s.resize(len2);
    char* first2 = (char*)s.data();
    char* last2 = first2 + s.length();
    char* next2 = nullptr;
    result_t result = do_out(state, first1, last1, next1, first2, last2, next2);
    if (next2 != nullptr && next2 >= first2)
        s.resize(next2 - first2);
    return result;
}

codecvt_ansi_utf16_wchar_t::result_t
codecvt_ansi_utf16_wchar_t::utf16_to_ansi(const std::wstring& ws, std::string& s) const
{
    mbstate_t state = {};
    return utf16_to_ansi(state, ws, s);
}

// ANSI codepage (char*) --> UTF-16 (wchar_t*)
codecvt_ansi_utf16_wchar_t::result_t
codecvt_ansi_utf16_wchar_t::do_in(mbstate_t& state,
                                  const extern_type* first1, const extern_type* last1, const extern_type*& next1,
                                  intern_type* first2, intern_type* last2, intern_type*& next2) const
{
    result_t ret = codecvt_base_t::ok;
    mbstate_adapter state_adapter(state);
    next1 = first1;
    next2 = first2;
    if (state_adapter.codecvt_state() == codecvt_state::initial)
    {
        state_adapter.codecvt_state(codecvt_state::passed_once_or_more);
        if (next2 < last2 && m_cvt_mode.generate_header())
            *next2++ = utf16::bom_value;
    }
#if defined(__STDEXT_USE_ICONV)
    iconv_t conv = iconv_open("WCHAR_T", m_cvt_mode.encoding_name_iconv().c_str());
    if (conv == (iconv_t) -1)
        return codecvt_base_t::error;
    if (next1 < last1 && next2 < last2)
    {
        size_t size1 = last1 - next1;
        size_t size2 = (last2 - next2) * sizeof(wchar_t);
        char* ps1 = (char*)next1;
        char* ps2 = (char*)next2;
        size_t rc = iconv(conv, &ps1, &size1, &ps2, &size2);
        if (rc == (size_t) -1)
        {
            switch (errno)
            {
            case E2BIG:
                ret = codecvt_base_t::partial;
                break;
            case EILSEQ:
            case EINVAL:
                ret = codecvt_base_t::error;
                break;
            default:
                break;
            }
        }
        else
        {
            next1 = last1 - size1;
            next2 = last2 - (size2 / sizeof(wchar_t));
            if (first1 == next1)
                ret = codecvt_base_t::partial;
        }
    }
    iconv_close(conv);
#else
    std::locale loc = std::locale(m_cvt_mode.encoding_name_windows());
    if (std::has_facet<codecvt_base_t>(loc))
    {
        const codecvt_base_t& facet = use_facet<codecvt_base_t>(loc);
        mbstate_t state2;
        intern_type* first2_copy = next2;
        ret = facet.in(state2, first1, last1, next1, first2_copy, last2, next2);
    }
    else
        ret = codecvt::error;
#endif
    return ret;
}

// UTF-16 (wchar_t*) --> ANSI codepage (char*)
codecvt_ansi_utf16_wchar_t::result_t
codecvt_ansi_utf16_wchar_t::do_out(mbstate_t& state,
                                   const intern_type* first1, const intern_type* last1, const intern_type*& next1,
                                   extern_type* first2, extern_type* last2, extern_type*& next2) const
{
    result_t ret = codecvt_base_t::ok;
    mbstate_adapter state_adapter(state);
    next1 = first1;
    next2 = first2;
    if (state_adapter.codecvt_state() == codecvt_state::initial)
    {
        state_adapter.codecvt_state(codecvt_state::passed_once_or_more);
        if (next1 < last1 && utf16::is_bom(*next1))
            ++next1;
    }
#if defined(__STDEXT_USE_ICONV)
    iconv_t conv = iconv_open(m_cvt_mode.encoding_name_iconv().c_str(), "WCHAR_T");
    if (conv == (iconv_t) -1)
        return codecvt_base_t::error;
    if (next1 < last1 && next2 < last2)
    {
        size_t size1 = (last1 - next1) * sizeof(wchar_t);
        size_t size2 = last2 - next2;
        char* ps1 = (char*)next1;
        char* ps2 = next2;
        size_t rc = iconv(conv, &ps1, &size1, &ps2, &size2);
        if (rc == (size_t) -1)
        {
            switch (errno)
            {
            case E2BIG:
                ret = codecvt_base_t::partial;
                break;
            case EILSEQ:
            case EINVAL:
                ret = codecvt_base_t::error;
                break;
            default:
                break;
            }
        }
        else
        {
            next1 = last1 - (size1 / sizeof(wchar_t));
            next2 = last2 - size2;
            if (first1 == next1)
                ret = codecvt_base_t::partial;
        }
    }
    iconv_close(conv);
#else
    std::locale loc = std::locale(m_cvt_mode.encoding_name_windows());
    if (std::has_facet<codecvt_base_t>(loc))
    {
        const codecvt_base_t& facet = use_facet<codecvt_base_t>(loc);
        mbstate_t state2;
        const intern_type* first1_copy = next1;
        ret = facet.out(state2, first1_copy, last1, next1, first2, last2, next2);
    }
    else
        ret = codecvt::error;
#endif
    return ret;
}

codecvt_ansi_utf16_wchar_t::result_t
codecvt_ansi_utf16_wchar_t::do_unshift(mbstate_t&, extern_type* first2, extern_type*, extern_type*& next2) const noexcept
{
    next2 = first2;
    return codecvt_base_t::ok;
}

int codecvt_ansi_utf16_wchar_t::do_length(mbstate_t& state,
                                          const extern_type* first1,
                                          const extern_type* last1,
                                          size_t maxlen2) const noexcept
{
    size_t len2 = 0;
    mbstate_t curr_state = state;
    while (len2 < maxlen2 && first1 != last1)
    {
        const extern_type* next1;
        intern_type* next2;
        intern_type buf2[2];
        switch (do_in(curr_state, first1, last1, next1, buf2, buf2 + 1, next2))
        {
        case codecvt_base_t::noconv:
            return (int)(len2 + (last1 - first1));
        case codecvt_base_t::ok:
            if (next2 == buf2 + 1)
                ++len2;
            first1 = next1;
            break;
        default:
            return (int)len2;
        }
    }
    return (int)len2;
}

int codecvt_ansi_utf16_wchar_t::do_max_length() const noexcept
{
    // 1 character of intern type (wchar_t) can be converted to maximum N characters of extern type (char)
    return 1;
}

int codecvt_ansi_utf16_wchar_t::do_encoding() const noexcept
{
    return (m_cvt_mode.consume_header() || m_cvt_mode.generate_header() ? -1 : 0);
}

}

