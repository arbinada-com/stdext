/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 JSON (JavaScript Object Notation) tools
 Based on RFC 8259 (https://tools.ietf.org/html/rfc8259)
 */
#pragma once

#include <string>
#include "parsers.h"

namespace stdext
{
    namespace json
    {

        enum class parser_msg_kind : int
        {
            // lexer
            err_invalid_literal_fmt = 1010,
            err_invalid_number = 1012,
            err_reader_io = 1020,
            err_unallowed_char_fmt = 1030,
            err_unallowed_escape_seq = 1040,
            err_unclosed_string = 1050,
            err_unexpected_char_fmt = 1060,
            err_unrecognized_escape_seq_fmt = 1070,
            // parser
            err_unexpected_char = 2010,
        };
        std::wstring to_wmessage(const parser_msg_kind kind);

        typedef parsers::message<json::parser_msg_kind> message_t;
        typedef parsers::msg_collector<json::parser_msg_kind> msg_collector_t;


    }
}
