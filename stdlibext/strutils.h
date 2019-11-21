/*
 C++ standard library extensions
 (c) 2001-2019 Serguei Tarassov (see license.txt)

 String runtime utils
 */
#pragma once

#include <string>

namespace stdext
{
    namespace strutils
    {
        std::string format(const std::string fmt, ...);
        std::wstring format(const std::wstring fmt, ...);
        std::wstring replace_all(std::wstring s, const std::wstring& match, const std::wstring& repl);
        std::string to_string(const std::wstring& ws);
    }
}