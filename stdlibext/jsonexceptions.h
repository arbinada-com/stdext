/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 JSON (JavaScrip Object Noration) tools
 Based on RFC 8259 (https://tools.ietf.org/html/rfc8259)
 */

#pragma once
#include <exception>
#include "strutils.h"

namespace stdext
{
    namespace json
    {
        class exception : public std::exception
        {
        public:
            exception(const std::wstring msg)
                : std::exception(strutils::to_string(msg).c_str()), m_wmesssage(msg)
            { }
            std::wstring wmessage() const noexcept { return m_wmesssage; }
        protected:
            std::wstring m_wmesssage;
        };

    }
}

