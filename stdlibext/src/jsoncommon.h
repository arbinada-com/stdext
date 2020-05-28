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
            err_expected_array = 2100,
            err_expected_array_item = 2105,
            err_expected_literal = 2110,
            err_expected_member_name = 2112,
            err_expected_name_separator = 2014,
            err_expected_number = 2116,
            err_expected_object = 2120,
            err_expected_string = 2125,
            err_expected_value = 2150,
            err_expected_value_but_found_fmt = 2155,
            err_member_name_duplicate_fmt = 2200,
            err_member_name_is_empty = 2205,
            err_parent_is_not_container = 2250,
            err_unclosed_array = 2290,
            err_unclosed_object = 2295,
            err_unexpected_lexeme_fmt = 2300,
            err_unexpected_text_end = 2310,
            err_unsupported_dom_value_type_fmt = 2400
        };
        std::wstring to_wmessage(const parser_msg_kind kind);

        typedef parsers::message<json::parser_msg_kind> message_t;
        typedef parsers::msg_collector<json::parser_msg_kind> msg_collector_t;

        bool is_unescaped(const wchar_t c);
        std::wstring to_escaped(const wchar_t c, const bool force_to_numeric = false);
    }
}
