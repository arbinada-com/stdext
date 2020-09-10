/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "jsoncommon.h"
#include "../strutils.h"
#include "../locutils.h"

using namespace std;

namespace stdext
{
namespace json
{

bool is_unescaped(const wchar_t c)
{
    return !locutils::utf16::is_noncharacter(c) &&
        (
            c == 0x20 || c == 0x21 ||
            (c >= 0x23 && c <= 0x5B) ||
            c >= 0x5D // 0x10FFFF upper limit is for UTF-32
                      // UTF-16 surrogate pair is encoded as two separated characters
        );
}

std::wstring to_escaped(const wchar_t c, const bool force_to_numeric)
{
    if (force_to_numeric || !is_unescaped(c))
    {
        switch (c)
        {
        case L'\"': return L"\\\"";
        case L'\\': return L"\\\\";
        case L'\b': return L"\\b";
        case L'\f': return L"\\f";
        case L'\n': return L"\\n";
        case L'\r': return L"\\r";
        case L'\t': return L"\\t";
        default:
            return strutils::wformat(L"\\u%0.4X", c);
        }
    }
    return wstring{c};
}

std::wstring to_escaped(const std::wstring ws, const bool force_to_numeric)
{
    wstring ws2;
    ws2.reserve(ws.length() * (force_to_numeric ? 6 : 2));
    for (const wchar_t c : ws)
    {
        if (force_to_numeric || !is_unescaped(c))
            ws2 += to_escaped(c, force_to_numeric);
        else
            ws2 += c;
    }
    return ws2;
}

std::wstring to_unescaped(const std::wstring ws)
{
    std::locale loc;
    wstring ws2;
    ws2.reserve(ws.length());
    for (size_t i = 0; i < ws.length(); i++)
    {
        wchar_t c = ws[i];
        if (c == L'\\' && i + 5 < ws.length() && ws[i + 1] == L'u')
        {
            wstring value;
            for (size_t j = 2; j < 6; j++)
            {
                if (!std::isxdigit(ws[i + j], loc))
                    break;
                value += ws[i + j];
            }
            if (value.length() == 4)
            {
                c = static_cast<wchar_t>(std::stoi(value, nullptr, 16));
                i += 5;
            }
        }
        ws2 += c;
    }
    return ws2;
}

std::wstring to_wmessage(const json::parser_msg_kind kind)
{
    using namespace json;
    switch (kind)
    {
    // lexer
    case parser_msg_kind::err_invalid_literal_fmt: return L"Invalid literal '%ls'. Expected 'false', 'true' or 'null'";
    case parser_msg_kind::err_invalid_number: return L"Invalid number";
    case parser_msg_kind::err_reader_io: return L"Text reader I/O error";
    case parser_msg_kind::err_unallowed_char_fmt: return L"Unallowed character: %c (0x%x)";
    case parser_msg_kind::err_unallowed_escape_seq: return L"Invalid escape sequence. Expected '\\u' terminated with 4 hexadecimal digits '\\uXXXX'";
    case parser_msg_kind::err_unclosed_string: return L"Unclosed string";
    case parser_msg_kind::err_unexpected_char_fmt: return L"Unexpected character: %c (0x%x)";
    case parser_msg_kind::err_unrecognized_escape_seq_fmt: return L"Unrecognized character escape sequence: %ls";
    // parser
    case parser_msg_kind::err_expected_array: return L"Array expected";
    case parser_msg_kind::err_expected_array_item: return L"Array item expected";
    case parser_msg_kind::err_expected_literal: return L"Literal expected";
    case parser_msg_kind::err_expected_member_name: return L"Object member name expected";
    case parser_msg_kind::err_expected_name_separator: return L"Name separator ':' expected";
    case parser_msg_kind::err_expected_number: return L"Number expected";
    case parser_msg_kind::err_expected_object: return L"Object expected";
    case parser_msg_kind::err_expected_string: return L"String expected";
    case parser_msg_kind::err_expected_value: return L"Expected value";
    case parser_msg_kind::err_expected_value_but_found_fmt: return L"Expected value but '%ls' found";
    case parser_msg_kind::err_member_name_duplicate_fmt: return L"Duplicate member name '%ls'";
    case parser_msg_kind::err_member_name_is_empty: return L"Member name is empty";
    case parser_msg_kind::err_parent_is_not_container: return L"Parent DOM value is not container";
    case parser_msg_kind::err_unclosed_array: return L"Unclosed array";
    case parser_msg_kind::err_unclosed_object: return L"Unclosed object";
    case parser_msg_kind::err_unexpected_lexeme_fmt: return L"Unexpected '%ls'";
    case parser_msg_kind::err_unexpected_text_end: return L"Unexpected end of text";
    case parser_msg_kind::err_unsupported_dom_value_type_fmt: return L"Unsupported DOM value type: %ls";
    default:
        return strutils::wformat(L"Unsupported message %d", static_cast<int>(kind));
    }
}

}
}
