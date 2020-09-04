/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 Comma-Separated Values (CSV) tools
 Based on RFC 4180 (https://tools.ietf.org/html/rfc4180)
 */
#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "ioutils.h"
#include "parsers.h"

namespace stdext
{
    namespace csv
    {
        class field
        {
        public:
            field(std::wstring value)
                : m_value(value)
            {}
            field(const field& source) = default;
            field& operator=(const field& source) = default;
            field(field&& source) = default;
            field& operator=(field&& source) = default;
        public:
            std::wstring value() const { return m_value; }
        private:
            std::wstring m_value;
        };

        class row
        {
        public:
            typedef std::vector<csv::field> row_data_t;
            typedef typename row_data_t::size_type size_type;
        public:
            row() {}
            row(const row&) = default;
            row& operator=(const row&) = default;
            row(row&& source) = default;
            row& operator=(row&& source) = default;
            virtual ~row() {}
        public:
            const csv::field& operator[](const size_type pos) const { return field(pos); }
            csv::field& operator[](const size_type pos) { return field(pos); }
            const csv::field& field(const size_type pos) const { return m_row.at(pos); }
            csv::field& field(const size_type pos) { return m_row.at(pos); }
            void append_value(const std::wstring& value)
            {
                m_row.push_back(csv::field(value));
            }
            void clear() { m_row.clear(); }
            size_type field_count() const { return m_row.size(); }
            std::wstring value(const size_type pos) { return m_row.at(pos).value(); }
        private:
            row_data_t m_row;
        };

        class header : public csv::row
        {};


        enum class reader_msg_kind
        {
            io_error = 101,
            expected_separator = 201,
            row_field_count_different_from_header = 301
        };
        std::wstring to_wstring(const reader_msg_kind value);

        class reader
        {
        public:
            typedef parsers::msg_collector<reader_msg_kind> msg_collector_t;
            typedef msg_collector_t::message_t message_t;
        public:
            reader() = delete;
            reader(ioutils::text_reader* const rd);
            reader(std::wistream& stream);
            reader(std::wistream& stream, const ioutils::text_io_policy& policy);
            reader(const std::wstring file_name, const ioutils::text_io_policy& policy);
            reader(std::wifstream& stream, const ioutils::text_io_policy& policy);
            reader(const reader&) = delete;
            reader& operator =(const reader&) = delete;
            reader(reader&&) = delete;
            reader& operator =(reader&&) = delete;
            ~reader();
        public:
            bool next_row(csv::row& r);
            bool read_header();
            inline bool eof() const noexcept { return m_reader->eof(); }
            bool has_error() const { return m_messages.has_errors(); }
            const csv::header& header() { return m_header; }
            bool has_header() { return m_header.field_count() > 0; }
            const parsers::textpos pos() const noexcept { return m_pos; }
            long row_count() const { return m_row_num; }
            inline wchar_t separator() const { return m_separator; }
            void separator(const wchar_t value) { m_separator = value; }
            const msg_collector_t& messages() const { return m_messages; }
            const msg_collector_t::errors_t& errors() const { return m_messages.errors(); }
        private:
            bool next_char(wchar_t& wc);
            void add_error(const reader_msg_kind kind, const std::wstring text);
            void add_error(const reader_msg_kind kind, parsers::textpos pos, const std::wstring text);
        private:
            wchar_t m_separator = ',';
            parsers::textpos m_pos;
            long m_row_num = 0;
            csv::header m_header;
            ioutils::text_reader* m_reader = nullptr;
            bool m_owns_reader = false;
            msg_collector_t m_messages;
        };
    }
}
