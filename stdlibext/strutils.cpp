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
        int n = vsnprintf_s((char*)s.data(), s.size(), size - 1, fmt.c_str(), ap);
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
        int n = _vsnwprintf_s((wchar_t *)s.data(), s.size(), size - 1, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size)
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

string strutils::to_string(const wstring& ws)
{
    string s;
    for (char wc : ws)
        s += wc; // implicit char conversion
    return s;
}
