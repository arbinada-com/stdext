#include <cstdlib>
#include <cctype>
#include <cwctype>
#include <algorithm>
#include <memory>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#if defined(__STDEXT_USE_BOOST)
#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>
#endif

#include "strutils.h"
#include "platforms.h"

using namespace std;

namespace stdext {
namespace str {

inline int basic_vsnprintf_s(std::string& s, const char* fmt, va_list args)
{
#if defined(__STDEXT_WINDOWS)
    #if defined(__BCPLUSPLUS__)
    return vsnprintf_s((char*)s.data(), s.size(), fmt, args);
    #else
    return vsnprintf_s((char*)s.data(), s.size(), s.size() - 1, fmt, args);
    #endif
#elif defined(__STDEXT_LINUX)
    return vsnprintf((char*)s.data(), s.size(), fmt, args);
#else
    #error Unsupported platfotm
#endif
}

inline int basic_vsnprintf_s(std::wstring& s, const wchar_t* fmt, va_list args)
{
#if defined(__STDEXT_WINDOWS)
    #if defined(__BCPLUSPLUS__)
    return vsnwprintf_s((wchar_t*)s.data(), s.size(), fmt, args);
    #else
    return _vsnwprintf_s((wchar_t*)s.data(), s.size(), s.size() - 1, fmt, args);
    #endif
#elif defined(__STDEXT_LINUX)
    return vswprintf((wchar_t*)s.data(), s.size(), fmt, args);
#else
    #error Unsupported platfotm
#endif
}

inline size_t basic_strlen(const char* s) { return strlen(s); }
inline size_t basic_strlen(const wchar_t* s) { return wcslen(s); }

template<class StringT, class CharT>
StringT basic_format(const CharT* fmt, va_list args)
{
    size_t size = basic_strlen(fmt) * 2;
    while (true)
    {
        StringT s;
        s.resize(size);
        va_list args2;
        va_copy(args2, args); // On Linux/gcc args va_list pointer may be changed so use the copy
        int n = basic_vsnprintf_s(s, fmt, args2);
        if (n > -1 && (size_t)n < size)
        {
            s.resize(n);
            return s;
        }
        if (n > -1)
            size = n + 1;
        else
            size *= 2;
    }
}

std::string format(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    string s = basic_format<std::string, char>(fmt, ap);
    va_end(ap);
    return s;
}

string format(const std::string fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    string s = basic_format<std::string, char>(fmt.c_str(), ap);
    va_end(ap);
    return s;
}

std::wstring wformat(const wchar_t* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    wstring s = basic_format<std::wstring, wchar_t>(fmt, ap);
    va_end(ap);
    return s;
}

wstring wformat(const std::wstring fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    wstring s = basic_format<std::wstring, wchar_t>(fmt.c_str(), ap);
    va_end(ap);
    return s;
}


const std::string whitespaces() { return " \n\r\t\f\v"; }
const std::wstring wwhitespaces() { return L" \n\r\t\f\v"; }

template<class StringT>
StringT basic_ltrim_chars(const StringT& s, const StringT& chars)
{
    size_t start = s.find_first_not_of(chars);
    return (start == StringT::npos) ? StringT() : s.substr(start);
}

template<class StringT>
StringT basic_rtrim_chars(const StringT& s, const StringT& chars)
{
    size_t end = s.find_last_not_of(chars);
    return (end == StringT::npos) ? StringT() : s.substr(0, end + 1);
}

std::string ltrim(const std::string& s)
{
    return basic_ltrim_chars<std::string>(s, whitespaces());
}

std::wstring ltrim(const std::wstring& s)
{
    return basic_ltrim_chars<std::wstring>(s, wwhitespaces());
}

std::string ltrim_chars(const std::string& s, const std::string& chars)
{
    return basic_ltrim_chars<std::string>(s, chars);
}

std::wstring ltrim_chars(const std::wstring& s, const std::wstring& chars)
{
    return basic_ltrim_chars<std::wstring>(s, chars);
}

std::string rtrim(const std::string& s)
{
    return basic_rtrim_chars<std::string>(s, whitespaces());
}

std::wstring rtrim(const std::wstring& s)
{
    return basic_rtrim_chars<std::wstring>(s, wwhitespaces());
}

std::string rtrim_chars(const std::string& s, const std::string& chars)
{
    return basic_rtrim_chars<std::string>(s, chars);
}

std::wstring rtrim_chars(const std::wstring& s, const std::wstring& chars)
{
    return basic_rtrim_chars<std::wstring>(s, chars);
}

std::string trim(const std::string& s)
{
    return rtrim(ltrim(s));
}

std::wstring trim(const std::wstring& s)
{
    return rtrim(ltrim(s));
}

std::string trim_chars(const std::string& s, const std::string& chars)
{
    return rtrim_chars(ltrim_chars(s, chars), chars);
}

std::wstring trim_chars(const std::wstring& s, const std::wstring& chars)
{
    return rtrim_chars(ltrim_chars(s, chars), chars);
}


template<class StringT>
StringT basic_replace(
    StringT& s,
    const StringT& match,
    const StringT& repl,
    const size_t from_pos,
    const bool all)
{
    size_t pos = from_pos;
    while ((pos = s.find(match, pos)) != StringT::npos)
    {
        s.replace(pos, match.length(), repl);
        if (!all)
            break;
        pos += repl.length();
    }
    return s;
}

std::string replace(
    std::string s,
    const std::string& match,
    const std::string& repl,
    const std::size_t from_pos,
    const bool all)
{
    return basic_replace<std::string>(s, match, repl, from_pos, all);
}

std::wstring replace(
    std::wstring s,
    const std::wstring& match,
    const std::wstring& repl,
    const std::size_t from_pos,
    const bool all)
{
    return basic_replace<std::wstring>(s, match, repl, from_pos, all);
}

std::string replace_all(std::string s, const std::string& match, const std::string& repl)
{
    return basic_replace<std::string>(s, match, repl, 0, true);
}

std::wstring replace_all(std::wstring s, const std::wstring& match, const std::wstring& repl)
{
    return basic_replace<std::wstring>(s, match, repl, 0, true);
}

std::string quoted(const std::string& s)
{
    return "'" + s + "'";
}
std::wstring quoted(const std::wstring& s)
{
    return L"'" + s + L"'";
}
std::string double_quoted(const std::string& s)
{
    return "\"" + s + "\"";
}
std::wstring double_quoted(const std::wstring& s)
{
    return L"\"" + s + L"\"";
}


string to_string(const wstring& ws)
{
#if defined(__STDEXT_USE_BOOST)
    return boost::locale::conv::utf_to_utf<char>(ws);
#else
    string s(ws.begin(), ws.end()); // ANSI charset only
    return s;
#endif
}

wstring to_wstring(const string& s)
{
#if defined(__STDEXT_USE_BOOST)
    return boost::locale::conv::utf_to_utf<wchar_t>(s);
#else
    // ANSI charset only
    wstring ws;
    ws.resize(s.length());
    for (string::size_type i = 0; i < s.length(); i++)
        ws[i] = (wchar_t)((unsigned char)s[i]);
    return ws;
#endif
}

}
}
