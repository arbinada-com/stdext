/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 JSON (JavaScript Object Notation) tools
 Based on RFC 8259 (https://tools.ietf.org/html/rfc8259)
 */
#pragma once

#include <stack>
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


        class sax_handler_intf
        {
        public:
            virtual void on_literal(const json::dom_literal_type type, const std::wstring& text) = 0;
            virtual void on_number(const json::dom_number_type type, const std::wstring& text) = 0;
            virtual void on_string(const std::wstring& text) = 0;
            virtual void on_begin_object() = 0;
            virtual void on_member_name(const std::wstring& text) = 0;
            virtual void on_end_object(const std::size_t member_count) = 0;
            virtual void on_begin_array() = 0;
            virtual void on_end_array(const std::size_t element_count) = 0;
            virtual void textpos_changed(const parsers::textpos& pos) = 0;
        };


        class sax_parser
        {
        public:
            sax_parser() = delete;
            sax_parser(ioutils::text_reader& reader, msg_collector_t& msgs, json::sax_handler_intf& handler);
            sax_parser(const sax_parser&) = delete;
            sax_parser& operator =(const sax_parser&) = delete;
            sax_parser(sax_parser&&) = delete;
            sax_parser& operator =(sax_parser&&) = delete;
            ~sax_parser();
        public:
            bool run();
            bool has_errors() const { return m_messages.has_errors(); }
            const msg_collector_t& messages() const { return m_messages; }
        private:
            void add_error(const parser_msg_kind kind, const parsers::textpos pos);
            void add_error(const parser_msg_kind kind, const parsers::textpos pos, const std::wstring text);
            inline bool eof() const { return m_lexer != nullptr && m_lexer->eof(); }
            bool is_current_token(const json::token tok);
            bool next_lexeme();
            bool parse_array();
            bool parse_array_items(std::size_t& element_count);
            bool parse_doc();
            bool parse_literal();
            bool parse_number();
            bool parse_object();
            bool parse_object_members(std::size_t& member_count);
            bool parse_string();
            bool parse_value();
            inline const parsers::textpos pos() const { return m_curr.pos(); }
        private:
            ioutils::text_reader& m_reader;
            json::lexer* m_lexer = nullptr;
            json::lexeme m_curr;
            msg_collector_t& m_messages;
            json::sax_handler_intf& m_handler;
        };


        class dom_handler : public sax_handler_intf
        {
        public:
            typedef std::stack<json::dom_value*> containers_t;
            typedef std::stack<std::wstring> member_names_t;
        public:
            dom_handler(json::dom_document& doc, msg_collector_t& msgs, const std::wstring& source_name)
                : m_doc(doc), m_messages(msgs), m_source_name(source_name)
            {}
        public:
            virtual void on_literal(const json::dom_literal_type type, const std::wstring& text) override;
            virtual void on_number(const json::dom_number_type type, const std::wstring& text) override;
            virtual void on_string(const std::wstring& text) override;
            virtual void on_begin_object() override;
            virtual void on_member_name(const std::wstring& text) override;
            virtual void on_end_object(const std::size_t member_count) override;
            virtual void on_begin_array() override;
            virtual void on_end_array(const std::size_t element_count) override;
            virtual void textpos_changed(const parsers::textpos& pos) override { m_pos = pos; }
        private:
            bool accept_value(dom_value_ptr& node);
            void add_error(const parser_msg_kind kind);
            void add_error(const parser_msg_kind kind, const std::wstring text);
        private:
            json::dom_document& m_doc;
            containers_t m_containers;
            member_names_t m_member_names;
            msg_collector_t& m_messages;
            std::wstring m_source_name;
            parsers::textpos m_pos;
        };

        class dom_parser
        {
        public:
            dom_parser() = delete;
            dom_parser(ioutils::text_reader& reader, msg_collector_t& msgs, json::dom_document& doc);
            dom_parser(const dom_parser&) = delete;
            dom_parser& operator =(const dom_parser&) = delete;
            dom_parser(dom_parser&&) = delete;
            dom_parser& operator =(dom_parser&&) = delete;
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
            ioutils::text_reader& m_reader;
            msg_collector_t& m_messages;
            json::dom_document& m_doc;
        };
    }
}

