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

bool json::parser::run()
{
    bool result = parse_doc();
    return result;
}

bool json::parser::parse_doc()
{
    m_doc.clear();
    m_current = nullptr;
    bool result = false;
    json::lexeme lex;
    if (m_lexer->next_lexeme(lex))
    {
        switch (lex.token())
        {
        case token::begin_array:
            break;
        case token::begin_object:
            break;
        case token::literal_false:
        case token::literal_null:
        case token::literal_true:
            result = parse_literal(lex);
            break;
        case token::number_decimal:
        case token::number_float:
        case token::number_int:
            result = parse_number(lex);
            break;
        case token::string:
            break;
        default:
            add_error(parser_msg_kind::err_unexpected_lexeme_fmt, lex.pos(),
                strutils::format(
                    json::to_wmessage(parser_msg_kind::err_unexpected_lexeme_fmt).c_str(), 
                    json::to_wstring(lex.token()).c_str() ) );
            break;
        }
    }
    else if (m_lexer->eof() && !has_errors())
        return true;
    if (result && !m_lexer->eof())
    {
        result = !m_lexer->next_lexeme(lex);
        if (!result)
        {
            add_error(parser_msg_kind::err_unexpected_lexeme_fmt, lex.pos(),
                strutils::format(
                    json::to_wmessage(parser_msg_kind::err_unexpected_lexeme_fmt).c_str(),
                    json::to_wstring(lex.token()).c_str()));
        }
    }
    return result;
}

bool json::parser::append_value(json::dom_value* value)
{
        //add_error(parser_msg_kind::err_unsupported_dom_value_type_fmt,
        //    lex.pos(),
        //    strutils::format(
        //        json::to_wmessage(parser_msg_kind::err_unsupported_dom_value_type_fmt), 
        //        json::to_wstring(vtype)));
        //return false;
    if (m_current == nullptr)
    {
        m_current = value;
        m_doc.root(m_current);
    }
    else
    {
        switch (m_current->type())
        {
        case dom_value_type::vt_array:
            dynamic_cast<json::dom_array*>(m_current)->append(value);
            break;
        default:
            return false;
        }
    }
    return true;
}

bool json::parser::parse_literal(json::lexeme& lex)
{
    unique_ptr<json::dom_literal> value;
    if (is_literal_token(lex.token()))
        value.reset(m_doc.create_literal(lex.text()));
    else
    {
        add_error(parser_msg_kind::err_expected_literal, lex.pos());
        return false;
    }
    append_value(value.release());
    return true;
}

bool json::parser::parse_number(json::lexeme& lex)
{
    unique_ptr<json::dom_number> value;
    switch (lex.token())
    {
    case token::number_decimal:
    case token::number_float:
        value.reset(m_doc.create_number(lex.text(), dom_number_value_type::nvt_float));
        break;
    case token::number_int: 
        value.reset(m_doc.create_number(lex.text(), dom_number_value_type::nvt_int));
        break;
    default:
        add_error(parser_msg_kind::err_expected_number, lex.pos());
        return false;
    }
    append_value(value.release());
    return true;
}

