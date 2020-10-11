/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "jsonparser.h"
#include <locale>
#include <memory>
#include "../strutils.h"

using namespace std;

namespace stdext
{
namespace json
{

/*
 * dom_value_ptr class
 */
dom_value_ptr::dom_value_ptr(dom_value* const value)
    : m_value(value)
{ }

dom_value_ptr::~dom_value_ptr()
{
    if (!m_accepted && m_value != nullptr)
        delete m_value;
}

bool dom_value_ptr::accept()
{
    m_accepted = true;
    return true;
}

/*
 * sax_parser class
 */
sax_parser::sax_parser(ioutils::text_reader& reader, msg_collector_t& msgs, sax_handler_intf& handler)
    : m_reader(reader), m_messages(msgs), m_handler(handler)
{
    m_lexer = new lexer(m_reader, m_messages);
}

sax_parser::~sax_parser()
{
}

void sax_parser::add_error(const parser_msg_kind kind, const parsers::textpos pos)
{
    add_error(kind, pos, to_wmessage(kind));
}

void sax_parser::add_error(const parser_msg_kind kind, const parsers::textpos pos, const std::wstring text)
{
    m_messages.add_error(
        parsers::msg_origin::parser,
        kind,
        pos,
        m_reader.source_name(),
        text);

}

bool sax_parser::is_current_token(const json::token tok)
{
    return m_curr.token() == tok;
}

bool sax_parser::next_lexeme()
{
    return m_lexer->next_lexeme(m_curr);
}

bool sax_parser::run()
{
    return parse_doc();
}

bool sax_parser::parse_doc()
{
    bool result = false;
    if (next_lexeme())
        result = parse_value();
    else if (m_lexer->eof() && !has_errors())
        return true;
    if (result && !m_lexer->eof())
    {
        result = !next_lexeme();
        if (!result)
            add_error(parser_msg_kind::err_unexpected_lexeme_fmt, pos(),
                str::wformat(
                    to_wmessage(parser_msg_kind::err_unexpected_lexeme_fmt).c_str(),
                    m_curr.text().c_str()));
    }
    return result;
}

bool sax_parser::parse_value()
{
    bool result = false;
    switch (m_curr.token())
    {
    case token::begin_array:
        result = parse_array();
        break;
    case token::begin_object:
        result = parse_object();
        break;
    case token::literal_false:
    case token::literal_null:
    case token::literal_true:
        result = parse_literal();
        break;
    case token::number_decimal:
    case token::number_float:
    case token::number_int:
        result = parse_number();
        break;
    case token::string:
        result = parse_string();
        break;
    default:
        add_error(parser_msg_kind::err_expected_value_but_found_fmt, pos(),
            str::wformat(
                to_wmessage(parser_msg_kind::err_expected_value_but_found_fmt).c_str(),
                m_curr.text().c_str()));
        break;
    }
    return result;
}

bool sax_parser::parse_array()
{
    bool result = is_current_token(token::begin_array);
    if (!result)
        add_error(parser_msg_kind::err_expected_array, pos());
    else
    {
        m_handler.on_begin_array();
        std::size_t element_count = 0;
        result = next_lexeme();
        if (result)
        {
            if (!is_current_token(token::end_array))
                result = parse_array_items(element_count);
            if (result)
                result = is_current_token(token::end_array);
        }
        if (result)
            m_handler.on_end_array(element_count);
        else
            add_error(parser_msg_kind::err_unclosed_array, m_lexer->pos());
    }
    return result;
}

bool sax_parser::parse_array_items(std::size_t& element_count)
{
    bool result = true;
    bool is_next_item = true;
    while (result && is_next_item)
    {
        result = parse_value();
        if (result)
        {
            element_count++;
            is_next_item = next_lexeme() && is_current_token(token::value_separator);
            if (is_next_item)
            {
                result = next_lexeme();
                if (!result)
                    add_error(parser_msg_kind::err_expected_array_item, m_lexer->pos());
            }
        }
        else
            add_error(parser_msg_kind::err_expected_array_item, pos());
    }
    return result;
}

bool sax_parser::parse_object()
{
    bool result = is_current_token(token::begin_object);
    if (!result)
        add_error(parser_msg_kind::err_expected_object, pos());
    else
    {
        m_handler.on_begin_object();
        std::size_t member_count = 0;
        result = next_lexeme();
        if (result)
        {
            if (!is_current_token(token::end_object))
                result = parse_object_members(member_count);
            if (result)
                result = is_current_token(token::end_object);
        }
        if (result)
            m_handler.on_end_object(member_count);
        else
            add_error(parser_msg_kind::err_unclosed_object, m_lexer->pos());
    }
    return result;
}

bool sax_parser::parse_object_members(std::size_t& member_count)
{
    bool result = true;
    bool is_next_member = true;
    while (result && is_next_member)
    {
        result = is_current_token(token::string);
        if (result)
        {
            m_handler.on_member_name(m_curr.text());
            member_count++;
            result = next_lexeme();
            if (result)
            {
                if (is_current_token(token::name_separator))
                {
                    result = next_lexeme();
                    if (result && parse_value())
                    {
                        is_next_member = next_lexeme() && is_current_token(token::value_separator);
                        if (is_next_member)
                        {
                            result = next_lexeme();
                            if (!result)
                                add_error(parser_msg_kind::err_expected_member_name, m_lexer->pos());
                        }
                    }
                    else
                        add_error(parser_msg_kind::err_expected_value, pos());
                }
                else
                    add_error(parser_msg_kind::err_expected_name_separator, pos());
            }
            else
                add_error(parser_msg_kind::err_expected_name_separator, m_lexer->pos());
        }
        else
            add_error(parser_msg_kind::err_expected_member_name, pos());
    }
    return result;
}

bool sax_parser::parse_literal()
{
    bool result = is_literal_token(m_curr.token());
    if (result)
        m_handler.on_literal(to_literal_type(m_curr.text()), m_curr.text());
    else
        add_error(parser_msg_kind::err_expected_literal, m_curr.pos());
    return result;
}

bool sax_parser::parse_number()
{
    switch (m_curr.token())
    {
    case token::number_decimal:
    case token::number_float:
        m_handler.on_number(dom_number_type::nvt_float, m_curr.text());
        return true;
    case token::number_int:
        m_handler.on_number(dom_number_type::nvt_int, m_curr.text());
        return true;
    default:
        add_error(parser_msg_kind::err_expected_number, m_curr.pos());
        return false;
    }
}

bool sax_parser::parse_string()
{
    bool result = m_curr.token() == token::string;
    if (result)
        m_handler.on_string(m_curr.text());
    else
        add_error(parser_msg_kind::err_expected_string, m_curr.pos());
    return result;
}


/*
 * DOM parser handler
 */
void dom_handler::on_literal(const dom_literal_type, const std::wstring& text)
{
    dom_value_ptr node(m_doc.create_literal(text));
    accept_value(node);
}

void dom_handler::on_number(const dom_number_type type, const std::wstring& text)
{
    dom_value_ptr node(m_doc.create_number(text, type));
    accept_value(node);
}

void dom_handler::on_string(const std::wstring& text)
{
    dom_value_ptr node(m_doc.create_string(text));
    accept_value(node);
}

void dom_handler::on_begin_object()
{
    dom_value_ptr node(m_doc.create_object());
    if (accept_value(node))
        m_containers.push(node.value());
}

void dom_handler::on_member_name(const std::wstring& text)
{
    m_member_names.push(text);
}

void dom_handler::on_end_object(const std::size_t)
{
    m_containers.pop();
}

void dom_handler::on_begin_array()
{
    dom_value_ptr node(m_doc.create_array());
    if (accept_value(node))
        m_containers.push(node.value());
}

void dom_handler::on_end_array(const std::size_t)
{
    m_containers.pop();
}

void dom_handler::add_error(const parser_msg_kind kind)
{
    add_error(kind, to_wmessage(kind));
}

void dom_handler::add_error(const parser_msg_kind kind, const std::wstring text)
{
    m_messages.add_error(parsers::msg_origin::parser, kind, m_pos, m_source_name, text);

}

bool dom_handler::accept_value(dom_value_ptr& node)
{
    if (m_containers.empty())
    {
        if (m_doc.root() == nullptr)
        {
            m_doc.root(node.value());
            node.accept();
            return true;
        }
        else
        {
            add_error(parser_msg_kind::err_parent_is_not_container);
            return false;
        }
    }
    switch (m_containers.top()->type())
    {
    case dom_value_type::vt_array:
    {
        dom_array* arr = dynamic_cast<dom_array*>(m_containers.top());
        if (arr == nullptr)
        {
            add_error(parser_msg_kind::err_parent_is_not_container);
            return false;
        }
        arr->append(node.value());
        node.accept();
        break;
    }
    case dom_value_type::vt_object:
    {
        dom_object* obj = dynamic_cast<dom_object*>(m_containers.top());
        if (obj == nullptr)
        {
            add_error(parser_msg_kind::err_parent_is_not_container);
            return false;
        }
        if (m_member_names.empty() || m_member_names.top().empty())
        {
            add_error(parser_msg_kind::err_member_name_is_empty);
            return false;
        }
        if (obj->cmembers()->contains_name(m_member_names.top()))
        {
            add_error(parser_msg_kind::err_member_name_duplicate_fmt,
                str::wformat(
                    to_wmessage(parser_msg_kind::err_member_name_duplicate_fmt).c_str(),
                    m_member_names.top().c_str()));

            return false;
        }
        obj->append_member(m_member_names.top(), node.value());
        m_member_names.pop();
        break;
    }
    default:
        add_error(parser_msg_kind::err_parent_is_not_container);
        return false;
    }
    node.accept();
    return true;
}


/*
 * DOM parser class
 */
dom_parser::dom_parser(ioutils::text_reader& reader, msg_collector_t& msgs, dom_document& doc)
    : m_reader(reader), m_messages(msgs), m_doc(doc)
{}

bool dom_parser::run()
{
    m_doc.clear();
    dom_handler handler(m_doc, m_messages, m_reader.source_name());
    sax_parser parser(m_reader, m_messages, handler);
    return parser.run();
}

}
}
