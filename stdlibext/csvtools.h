/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 Comma-Separated Values (CSV) tools
 Based on RFC 4180 (https://tools.ietf.org/html/rfc4180)
 */
#pragma once

#include <vector>

namespace stdext
{
    namespace csv
    {
        template <class CharT, class Traits = std::char_traits<CharT> >
        class basic_field
        {
        public:
            typedef typename std::basic_string<CharT, Traits> value_t;
        public:
            basic_field(value_t value)
                : m_value(value)
            { }
            basic_field(const basic_field&) = delete;
            basic_field& operator=(const basic_field&) = delete;
            basic_field(basic_field&& source)
            {
                m_value = std::move(source.m_value);
            }
            virtual ~basic_field() {}
            value_t value() const { return m_value; }
        private:
            value_t m_value;
        };

        class field : public basic_field<char>
        {};

        class wfield : public basic_field<wchar_t>
        {};


        template <class CharT, class Traits = std::char_traits<CharT> >
        class basic_row
        {
        public:
            typedef typename csv::basic_field<CharT, Traits> field_t;
            typedef typename field_t::value_t value_t;
            typedef typename std::vector<field_t> row_data_t;
            typedef typename row_data_t::size_type size_type;
        public:
            basic_row() {}
            basic_row(const basic_row&) = delete;
            basic_row& operator=(const basic_row&) = delete;
            basic_row(basic_row&& source)
            {
                m_row = std::move(source.m_row);
            }
        public:
            field_t& operator[](const size_type pos) { return field(pos); }
            field_t& field(const size_type pos) { return m_row.at(pos); }
            void append_value(const value_t& value)
            {
                m_row.push_back(field_t(value));
            }
            void clear() { m_row.clear(); }
            size_type field_count() const { return m_row.size(); }
            value_t& value(const size_type pos) { return m_row.at(pos).value(); }
        private:
            row_data_t m_row;
        };

        template <class CharT, class Traits = std::char_traits<CharT> >
        class basic_header : public csv::basic_row<CharT, Traits>
        {};

        class row : public basic_row<char>
        {};

        class wrow : public basic_row<wchar_t>
        {};


        enum class reader_error
        {
            none = 0,
            IOError = 100,
            expectedSeparator = 200,
        };

        template <class CharT, class Traits = std::char_traits<CharT> >
        class basic_reader
        {
        public:
            typedef typename basic_row<CharT> row_t;
            typedef typename std::basic_istream<CharT, Traits> stream_t;
        public:
            basic_reader(stream_t& stream)
                : m_stream(stream)
            {}
            basic_reader(const basic_reader&) = delete;
            basic_reader& operator=(const basic_reader&) = delete;

        public:
            bool next_row(row_t& r)
            {
                r.clear();
                bool row_accepted = false;
                bool value_accepted = false;
                bool accepting_quoted_value = false;
                std::basic_string<CharT, Traits> value;
                m_col_num = 0;
                while (!m_stream.eof() && !row_accepted)
                {
                    CharT c = next_char();
                    if (!good())
                    {
                        if (!eof())
                            m_error = reader_error::IOError;
                        return false;
                    }
                    switch (c)
                    {
                    case '"':
                        if (!accepting_quoted_value)
                            accepting_quoted_value = true;
                        else 
                        {
                            if (is_next_char(c))
                                value += next_char();
                            else
                            {
                                accepting_quoted_value = false;
                                if (!is_next_char({ m_separator, '\r', '\n' }))
                                {
                                    m_error = reader_error::expectedSeparator;
                                    return false;
                                }
                            }
                        }
                        break;
                    case '\r':
                    case '\n':
                        if (accepting_quoted_value)
                            value += c;
                        else 
                        {
                            value_accepted = true;
                            row_accepted = true;
                            m_line_num++;
                            m_col_num = 0;
                            if (c == '\r' && is_next_char('\n'))
                                next_char();
                        }
                        break;
                    default:
                        if (c != m_separator || accepting_quoted_value)
                            value += c;
                        else
                            value_accepted = true;
                        break;
                    }
                    if (value_accepted)
                    {
                        value_accepted = false;
                        r.append_value(value);
                        value.clear();
                    }
                }
                m_row_num++;
                return true;
            }

            bool has_error() const { return m_error != reader_error::none; }
            reader_error error() const { return m_error; }
            long col_num() const { return m_col_num; }
            long line_num() const { return m_line_num; }
            long row_count() const { return m_row_num; }
            CharT separator() const { return m_separator; }
            void separator(const CharT value) { m_separator = value; }
        protected:
            CharT next_char()
            {
                CharT c = m_stream.get();
                if (good())
                    m_col_num++;
                return c;
            }
            bool is_next_char(CharT c) { return m_stream.peek() == c; }
            bool is_next_char(std::initializer_list<CharT> chars) 
            { 
                for (CharT c : chars)
                {
                    if (m_stream.peek() == c)
                        return true;
                }
                return false;
            }
            bool good() const { return m_stream.good(); }
            bool eof() const { return m_stream.eof(); }
        private:
            stream_t& m_stream;
            CharT m_separator = ',';
            long m_col_num = 0;
            long m_line_num = 0;
            long m_row_num = 0;
            reader_error m_error = reader_error::none;
        };

        class reader : public basic_reader<char>
        {
        public:
            reader(std::basic_istream<char>& stream)
                : basic_reader<char>(stream)
            {}
        };

        class wreader : public basic_reader<wchar_t>
        {
        public:
            wreader(std::basic_istream<wchar_t>& stream)
                : basic_reader<wchar_t>(stream)
            {}
        };

    }
}
