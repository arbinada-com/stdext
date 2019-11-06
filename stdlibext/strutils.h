#pragma once

#include <string>

namespace stdext
{
    namespace strutils
    {
        std::string format(const std::string fmt, ...);
        std::wstring format(const std::wstring fmt, ...);

        std::string to_string(const std::wstring& ws);
    }
}