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
        class dom_value_ptr
        {
        public:
            dom_value_ptr() = delete;
            dom_value_ptr(json::dom_value* const current);
            dom_value_ptr(const dom_value_ptr&) = default;
            dom_value_ptr& operator =(const dom_value_ptr&) = default;
            dom_value_ptr(dom_value_ptr&&) = default;
            dom_value_ptr& operator =(dom_value_ptr&&) = default;
            ~dom_value_ptr();
        public:
            bool accept();
            json::dom_value* value() const { return m_value; }
            void value(json::dom_value* const value) { m_value = value; }
        private:
            bool m_accepted = false;
            json::dom_value* m_value = nullptr;
        };

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
            class context
            {
            public:
                context() { }
                context(const context&) = default;
                context& operator =(const context&) = default;
                context(context&&) = default;
                context& operator =(context&&) = default;
                ~context() { }
            public:
                std::wstring member_name() const { return m_member_name; }
                void member_name(const std::wstring value) { m_member_name = value; }
            private:
                std::wstring m_member_name;
            };
        public:
            bool run();
            bool has_errors() const { return m_messages.has_errors(); }
            const msg_collector_t& messages() const { return m_messages; }
        private:
            bool accept_value(json::dom_value* const parent, dom_value_ptr& node, const context& ctx);
            void add_error(const parser_msg_kind kind, const parsers::textpos pos);
            void add_error(const parser_msg_kind kind, const parsers::textpos pos, const std::wstring text);
            inline bool eof() const { return m_lexer != nullptr && m_lexer->eof(); }
            bool is_current_token(const json::token tok);
            bool is_parent_container(const json::dom_value* parent);
            bool next();
            bool parse_array(json::dom_value* const parent, const context& ctx);
            bool parse_array_items(json::dom_array* const parent, const context& ctx);
            bool parse_doc();
            bool parse_literal(json::dom_value* const parent, const context& ctx);
            bool parse_number(json::dom_value* const parent, const context& ctx);
            bool parse_object(json::dom_value* const parent, const context& ctx);
            bool parse_object_members(json::dom_object* const parent, const context& ctx);
            bool parse_string(json::dom_value* const parent, const context& ctx);
            bool parse_value(json::dom_value* const parent, const context& ctx);
            inline const parsers::textpos pos() const { return m_curr.pos(); }
        private:
            ioutils::text_reader& m_reader;
            json::lexer* m_lexer = nullptr;
            json::lexeme m_curr;
            msg_collector_t& m_messages;
            json::dom_document& m_doc;
        };
    }
}

