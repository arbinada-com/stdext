#include <cstdlib>
#include <memory>
#include <stdio.h>
#include <stdarg.h>

#include "strutils.h"

using namespace std;
using namespace stdext;

int basic_vsnprintf_s(char* const s, size_t const size, const char* fmt, va_list args)
{
#if defined(__BCPLUSPLUS__)
    return vsnprintf_s(s, size, fmt, args);
#else
    return vsnprintf_s(s, size, size - 1, fmt, args);
#endif
}

int basic_vsnprintf_s(wchar_t* const s, size_t const size, const wchar_t* fmt, va_list args)
{
#if defined(__BCPLUSPLUS__)
    return vsnwprintf_s(s, size, fmt, args);
#else
    return _vsnwprintf_s(s, size, size - 1, fmt, args);
#endif
}

template<class CharT, class Traits = std::char_traits<CharT>, class Alloc = allocator<CharT> >
std::basic_string<CharT, Traits, Alloc> basic_format(
    const std::basic_string<CharT, Traits, Alloc>& fmt, 
    va_list args)
{
    size_t size = fmt.size() * 2;
    std::basic_string<CharT, Traits, Alloc> s;
    while (true)
    {
        s.resize(size);
        int n = basic_vsnprintf_s((CharT*)s.data(), s.size(), fmt.c_str(), args);
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
    return s;
}

string strutils::format(const string fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    string s = basic_format<char>(fmt, ap);
    va_end(ap);
    return s;
}

wstring strutils::format(const wstring fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    wstring s = basic_format<wchar_t>(fmt, ap);
    va_end(ap);
    return s;
}

const std::string whitespaces() { return " \n\r\t\f\v"; }
const std::wstring wwhitespaces() { return L" \n\r\t\f\v"; }

template<class CharT, class Traits = std::char_traits<CharT>, class Alloc = allocator<CharT> >
std::basic_string<CharT, Traits, Alloc> basic_ltrim_chars(
    const std::basic_string<CharT, Traits, Alloc>& s, 
    const std::basic_string<CharT, Traits, Alloc>& chars)
{
    size_t start = s.find_first_not_of(chars);
    return (start == basic_string<CharT, Traits, Alloc>::npos) ? basic_string<CharT, Traits, Alloc>() : s.substr(start);
}

template<class CharT, class Traits = std::char_traits<CharT>, class Alloc = allocator<CharT> >
std::basic_string<CharT, Traits, Alloc> basic_rtrim_chars(
    const std::basic_string<CharT, Traits, Alloc>& s,
    const std::basic_string<CharT, Traits, Alloc>& chars)
{
    size_t end = s.find_last_not_of(chars);
    return (end == std::basic_string<CharT, Traits>::npos) ? basic_string<CharT, Traits, Alloc>() : s.substr(0, end + 1);
}

std::string strutils::ltrim(const std::string& s)
{
    return basic_ltrim_chars<char, std::char_traits<char>, std::allocator<char> >(s, whitespaces());
}

std::wstring strutils::ltrim(const std::wstring& s)
{
    return basic_ltrim_chars<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >(s, wwhitespaces());
}

std::string strutils::ltrim_chars(const std::string& s, const std::string& chars)
{
    return basic_ltrim_chars<char, std::char_traits<char>, std::allocator<char> >(s, chars);
}

std::wstring strutils::ltrim_chars(const std::wstring& s, const std::wstring& chars)
{
    return basic_ltrim_chars<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >(s, chars);
}

std::string strutils::rtrim(const std::string& s)
{
    return basic_rtrim_chars<char, std::char_traits<char>, std::allocator<char> >(s, whitespaces());
}

std::wstring strutils::rtrim(const std::wstring& s)
{
    return basic_rtrim_chars<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >(s, wwhitespaces());
}

std::string strutils::rtrim_chars(const std::string& s, const std::string& chars)
{
    return basic_rtrim_chars<char, std::char_traits<char>, std::allocator<char> >(s, chars);
}

std::wstring strutils::rtrim_chars(const std::wstring& s, const std::wstring& chars)
{
    return basic_rtrim_chars<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t> >(s, chars);
}

std::string strutils::trim(const std::string& s)
{
    return rtrim(ltrim(s));
}

std::wstring strutils::trim(const std::wstring& s)
{
    return rtrim(ltrim(s));
}

std::string strutils::trim_chars(const std::string& s, const std::string& chars)
{
    return rtrim_chars(ltrim_chars(s, chars), chars);
}

std::wstring strutils::trim_chars(const std::wstring& s, const std::wstring& chars)
{
    return rtrim_chars(ltrim_chars(s, chars), chars);
}


std::wstring strutils::replace_all(std::wstring s, const std::wstring& match, const std::wstring& repl)
{
    size_t pos = 0;
    while ((pos = s.find(match, pos)) != std::string::npos)
    {
        s.replace(pos, match.length(), repl);
        pos += repl.length();
    }
    return s;
}

string strutils::to_string(const wstring& ws)
{
    string s;
    for (char wc : ws)
        s += wc; // implicit char conversion
    return s;
}
