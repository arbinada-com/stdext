/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "jsoncommon.h"
#include "strutils.h"

using namespace std;
using namespace stdext;

std::wstring json::to_wmessage(const parser_msg_kind kind)
{
    switch (kind)
    {
        // lexer
    case parser_msg_kind::err_invalid_literal_fmt: return L"Invalid literal '%s'. Expected 'false', 'true' or 'null'";
    case parser_msg_kind::err_invalid_number: return L"Invalid number";
    case parser_msg_kind::err_reader_io: return L"Text reader I/O error";
    case parser_msg_kind::err_unallowed_char_fmt: return L"Unallowed character: %c (0x%x)";
    case parser_msg_kind::err_unallowed_escape_seq: return L"Invalid escape sequence. Expected '\\u' terminated with 4 hexadecimal digits '\\uXXXX'";
    case parser_msg_kind::err_unclosed_string: return L"Unclosed string";
    case parser_msg_kind::err_unexpected_char_fmt: return L"Unexpected character: %c (0x%x)";
    case parser_msg_kind::err_unrecognized_escape_seq_fmt: return L"Unrecognized character escape sequence: %s";
        // parser
    default:
        return strutils::format(L"Unsupported message %d", static_cast<int>(kind));
    }
}
