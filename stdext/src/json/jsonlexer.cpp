/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "jsonlexer.h"
#include <locale>
#include "../strutils.h"

using namespace std;

namespace stdext
{
namespace json
{

bool is_number_token(const token value)
{
    return value == token::number_decimal ||
        value == token::number_float ||
        value == token::number_int;
}

bool is_literal_token(const token value)
{
    return value == token::literal_false ||
        value == token::literal_null ||
        value == token::literal_true;
}

bool is_value_token(const token value)
{
    return value == token::begin_array ||
        value == token::begin_object ||
        value == token::string ||
        is_number_token(value) ||
        is_literal_token(value);
}


std::wstring to_wstring(const token tok)
{
    switch (tok)
    {
    case token::unknown: return L"unknown";
    case token::begin_array: return L"begin_array";
    case token::begin_object: return L"begin_object";
    case token::end_array: return L"end_array";
    case token::end_object: return L"end_object";
    case token::literal_false: return L"literal_false";
    case token::literal_null: return L"literal_null";
    case token::literal_true: return L"literal_true";
    case token::name_separator: return L"name_separator";
    case token::number_decimal: return L"number_decimal";
    case token::number_float: return L"number_float";
    case token::number_int: return L"number_int";
    case token::string: return L"string";
    case token::value_separator: return L"value_separator";
    default:
        return L"unsupported";
    }
}


/*
 * lexeme class
 */
void lexeme::reset(const parsers::textpos pos, const json::token tok, const wchar_t text)
{
    wstring s;
    s += text;
    reset(pos, tok, s);
}

void lexeme::reset(const parsers::textpos pos, const json::token tok, const std::wstring text)
{
    m_pos = pos;
    m_token = tok;
    m_text = text;
}

/*
 * lexer class
 */
void lexer::accept_char(lexeme& lex)
{
    lex.inc_text(m_c);
    accept_char();
}

void lexer::accept_char(std::wstring& value)
{
    value += m_c;
    accept_char();
}

void lexer::add_error(const parser_msg_kind kind)
{
    add_error(kind, m_pos, to_wmessage(kind));
}

void lexer::add_error(const parser_msg_kind kind, const parsers::textpos pos)
{
    add_error(kind, pos, to_wmessage(kind));
}

void lexer::add_error(const parser_msg_kind kind, const std::wstring text)
{
    add_error(kind, m_pos, text);
}

void lexer::add_error(const parser_msg_kind kind, const parsers::textpos pos, const std::wstring text)
{
    m_messages.add_error(
        parsers::msg_origin::lexer,
        kind,
        pos,
        m_reader.source_name(),
        text);
}

bool lexer::handle_escaped_char(wchar_t& c, const parsers::textpos start)
{
    std::locale loc;
    wstring value;
    for (int i = 0; i < 4; i++)
    {
        if (!(next_char() && std::isxdigit(m_c, loc)))
        {
            add_error(parser_msg_kind::err_unallowed_escape_seq, start);
            return false;
        }
        value += m_c;
        accept_char();
    }
    int code = std::stoi(value, nullptr, 16);
    c = static_cast<wchar_t>(code);
    return true;
}

bool lexer::handle_literal(lexeme& lex)
{
    parsers::textpos pos = m_pos;
    wstring value;
    accept_char(value);
    while (next_char())
    {
        if (is_whitespace(m_c) || is_structural(m_c))
            break;
        accept_char(value);
    }
    if (value == L"false")
        lex.reset(pos, token::literal_false, value);
    else if (value == L"null")
        lex.reset(pos, token::literal_null, value);
    else if (value == L"true")
        lex.reset(pos, token::literal_true, value);
    else
    {
        add_error(parser_msg_kind::err_invalid_literal_fmt,
            pos,
            str::wformat(
                to_wmessage(parser_msg_kind::err_invalid_literal_fmt),
                value.c_str()));
        return false;
    }
    return true;
}

bool lexer::handle_number(lexeme& lex)
{
    lex.reset(m_pos, token::number_int, m_c);
    parsers::numeric_parser np;
    do
    {
        if (np.read_char(m_c))
            accept_char();
        else
            break;
    } while (next_char());
    json::token tok = token::unknown;
    switch (np.type())
    {
	case parsers::numeric_parser::numeric_type::nt_decimal:
        tok = token::number_decimal;
        break;
    case parsers::numeric_parser::numeric_type::nt_float:
        tok = token::number_float;
        break;
    case parsers::numeric_parser::numeric_type::nt_integer:
        tok = token::number_int;
        break;
    default:
        add_error(parser_msg_kind::err_invalid_number);
        return false;
    }
    if (char_accepted() || is_whitespace(m_c) || is_structural(m_c))
    {
        lex.reset(lex.pos(), tok, np.value());
        return true;
    }
    add_error(parser_msg_kind::err_invalid_number);
    return false;
}

bool lexer::handle_string(lexeme& lex)
{
    wstring value;
    lex.reset(m_pos, token::string, L"");
    while (next_char())
    {
        if (is_unescaped(m_c))
            accept_char(value);
        else if (m_c == L'"')
        {
            lex.text(value);
            accept_char();
            return true;
        }
        else if (is_escape(m_c))
        {
            parsers::textpos pos = m_pos;
            if (!next_char())
                break;
            switch (m_c)
            {
            case  L'"':
            case  L'\\':
            case  L'/':
                accept_char(value);
                break;
            case  L'b':
                value += L'\b';
                accept_char();
                break;
            case  L'f':
                value += L'\f';
                accept_char();
                break;
            case  L'n':
                value += L'\n';
                accept_char();
                break;
            case  L'r':
                value += L'\r';
                accept_char();
                break;
            case  L't':
                value += L'\t';
                accept_char();
                break;
            case  L'u':
                wchar_t c;
                if (!handle_escaped_char(c, pos))
                    return false;
                value += c;
                break;
            default:
                wstring s = L"\\";
                s += m_c;
                add_error(parser_msg_kind::err_unrecognized_escape_seq_fmt,
                    pos,
                    str::wformat(to_wmessage(parser_msg_kind::err_unrecognized_escape_seq_fmt), s.c_str()));
                return false;
            }
        }
        else
        {
            add_error(parser_msg_kind::err_unallowed_char_fmt,
                str::wformat(
                    to_wmessage(parser_msg_kind::err_unallowed_char_fmt),
                    m_c, static_cast<unsigned int>(m_c)));
        }
    }
    add_error(parser_msg_kind::err_unclosed_string);
    return false;
}

bool lexer::is_digit(const wchar_t c)
{
    return c >= L'0' && c <= L'9';
}

bool lexer::is_escape(const wchar_t c)
{
    return c == L'\\';
}

bool lexer::is_structural(const wchar_t c)
{
    return c == L'[' || c == L'{' || c == L']' || c == L'}' || c == L':' || c == L',';
}

bool lexer::is_whitespace(const wchar_t c)
{
    return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n';
}

bool lexer::next_char()
{
    wchar_t prev = m_c;
    if (m_reader.next_char(m_c))
    {
        m_c_accepted = false;
        m_pos++;
        if (prev == L'\n')
            m_pos.newline();
        return true;
    }
    else
    {
        m_c = 0;
        if (!m_reader.eof())
            add_error(parser_msg_kind::err_reader_io);
    }
    return false;
}

bool lexer::next_lexeme(lexeme& lex)
{
    if (char_accepted() || m_initial)
    {
        if (!next_char())
            return false;
    }
    m_initial = false;
    skip_whitespaces();
    if (!char_accepted())
    {
        switch (m_c)
        {
        case L'[':
            lex.reset(m_pos, token::begin_array, m_c);
            accept_char();
            return true;
        case L'{':
            lex.reset(m_pos, token::begin_object, m_c);
            accept_char();
            return true;
        case L']':
            lex.reset(m_pos, token::end_array, m_c);
            accept_char();
            return true;
        case L'}':
            lex.reset(m_pos, token::end_object, m_c);
            accept_char();
            return true;
        case L'"':
            return handle_string(lex);
        case L'f':
        case L'n':
        case L't':
            return handle_literal(lex);
        case L':':
            lex.reset(m_pos, token::name_separator, m_c);
            accept_char();
            return true;
        case L',':
            lex.reset(m_pos, token::value_separator, m_c);
            accept_char();
            return true;
        case L'-':
        case L'0':
        case L'1':
        case L'2':
        case L'3':
        case L'4':
        case L'5':
        case L'6':
        case L'7':
        case L'8':
        case L'9':
            return handle_number(lex);
        default:
            add_error(parser_msg_kind::err_unexpected_char_fmt,
                str::wformat(
                    to_wmessage(parser_msg_kind::err_unexpected_char_fmt),
                    m_c, static_cast<unsigned int>(m_c)));
            break;
        }
    }
    return false;
}

void lexer::skip_whitespaces()
{
    while (is_whitespace(m_c))
    {
        accept_char();
        next_char();
    }
}

}
}
