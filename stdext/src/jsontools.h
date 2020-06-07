/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 JSON (JavaScript Object Notation) tools
 Based on RFC 8259 (https://tools.ietf.org/html/rfc8259)
 */
#pragma once

#include <string>
#include <vector>
#include <limits>
#include "jsoncommon.h"
#include "jsondom.h"
#include "locutils.h"
#include "ioutils.h"
#include "testutils.h"

namespace stdext
{
    namespace json
    {
        class dom_document_writer
        {
        public:
            dom_document_writer(const json::dom_document& doc);
            dom_document_writer() = delete;
            dom_document_writer(const dom_document_writer&) = delete;
            dom_document_writer& operator=(const dom_document_writer&) = delete;
            dom_document_writer(dom_document_writer&&) = delete;
            dom_document_writer& operator=(dom_document_writer&&) = delete;
        public:
            class config
            {
                friend class dom_document_writer;
            public:
                config() {}
            public:
                bool pretty_print() const noexcept { return m_pretty_print; }
                void pretty_print(const bool value) { m_pretty_print = value; }
            private:
                bool m_pretty_print = false;
            };
        public:
            config& conf() { return m_conf; }
            std::wstring escape(const std::wstring s) const;
            void write(ioutils::text_writer& w);
            void write(std::wostream& stream);
            void write(std::wstring& s);
            void write_to_file(const std::wstring file_name, const ioutils::text_io_options& options);
        private:
            config m_conf;
            const json::dom_document& m_doc;
        };


        class dom_document_generator
        {
        public:
            dom_document_generator(json::dom_document& doc);
            dom_document_generator() = delete;
            dom_document_generator(const dom_document_generator&) = delete;
            dom_document_generator& operator=(const dom_document_generator&) = delete;
            dom_document_generator(dom_document_generator&&) = delete;
            dom_document_generator& operator=(dom_document_generator&&) = delete;
        public:
            typedef std::vector<json::dom_value*> containers_t;

            class config
            {
            public:
                inline static unsigned int default_avg_children() noexcept { return 5; }
                inline static unsigned int default_avg_string_length() noexcept { return 50; }
                unsigned int avg_children() const noexcept { return m_avg_children; }
                void avg_children(const unsigned int value) noexcept { m_avg_children = (value > 0 ? value : m_avg_children); }
                unsigned int avg_string_length() const noexcept { return m_avg_string_length; }
                void avg_string_length(const unsigned int value) noexcept { m_avg_string_length = (value > 0 ? value : m_avg_string_length); }
                unsigned int depth() const { return m_depth; }
                void depth(const int value) { m_depth = value > 0 ? value : 1; }
                locutils::wchar_range& name_char_range() noexcept { return m_name_char_range; }
                locutils::wchar_range& value_char_range() noexcept { return m_value_char_range; }
            private:
                unsigned int m_avg_children = config::default_avg_children();
                unsigned int m_avg_string_length = config::default_avg_string_length();
                unsigned int m_depth = 1;
                locutils::wchar_range m_name_char_range{0x21, 0x7E};
                locutils::wchar_range m_value_char_range;
            };

        public:
            dom_document_generator::config& conf() { return m_config; }
            json::dom_document& doc() const { return m_doc; }
            void run();
        private:
            void generate_level(const int curr_level, const int level_count, const containers_t& parents);
            json::dom_value* generate_array();
            json::dom_value* generate_object();
            json::dom_value* generate_value(json::dom_value_type type);
            std::wstring random_literal_name();
            json::dom_value_type random_value_type();
            json::dom_value_type random_value_container_type();
            json::dom_value_type random_value_scalar_type();
        private:
            dom_document_generator::config m_config;
            json::dom_document& m_doc;
            testutils::rnd_helper m_rnd;
        };

    }
}

