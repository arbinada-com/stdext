#include <cstdlib>
#include <memory>
#include <stdio.h>
#include <stdarg.h>

#include "strutils.h"

using namespace std;
using namespace stdext;

int basic_vsnprintf_s(std::string& s, const std::string& fmt, va_list args)
{
#if defined(__BCPLUSPLUS__)
    return vsnprintf_s((char*)s.data(), s.size(), fmt.c_str(), args);
#else
    return vsnprintf_s((char*)s.data(), s.size(), s.size() - 1, fmt.c_str(), args);
#endif
}

int basic_vsnprintf_s(std::wstring& s, const std::wstring& fmt, va_list args)
{
#if defined(__BCPLUSPLUS__)
    return vsnwprintf_s((wchar_t*)s.data(), s.size(), fmt.c_str(), args);
#else
    return _vsnwprintf_s((wchar_t*)s.data(), s.size(), s.size() - 1, fmt.c_str(), args);
#endif
}

template<class StringT>
StringT basic_format(const StringT& fmt, va_list args)
{
    size_t size = fmt.size() * 2;
    StringT s;
    while (true)
    {
        s.resize(size);
        int n = basic_vsnprintf_s(s, fmt, args);
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
    string s = basic_format<std::string>(fmt, ap);
    va_end(ap);
    return s;
}

wstring strutils::format(const wstring fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    wstring s = basic_format<std::wstring>(fmt, ap);
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

std::string strutils::ltrim(const std::string& s)
{
    return basic_ltrim_chars<std::string>(s, whitespaces());
}

std::wstring strutils::ltrim(const std::wstring& s)
{
    return basic_ltrim_chars<std::wstring>(s, wwhitespaces());
}

std::string strutils::ltrim_chars(const std::string& s, const std::string& chars)
{
    return basic_ltrim_chars<std::string>(s, chars);
}

std::wstring strutils::ltrim_chars(const std::wstring& s, const std::wstring& chars)
{
    return basic_ltrim_chars<std::wstring>(s, chars);
}

std::string strutils::rtrim(const std::string& s)
{
    return basic_rtrim_chars<std::string>(s, whitespaces());
}

std::wstring strutils::rtrim(const std::wstring& s)
{
    return basic_rtrim_chars<std::wstring>(s, wwhitespaces());
}

std::string strutils::rtrim_chars(const std::string& s, const std::string& chars)
{
    return basic_rtrim_chars<std::string>(s, chars);
}

std::wstring strutils::rtrim_chars(const std::wstring& s, const std::wstring& chars)
{
    return basic_rtrim_chars<std::wstring>(s, chars);
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


template<class StringT>
StringT basic_replace_all(StringT& s, const StringT& match, const StringT& repl)
{
    size_t pos = 0;
    while ((pos = s.find(match, pos)) != StringT::npos)
    {
        s.replace(pos, match.length(), repl);
        pos += repl.length();
    }
    return s;
}

std::string strutils::replace_all(std::string s, const std::string& match, const std::string& repl)
{
    return basic_replace_all<std::string>(s, match, repl);
}

std::wstring strutils::replace_all(std::wstring s, const std::wstring& match, const std::wstring& repl)
{
    return basic_replace_all<std::wstring>(s, match, repl);
}

string strutils::to_string(const wstring& ws)
{
    string s;
    for (char wc : ws)
        s += wc; // implicit char conversion
    return s;
}
