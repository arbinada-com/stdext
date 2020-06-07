/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 JSON (JavaScript Object Notation) tools
 Based on RFC 8259 (https://tools.ietf.org/html/rfc8259)
 */

#pragma once
#include <exception>
#include "../strutils.h"

namespace stdext
{
    namespace json
    {
        class exception : public std::exception
        {
        public:
            exception(const std::wstring msg)
                : std::exception(), m_wmesssage(msg)
            { }
            std::wstring wmessage() const noexcept { return m_wmesssage; }
            const char* what() const noexcept override { return strutils::to_string(m_wmesssage).c_str(); }
        protected:
            std::wstring m_wmesssage;
        };

    }
}

