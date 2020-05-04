/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 JSON (JavaScript Object Notation) tools
 Based on RFC 8259 (https://tools.ietf.org/html/rfc8259)
 */
#pragma once

#include <string>
#include "parsers.h"
#include "ioutils.h"
#include "jsonexceptions.h"

namespace stdext
{
    namespace json
    {
        enum class token
        {
            unknown,
            begin_array,
            begin_object,
            end_array,
            end_object,
            literal_false,
            literal_null,
            literal_true,
            name_separator,
            number_decimal,
            number_float,
            number_int,
            string,
            value_separator
        };

        class lexeme
        {
        public:
            lexeme() { }
            lexeme(const lexeme& source) = default;
            lexeme& operator =(const lexeme& source) = default;
            lexeme(lexeme&& source) = default;
            lexeme& operator =(lexeme&& source) = default;
            ~lexeme() { }
            parsers::textpos pos() const noexcept { return m_pos; }
            void pos(const parsers::textpos& value) noexcept { m_pos = value; }
            json::token token() const noexcept { return m_token; }
            std::wstring text() const noexcept { return m_text; }
            void text(const std::wstring value) noexcept { m_text = value; }
        private:
            parsers::textpos m_pos;
            json::token m_token = token::unknown;
            std::wstring m_text;
        };

        class lexer
        {
        public:
            lexer() = delete;
            lexer(ioutils::text_reader* const reader)
                : m_reader(reader)
            { }
            lexer(const lexer&) = delete;
            lexer& operator =(const lexer&) = delete;
            lexer(lexer&&) = delete;
            lexer& operator =(lexer&&) = delete;
            ~lexer() { }
        public:
            bool next_lexeme(lexeme& lex);
            bool has_error();
        private:
            bool eof();
            bool is_whitespace(const wchar_t c);
            bool next_char();
            void skip_whitespaces();
        private:
            ioutils::text_reader* m_reader;
            wchar_t m_c;
            parsers::textpos m_pos;
        };
    }
}
