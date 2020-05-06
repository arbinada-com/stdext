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
#include "jsoncommon.h"
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
        std::wstring to_wstring(const json::token tok);

        class lexeme
        {
        public:
            lexeme() { }
            lexeme(const json::token token, const parsers::textpos pos, const std::wstring text)
                : m_token(token), m_pos(pos), m_text(text)
            { }
            lexeme(const lexeme& source) = default;
            lexeme& operator =(const lexeme& source) = default;
            lexeme(lexeme&& source) = default;
            lexeme& operator =(lexeme&& source) = default;
            ~lexeme() { }
        public:
            parsers::textpos pos() const { return m_pos; }
            void pos(const parsers::textpos& value) { m_pos = value; }
            void reset(const parsers::textpos pos, const json::token tok, const wchar_t text);
            void reset(const parsers::textpos pos, const json::token tok, const std::wstring text);
            json::token token() const noexcept { return m_token; }
            void token(const json::token value) noexcept { m_token = value; }
            std::wstring text() const { return m_text; }
            void text(const std::wstring value) { m_text = value; }
        private:
            parsers::textpos m_pos;
            json::token m_token = token::unknown;
            std::wstring m_text;
        };

        class lexer
        {
        public:
            lexer() = delete;
            lexer(ioutils::text_reader* const reader, msg_collector_t* const msgs)
                : m_reader(reader), m_messages(msgs)
            { }
            lexer(const lexer&) = delete;
            lexer& operator =(const lexer&) = delete;
            lexer(lexer&&) = delete;
            lexer& operator =(lexer&&) = delete;
            ~lexer() { }
        public:
            bool eof();
            bool next_lexeme(lexeme& lex);
            bool has_errors() const { return m_messages->has_errors(); }
            msg_collector_t* const messages() const { return m_messages; }
        private:
            void add_error(const parser_msg_kind kind);
            void add_error(const parser_msg_kind kind, parsers::textpos pos);
            void add_error(const parser_msg_kind kind, const std::wstring text);
            void add_error(const parser_msg_kind kind, parsers::textpos pos, const std::wstring text);
            bool handle_escaped_char(wchar_t& c);
            bool handle_literal(lexeme& lex);
            bool handle_string(lexeme& lex);
            bool is_escape(const wchar_t c);
            bool is_structural(const wchar_t c);
            bool is_unescaped(const wchar_t c);
            bool is_whitespace(const wchar_t c);
            bool next_char();
            void skip_whitespaces();
        private:
            msg_collector_t* m_messages = nullptr;
            ioutils::text_reader* m_reader = nullptr;
            wchar_t m_c = 0;
            bool m_look_ahead = false;
            parsers::textpos m_pos{ 1, 0 };
        };
    }
}
