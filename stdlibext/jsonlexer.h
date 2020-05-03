/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 JSON (JavaScript Object Notation) tools
 Based on RFC 8259 (https://tools.ietf.org/html/rfc8259)
 */
#pragma once

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
            ioutils::text_reader* m_reader;
        };
    }
}
