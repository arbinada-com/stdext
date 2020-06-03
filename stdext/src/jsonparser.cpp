/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "jsonparser.h"
#include <locale>
#include <memory>
#include "strutils.h"

using namespace std;
using namespace stdext;
using namespace parsers;

/*
 * dom_value_ptr class
 */
json::dom_value_ptr::dom_value_ptr(json::dom_value* const value)
    : m_value(value)
{ }

json::dom_value_ptr::~dom_value_ptr()
{
    if (!m_accepted && m_value != nullptr)
        delete m_value;
}

bool json::dom_value_ptr::accept()
{
    m_accepted = true;
    return true;
}

/*
 * parser class
 */
json::parser::parser(ioutils::text_reader& reader, msg_collector_t& msgs, json::dom_document& doc)
    : m_reader(reader), m_messages(msgs), m_doc(doc)
{ 
    m_lexer = new json::lexer(m_reader, m_messages);
}

json::parser::~parser() 
{
    delete m_lexer;
}

void json::parser::add_error(const parser_msg_kind kind, const parsers::textpos pos)
{
    add_error(kind, pos, json::to_wmessage(kind));
}

void json::parser::add_error(const parser_msg_kind kind, const parsers::textpos pos, const std::wstring text)
{
    m_messages.add_error(
        msg_origin::parser,
        kind,
        pos,
        m_reader.source_name(),
        text);

}

bool json::parser::is_current_token(const json::token tok)
{
    return m_curr.token() == tok;
}

bool json::parser::is_parent_container(const json::dom_value* parent)
{
    return (parent == nullptr && m_doc.root() == nullptr) || (parent != nullptr && parent->is_container());
}

bool json::parser::accept_value(json::dom_value* const parent, dom_value_ptr& node, const context& ctx)
{
    if (parent == nullptr)
        m_doc.root(node.value());
    else
    {
        switch (parent->type())
        {
        case dom_value_type::vt_array:
        {
            json::dom_array* arr = dynamic_cast<json::dom_array*>(parent);
            if (arr == nullptr)
            {
                add_error(parser_msg_kind::err_parent_is_not_container, pos());
                return false;
            }
            arr->append(node.value());
            node.accept();
            break;
        }
        case dom_value_type::vt_object:
        {
            json::dom_object* obj = dynamic_cast<json::dom_object*>(parent);
            if (obj == nullptr)
            {
                add_error(parser_msg_kind::err_parent_is_not_container, pos());
                return false;
            }
            if (ctx.member_name().empty())
            {
                add_error(parser_msg_kind::err_member_name_is_empty, pos());
                return false;
            }
            if (obj->cmembers()->contains_name(ctx.member_name()))
            {
                add_error(parser_msg_kind::err_member_name_duplicate_fmt, pos(),
                    strutils::wformat(
                        json::to_wmessage(parser_msg_kind::err_member_name_duplicate_fmt).c_str(),
                        m_curr.text().c_str()));

                return false;
            }
            obj->append_member(ctx.member_name(), node.value());
            break;
        }
        default:
            add_error(parser_msg_kind::err_parent_is_not_container, pos());
            return false;
        }
    }
    node.accept();
    return true;
}

bool json::parser::next()
{
    return m_lexer->next_lexeme(m_curr);
}

bool json::parser::run()
{
    return parse_doc();
}

bool json::parser::parse_doc()
{
    m_doc.clear();
    bool result = false;
    if (next())
    {
        context ctx;
        result = parse_value(nullptr, ctx);
    }
    else if (m_lexer->eof() && !has_errors())
        return true;
    if (result && !m_lexer->eof())
    {
        result = !next();
        if (!result)
            add_error(parser_msg_kind::err_unexpected_lexeme_fmt, pos(),
                strutils::wformat(
                    json::to_wmessage(parser_msg_kind::err_unexpected_lexeme_fmt).c_str(),
                    m_curr.text().c_str()));
    }
    return result;
}

bool json::parser::parse_value(json::dom_value* const parent, const context& ctx)
{
    bool result = false;
    switch (m_curr.token())
    {
    case token::begin_array:
        result = parse_array(parent, ctx);
        break;
    case token::begin_object:
        result = parse_object(parent, ctx);
        break;
    case token::literal_false:
    case token::literal_null:
    case token::literal_true:
        result = parse_literal(parent, ctx);
        break;
    case token::number_decimal:
    case token::number_float:
    case token::number_int:
        result = parse_number(parent, ctx);
        break;
    case token::string:
        result = parse_string(parent, ctx);
        break;
    default:
        add_error(parser_msg_kind::err_expected_value_but_found_fmt, pos(),
            strutils::wformat(
                json::to_wmessage(parser_msg_kind::err_expected_value_but_found_fmt).c_str(),
                m_curr.text().c_str()));
        break;
    }
    return result;
}

bool json::parser::parse_array(json::dom_value* const parent, const context& ctx)
{
    bool result = is_current_token(token::begin_array);
    if (!result)
        add_error(parser_msg_kind::err_expected_array, pos());
    else
    {
        dom_value_ptr arr(m_doc.create_array());
        result = accept_value(parent, arr, ctx);
        if (result)
        {
            result = next();
            if (result)
            {
                if (!is_current_token(token::end_array))
                    result = parse_array_items(dynamic_cast<dom_array*>(arr.value()), ctx);
                if (result)
                    result = is_current_token(token::end_array);
            }
        }
        if (!result)
            add_error(parser_msg_kind::err_unclosed_array, m_lexer->pos());
    }
    return result;
}

bool json::parser::parse_array_items(json::dom_array* const parent, const context& ctx)
{
    bool result = true;
    bool is_next_item = true;
    while (result && is_next_item)
    {
        result = parse_value(parent, ctx);
        if (result)
        {
            is_next_item = next() && is_current_token(token::value_separator);
            if (is_next_item)
            {
                result = next();
                if (!result)
                    add_error(parser_msg_kind::err_expected_array_item, m_lexer->pos());
            }
        }
        else
            add_error(parser_msg_kind::err_expected_array_item, pos());
    }
    return result;
}

bool json::parser::parse_object(json::dom_value* const parent, const context& ctx)
{
    bool result = is_current_token(token::begin_object);
    if (!result)
        add_error(parser_msg_kind::err_expected_object, pos());
    else
    {
        dom_value_ptr current(m_doc.create_object());
        result = accept_value(parent, current, ctx);
        if (result)
        {
            result = next();
            if (result)
            {
                if (!is_current_token(token::end_object))
                    result = parse_object_members(dynamic_cast<dom_object*>(current.value()), ctx);
                if (result)
                    result = is_current_token(token::end_object);
            }
        }
        if (!result)
            add_error(parser_msg_kind::err_unclosed_object, m_lexer->pos());
    }
    return result;
}

bool json::parser::parse_object_members(json::dom_object* const parent, const context& ctx)
{
    bool result = true;
    bool is_next_member = true;
    while (result && is_next_member)
    {
        result = is_current_token(token::string);
        if (result)
        {
            context member_ctx(ctx);
            member_ctx.member_name(m_curr.text());
            result = next();
            if (result)
            {
                if (is_current_token(token::name_separator))
                {
                    result = next();
                    if (result && parse_value(parent, member_ctx))
                    {
                        is_next_member = next() && is_current_token(token::value_separator);
                        if (is_next_member)
                        {
                            result = next();
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

bool json::parser::parse_literal(json::dom_value* const parent, const context& ctx)
{
    bool result = is_literal_token(m_curr.token());
    if (result)
    {
        dom_value_ptr node(m_doc.create_literal(m_curr.text()));
        result = accept_value(parent, node, ctx);
    }
    else
        add_error(parser_msg_kind::err_expected_literal, m_curr.pos());
    return result;
}

bool json::parser::parse_number(json::dom_value* const parent, const context& ctx)
{
    switch (m_curr.token())
    {
    case token::number_decimal:
    case token::number_float:
    {
        dom_value_ptr node(m_doc.create_number(m_curr.text(), dom_number_value_type::nvt_float));
        return accept_value(parent, node, ctx);
    }
    case token::number_int: 
    {
        dom_value_ptr node(m_doc.create_number(m_curr.text(), dom_number_value_type::nvt_int));
        return accept_value(parent, node, ctx);
    }
    default:
        add_error(parser_msg_kind::err_expected_number, m_curr.pos());
        return false;
    }
}

bool json::parser::parse_string(json::dom_value* const parent, const context& ctx)
{
    bool result = m_curr.token() == token::string;
    if (result)
    {
        dom_value_ptr node(m_doc.create_string(m_curr.text()));
        result = accept_value(parent, node, ctx);
    }
    else
        add_error(parser_msg_kind::err_expected_string, m_curr.pos());
    return result;
}


