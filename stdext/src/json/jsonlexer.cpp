/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "jsonlexer.h"
#include <locale>
#include "../strutils.h"

using namespace std;
using namespace stdext;
using namespace parsers;

bool json::is_number_token(const json::token value)
{
    return value == token::number_decimal ||
        value == token::number_float ||
        value == token::number_int;
}

bool json::is_literal_token(const json::token value)
{
    return value == token::literal_false ||
        value == token::literal_null ||
        value == token::literal_true;
}

bool json::is_value_token(const json::token value)
{
    return value == token::begin_array ||
        value == token::begin_object ||
        value == token::string ||
        is_number_token(value) ||
        is_literal_token(value);
}


std::wstring json::to_wstring(const json::token tok)
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
void json::lexeme::reset(const parsers::textpos pos, const json::token tok, const wchar_t text)
{
    wstring s;
    s += text;
    reset(pos, tok, s);
}

void json::lexeme::reset(const parsers::textpos pos, const json::token tok, const std::wstring text)
{
    m_pos = pos;
    m_token = tok;
    m_text = text;
}

/*
 * lexer class
 */
void json::lexer::add_error(const parser_msg_kind kind)
{
    add_error(kind, m_pos, json::to_wmessage(kind));
}

void json::lexer::add_error(const parser_msg_kind kind, const parsers::textpos pos)
{
    add_error(kind, pos, json::to_wmessage(kind));
}

void json::lexer::add_error(const parser_msg_kind kind, const std::wstring text)
{
    add_error(kind, m_pos, text);
}

void json::lexer::add_error(const parser_msg_kind kind, const parsers::textpos pos, const std::wstring text)
{
    m_messages.add_error(
        msg_origin::lexer, 
        kind, 
        pos, 
        m_reader.source_name(), 
        text);
}

bool json::lexer::handle_escaped_char(wchar_t& c, const parsers::textpos start)
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
    }
    int code = std::stoi(value, nullptr, 16);
    c = static_cast<wchar_t>(code);
    return true;
}

bool json::lexer::handle_literal(lexeme& lex)
{
    m_look_ahead = true;
    textpos pos = m_pos;
    wstring value;
    do 
    {
        value += m_c;
        if (!next_char())
        {
            if (!eof())
                return false;
        }
    } while (!(eof() || is_whitespace(m_c) || is_structural(m_c)));
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
            strutils::wformat(
                json::to_wmessage(parser_msg_kind::err_invalid_literal_fmt),
                value.c_str()));
        return false;
    }
    return true;
}

bool json::lexer::handle_number(lexeme& lex)
{
    m_look_ahead = true;
    lex.reset(m_pos, token::number_int, m_c);
    int int_digit_count = m_c == L'-' ? 0 : 1;
    wchar_t prev = m_c;
    while (!eof() && next_char())
    {
        if (!is_digit(m_c))
            break;
        if (int_digit_count == 1 && prev == L'0' && m_c == L'0')
        {
            add_error(parser_msg_kind::err_invalid_number);
            return false;
        }
        int_digit_count++;
        lex.inc_text(m_c);
        prev = m_c;
    }
    if (int_digit_count == 0)
    {
        add_error(parser_msg_kind::err_invalid_number);
        return false;
    }
    if (m_c == L'.')
    {
        lex.token(token::number_decimal);
        lex.inc_text(m_c);
        int frac_digit_count = 0;
        while (!eof() && next_char())
        {
            if (!is_digit(m_c))
                break;
            frac_digit_count++;
            lex.inc_text(m_c);
        }
        if (frac_digit_count == 0)
        {
            add_error(parser_msg_kind::err_invalid_number);
            return false;
        }
    }
    if (m_c == L'e' || m_c == L'E')
    {
        lex.token(token::number_float);
        lex.inc_text(m_c);
        int exp_digit_count = 0;
        while (!eof() && next_char())
        {
            if (!is_digit(m_c))
            {
                if (!((m_c == L'-' || m_c == L'+') && exp_digit_count == 0))
                    break;
            }
            else
                exp_digit_count++;
            lex.inc_text(m_c);
        }
        if (exp_digit_count == 0)
        {
            add_error(parser_msg_kind::err_invalid_number);
            return false;
        }
    }
    if (eof() || is_whitespace(m_c) || is_structural(m_c))
        return true;
    add_error(parser_msg_kind::err_invalid_number);
    return false;
}

bool json::lexer::handle_string(lexeme& lex)
{
    wstring value;
    lex.reset(m_pos, token::string, L"");
    while (!eof())
    {
        if (!next_char())
            break;
        if (is_unescaped(m_c))
            value += m_c;
        else if (m_c == L'"')
        {
            lex.text(value);
            return true;
        }
        else if (is_escape(m_c))
        {
            textpos pos = m_pos;
            if (!next_char())
                break;
            switch (m_c)
            {
            case  L'"':
            case  L'\\':
            case  L'/':
                value += m_c;
                break;
            case  L'b':
                value += L'\b';
                break;
            case  L'f':
                value += L'\f';
                break;
            case  L'n':
                value += L'\n';
                break;
            case  L'r':
                value += L'\r';
                break;
            case  L't':
                value += L'\t';
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
                    strutils::wformat(json::to_wmessage(parser_msg_kind::err_unrecognized_escape_seq_fmt), s.c_str()));
                return false;
            }
        }
        else
        {
            add_error(parser_msg_kind::err_unallowed_char_fmt,
                strutils::wformat(
                    json::to_wmessage(parser_msg_kind::err_unallowed_char_fmt),
                    m_c, static_cast<unsigned int>(m_c)));
        }
    }
    add_error(parser_msg_kind::err_unclosed_string);
    return false;
}

bool json::lexer::is_digit(const wchar_t c)
{
    return c >= L'0' && c <= L'9';
}

bool json::lexer::is_escape(const wchar_t c)
{
    return c == L'\\';
}

bool json::lexer::is_structural(const wchar_t c)
{
    return c == L'[' || c == L'{' || c == L']' || c == L'}' || c == L':' || c == L',';
}

bool json::lexer::is_unescaped(const wchar_t c)
{
    return json::is_unescaped(c);
}

bool json::lexer::is_whitespace(const wchar_t c)
{
    return c == L' ' || c == L'\t' || c == L'\r' || c == L'\n';
}

bool json::lexer::next_char()
{
    wchar_t prev = m_c;
    if (m_reader.next_char(m_c))
    {
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

bool json::lexer::next_lexeme(lexeme& lex)
{
    if (!m_look_ahead)
    {
        if (!next_char())
            return false;
    }
    m_look_ahead = false;
    skip_whitespaces();
    if (!eof())
    {
        switch (m_c)
        {
        case L'[':
            lex.reset(m_pos, token::begin_array, m_c);
            return true;
        case L'{':
            lex.reset(m_pos, token::begin_object, m_c);
            return true;
        case L']':
            lex.reset(m_pos, token::end_array, m_c);
            return true;
        case L'}':
            lex.reset(m_pos, token::end_object, m_c);
            return true;
        case L'"':
            return handle_string(lex);
        case L'f':
        case L'n':
        case L't':
            return handle_literal(lex);
        case L':':
            lex.reset(m_pos, token::name_separator, m_c);
            return true;
        case L',':
            lex.reset(m_pos, token::value_separator, m_c);
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
                strutils::wformat(
                    json::to_wmessage(parser_msg_kind::err_unexpected_char_fmt),
                    m_c, static_cast<unsigned int>(m_c)));
            break;
        }
    }
    return false;
}

void json::lexer::skip_whitespaces()
{
    while (!eof() && is_whitespace(m_c))
        next_char();
}
