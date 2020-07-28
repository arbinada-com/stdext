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
                : std::exception(),
                  m_wmesssage(msg),
                  m_messsage(strutils::to_string(msg))
            {}
        public:
            std::wstring wmessage() const noexcept { return m_wmesssage; }
            const char* what() const noexcept override { return m_messsage.data(); }
        protected:
            std::wstring m_wmesssage;
            std::string m_messsage;
        };

    }
}

