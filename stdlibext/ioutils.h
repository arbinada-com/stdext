/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 I/O utils
 */
#pragma once

#include <iostream>

namespace stdext
{
    namespace ioutils
    {
        enum class file_encoding
        {
            ansi,
            utf8,
            utf16
        };

        /*
         * text_reader adapter class 
         * Designed for use in lexer/parser
         * Reads wide chars sequentely from text streams or from text files (both ANSI and unicode)
         */
        class text_reader
        {
        public:
            text_reader() = delete;
            text_reader(std::wistream& stream);
            text_reader(std::wistream& stream, const std::wstring source_name);
            text_reader(std::wistream* const stream);
            text_reader(std::wistream* const stream, const std::wstring source_name);
            text_reader(const std::wstring file_name, const file_encoding enc, const char* locale_name = nullptr);
            text_reader(const text_reader&) = delete;
            text_reader& operator=(const text_reader&) = delete;
            text_reader(text_reader&&) = delete;
            text_reader& operator=(text_reader&&) = delete;
            ~text_reader();
        public:
            bool next_char(wchar_t& c);
            inline wchar_t next_char() { return m_stream->get(); }
            inline std::streamsize count() const { return m_stream->gcount(); }
            inline bool eof() const { return m_stream->eof(); }
            inline bool good() const { return m_stream->good(); }
            inline int rdstate() const { return m_stream->rdstate(); }
            std::wstring source_name() const noexcept { return m_source_name; }
        public:
            inline bool is_next_char(wchar_t c) const { return m_stream->peek() == c; }
            bool is_next_char(std::initializer_list<wchar_t> chars) const;
        protected:
            std::wistream* m_stream = nullptr;
            bool m_owns_stream = false;
            long m_char_count = 0;
            std::wstring m_source_name;
        };

    }
}