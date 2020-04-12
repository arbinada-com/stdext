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

        std::string ltrim(const std::string& s);
        std::wstring ltrim(const std::wstring& s);
        std::string ltrim_chars(const std::string& s, const std::string& chars);
        std::wstring ltrim_chars(const std::wstring& s, const std::wstring& chars);
        std::string rtrim(const std::string& s);
        std::wstring rtrim(const std::wstring& s);
        std::string rtrim_chars(const std::string& s, const std::string& chars);
        std::wstring rtrim_chars(const std::wstring& s, const std::wstring& chars);
        std::string trim(const std::string& s);
        std::wstring trim(const std::wstring& s);
        std::string trim_chars(const std::string& s, const std::string& chars);
        std::wstring trim_chars(const std::wstring& s, const std::wstring& chars);

        std::wstring replace_all(std::wstring s, const std::wstring& match, const std::wstring& repl);
        std::string to_string(const std::wstring& ws);
    }
}