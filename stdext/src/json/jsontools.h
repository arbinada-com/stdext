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
#include <fstream>
#include "jsoncommon.h"
#include "jsondom.h"
#include "../locutils.h"
#include "../ioutils.h"
#include "../testutils.h"

namespace stdext
{
    namespace json
    {

        class dom_document_reader
        {
        public:
            dom_document_reader(json::dom_document& doc);
            dom_document_reader() = delete;
            dom_document_reader(const dom_document_reader&) = delete;
            dom_document_reader& operator=(const dom_document_reader&) = delete;
            dom_document_reader(dom_document_reader&&) = delete;
            dom_document_reader& operator=(dom_document_reader&&) = delete;
        public:
            bool read(ioutils::text_reader& reader);
            bool read(std::istream& stream, const ioutils::text_io_policy& policy);
            bool read(std::wistream& stream);
            bool read(std::wistream& stream, const ioutils::text_io_policy& policy);
            bool read(std::wifstream& stream, const ioutils::text_io_policy& policy);
            bool read_file(const std::wstring file_name, const ioutils::text_io_policy& policy);
        public:
            const json::msg_collector_t& messages() const noexcept { return m_messages; }
            std::wstring source_name() const noexcept { return m_source_name; }
            void source_name(const std::wstring& value) { m_source_name = value; }
        private:
            json::dom_document& m_doc;
            json::msg_collector_t m_messages;
            std::wstring m_source_name;
        };


        class dom_document_writer
        {
        public:
            dom_document_writer(json::dom_document& doc);
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
            void write(ioutils::text_writer& w);
            void write(std::wostream& stream);
            void write(std::wstring& ws);
            void write_to_file(const std::wstring file_name, const ioutils::text_io_policy& policy);
        private:
            config m_conf;
            json::dom_document& m_doc;
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
                void depth(const unsigned int value) { m_depth = value > 0 ? value : 1; }
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


        /*
         * Document diff functions
         */
        enum class dom_document_diff_kind
        {
            count_diff,
            member_name_diff,
            path_diff,
            type_diff,
            numtype_diff,
            value_diff
        };
        std::string to_string(const dom_document_diff_kind value);
        std::wstring to_wstring(const dom_document_diff_kind value);

        class dom_document_diff_item
        {
        public:
            dom_document_diff_item(const dom_document_diff_kind kind,
                                   const dom_value* lval,
                                   const dom_value* rval)
                : m_kind(kind), m_lval(lval), m_rval(rval)
            {}
            dom_document_diff_item(const dom_document_diff_item&) = default;
            dom_document_diff_item& operator=(const dom_document_diff_item&) = default;
            dom_document_diff_item(dom_document_diff_item&&) = default;
            dom_document_diff_item& operator=(dom_document_diff_item&&) = default;
        public:
            dom_document_diff_kind kind() const noexcept { return m_kind; }
            const dom_value* rval() const noexcept { return m_rval; }
            const dom_value* lval() const noexcept { return m_lval; }
            std::string to_string() const;
            std::wstring to_wstring() const;
        private:
            dom_document_diff_kind m_kind;
            const dom_value* m_lval;
            const dom_value* m_rval;
        };

        class dom_document_diff
        {
        public:
            typedef std::vector<dom_document_diff_item> items_t;
        public:
            dom_document_diff() {}
            dom_document_diff(const dom_document_diff&) = default;
            dom_document_diff& operator=(const dom_document_diff&) = default;
            dom_document_diff(dom_document_diff&&) = default;
            dom_document_diff& operator=(dom_document_diff&&) = default;
        public:
            void append(const dom_document_diff_item& item) { m_items.push_back(item); }
            bool has_differences() const noexcept { return m_items.size() > 0; }
            const items_t& items() const noexcept { return m_items; }
        private:
            items_t m_items;
        };

        class dom_document_diff_options
        {
        public:
            dom_document_diff_options() {}
            dom_document_diff_options(const dom_document_diff_options&) = default;
            dom_document_diff_options& operator=(const dom_document_diff_options&) = default;
            dom_document_diff_options(dom_document_diff_options&&) = default;
            dom_document_diff_options& operator=(dom_document_diff_options&&) = default;
        public:
            bool case_sensitive() const noexcept { return m_case_sensitive; }
            void case_sensitive(const bool value) noexcept { m_case_sensitive = value; }
            bool compare_all() const noexcept { return m_compare_all; }
            void compare_all(const bool value) noexcept { m_compare_all = value; }
        private:
            bool m_case_sensitive = true;
            bool m_compare_all = false;
        };

        /**
         * @brief make_diff
         * @param ldoc
         * @param rdoc
         * @param options
         * @return
         */
        dom_document_diff make_diff(const json::dom_document& ldoc,
                                    const json::dom_document& rdoc,
                                    const dom_document_diff_options& options);
        dom_document_diff make_diff(const json::dom_document& ldoc, const json::dom_document& rdoc);

    }
}

