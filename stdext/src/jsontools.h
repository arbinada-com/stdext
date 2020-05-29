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
                static const int default_avg_array_items = 10;
                static const int default_avg_object_members = 10;
                static const int default_avg_string_length = 50;
                int avg_array_items() const noexcept { return m_avg_array_items; }
                void avg_array_items(const int value) noexcept { m_avg_array_items = (value > 0 ? value : m_avg_array_items); }
                int avg_object_members() const noexcept { return m_avg_object_members; }
                void avg_object_members(const int value) noexcept { m_avg_object_members = (value > 0 ? value : m_avg_object_members); }
                int avg_string_length() const noexcept { return m_avg_string_length; }
                void avg_string_length(const int value) noexcept { m_avg_string_length = (value > 0 ? value : m_avg_string_length); }
                int min_depth() const { return m_min_depth; }
                void min_depth(const int value) { set_min(value, m_min_depth, m_max_depth); }
                int max_depth() const { return m_max_depth; }
                void max_depth(const int value) { set_max(value, m_min_depth, m_max_depth); }
                int max_values_by_level() const { return m_max_values_by_level; }
                void max_values_by_level(const int value) { set_max(value, m_min_values_by_level, m_max_values_by_level); }
                int min_values_by_level() const { return m_min_values_by_level; }
                void min_values_by_level(const int value) { set_min(value, m_min_values_by_level, m_max_values_by_level); }
            private:
                void set_min(const int value, int& min_value, int& max_value, const int min_limit = 1)
                {
                    min_value = value >= min_limit ? value : min_limit;
                    if (max_value < min_value)
                        max_value = min_value;
                }
                void set_max(const int value, int& min_value, int& max_value, const int max_limit = std::numeric_limits<int>::max())
                {
                    max_value = value <= max_limit ? value : max_limit;
                    if (max_value < min_value)
                        max_value = min_value;
                }
            private:
                int m_avg_array_items = config::default_avg_array_items;
                int m_avg_object_members = config::default_avg_object_members;
                int m_avg_string_length = config::default_avg_string_length;
                int m_min_depth = 1;
                int m_max_depth = 1;
                int m_min_values_by_level = 1;
                int m_max_values_by_level = 1;
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
        private:
            dom_document_generator::config m_config;
            json::dom_document& m_doc;
            testutils::rnd_helper m_rnd;
        };

    }
}

