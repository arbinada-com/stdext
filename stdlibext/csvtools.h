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

namespace stdext
{
    namespace csv
    {
        class field
        {
        public:
            typedef typename std::wstring value_t;
        public:
            field(value_t value)
                : m_value(value)
            { }
            field(const field& source) { *this = source; };
            field& operator=(const field& source)
            {
                m_value = source.m_value;
                return *this;
            }
            field(field&& source)
            {
                m_value = std::move(source.m_value);
            }
            field& operator=(field&& source)
            {
                m_value = std::move(source.m_value);
                return *this;
            }
            value_t value() const { return m_value; }
        private:
            value_t m_value;
        };

        class row
        {
        public:
            typedef typename csv::field::value_t value_t;
            typedef typename std::vector<csv::field> row_data_t;
            typedef typename row_data_t::size_type size_type;
        public:
            row() {}
            row(const row&) = delete;
            row& operator=(const row&) = delete;
            row(row&& source) { *this = std::move(source); }
            row& operator=(row&& source)
            {
                m_row = std::move(source.m_row);
            }
        public:
            csv::field& operator[](const size_type pos) { return field(pos); }
            csv::field& field(const size_type pos) { return m_row.at(pos); }
            void append_value(const value_t& value)
            {
                m_row.push_back(csv::field(value));
            }
            void clear() { m_row.clear(); }
            size_type field_count() const { return m_row.size(); }
            value_t value(const size_type pos) { return m_row.at(pos).value(); }
        private:
            row_data_t m_row;
        };

        class header : public csv::row
        {};


        enum class reader_error
        {
            none = 0,
            IOError = 100,
            expectedSeparator = 200,
            rowFieldCountIsDifferentFromHeader = 300
        };

        enum class file_encoding
        {
            ansi,
            utf8,
            utf16
        };

        class reader
        {
        public:
            typedef std::wistream stream_t;
        public:
            reader(stream_t& stream);
            reader(stream_t* const stream);
            reader(const std::wstring filename, const file_encoding enc, const char* locale_name = nullptr);
            reader(const reader&) = delete;
            reader& operator=(const reader&) = delete;
            reader(reader&&) = delete;
            reader& operator=(reader&&) = delete;
            ~reader();
        public:
            bool next_row(csv::row& r);
            bool read_header();
            bool has_error() const { return m_error != csv::reader_error::none; }
            csv::reader_error error() const { return m_error; }
            const csv::header& header() { m_header; }
            bool has_header() { return m_header.field_count() > 0; }
            long col_num() const { return m_col_num; }
            long line_num() const { return m_line_num; }
            long row_count() const { return m_row_num; }
            wchar_t separator() const { return m_separator; }
            void separator(const wchar_t value) { m_separator = value; }
        protected:
            wchar_t next_char();
            inline bool is_next_char(wchar_t c) const { return m_stream->peek() == c; }
            bool is_next_char(std::initializer_list<wchar_t> chars) const;
            inline bool eof() const { return m_stream->eof(); }
            inline bool good() const { return m_stream->good(); }
        private:
            stream_t* m_stream = nullptr;
            bool m_owns_stream = false;
            wchar_t m_separator = ',';
            long m_col_num = 0;
            long m_line_num = 0;
            long m_row_num = 0;
            reader_error m_error = reader_error::none;
            csv::header m_header;
        };
    }
}
