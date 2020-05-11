/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 JSON (JavaScript Object Notation) tools
 Based on RFC 8259 (https://tools.ietf.org/html/rfc8259)
 */
#pragma once

#include "jsonexceptions.h"
#include "jsonlexer.h"
#include "jsondom.h"

namespace stdext
{
    namespace json
    {

        class parser
        {
        public:
            parser() = delete;
            parser(ioutils::text_reader& reader, msg_collector_t& msgs, json::dom_document& doc);
            parser(const parser&) = delete;
            parser& operator =(const parser&) = delete;
            parser(parser&&) = delete;
            parser& operator =(parser&&) = delete;
            ~parser();
        public:
            bool run();
            bool has_errors() const { return m_messages.has_errors(); }
            const msg_collector_t& messages() const { return m_messages; }
        private:
            void add_error(const parser_msg_kind kind, const parsers::textpos pos);
            void add_error(const parser_msg_kind kind, const parsers::textpos pos, const std::wstring text);
            bool append_value(json::dom_value_type vtype, json::lexeme& lex);
            bool parse_doc();
            bool parse_literal(json::lexeme& lex);
        private:
            json::lexer* m_lexer;
            msg_collector_t& m_messages;
            ioutils::text_reader& m_reader;
            json::dom_document& m_doc;
            json::dom_value* m_current = nullptr;
        };
    }
}

