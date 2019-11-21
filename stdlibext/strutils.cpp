#include <cstdlib>
#include <stdio.h>
#include <stdarg.h>

#include "strutils.h"

using namespace std;
using namespace stdext;

string strutils::format(const string fmt, ...)
{
    size_t size = fmt.size() * 2;
    string s;
    va_list ap;
    while (true)
    {
        s.resize(size);
        va_start(ap, fmt);
        int n =
#if defined(__BCPLUSPLUS__)
            vsnprintf_s((char*)s.data(), s.size(), fmt.c_str(), ap);
#else
            vsnprintf_s((char*)s.data(), s.size(), size - 1, fmt.c_str(), ap);
#endif
        va_end(ap);
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

wstring strutils::format(const wstring fmt, ...)
{
    size_t size = fmt.size() * 2;
    wstring s;
    va_list ap;
    while (true)
    {
        s.resize(size);
        va_start(ap, fmt);
        int n =
#if defined(__BCPLUSPLUS__)
            vsnwprintf_s((wchar_t *)s.data(), s.size(), fmt.c_str(), ap);
#else
            _vsnwprintf_s((wchar_t *)s.data(), s.size(), size - 1, fmt.c_str(), ap);
#endif
        va_end(ap);
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
