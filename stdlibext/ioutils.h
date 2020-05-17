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
        void set_imbue(std::wios* stream, const file_encoding enc, const char* locale_name = nullptr);
        bool is_high_surrogate(const wchar_t c);
        bool is_low_surrogate(const wchar_t c);
        bool is_noncharacter(const wchar_t c);
        inline wchar_t replacement_character() { return 0xFFFD; }

        enum class file_encoding_utf16
        {
            autodetect,
            big_endian,
            little_endian
        };

        /*
         * text_reader adapter class 
         * Designed for use in lexer/parser
         * Reads wide chars sequentely from text streams or from text files (both ANSI and unicode)
         */
        class text_file_options
        {
        public:
            text_file_options() {}
            text_file_options(file_encoding enc) 
                : m_enc(enc)
            {}
            text_file_options(const text_file_options&) = default;
            text_file_options& operator=(const text_file_options&) = default;
            text_file_options(text_file_options&&) = default;
            text_file_options& operator=(text_file_options&&) = default;
        public:
            file_encoding encoding() const { return m_enc; }
        private:
            file_encoding m_enc = file_encoding::utf8;
            char* m_locale_name;
        };

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
            void read_all(std::wstring& s);
            std::wstring source_name() const noexcept { return m_source_name; }
            std::wistream& stream() { return *m_stream; }
        public:
            inline bool is_next_char(wchar_t c) const { return m_stream->peek() == c; }
            bool is_next_char(std::initializer_list<wchar_t> chars) const;
        protected:
            std::wistream* m_stream = nullptr;
            bool m_owns_stream = false;
            long m_char_count = 0;
            std::wstring m_source_name;
        };

        class text_writer
        {
        public:
            text_writer() = delete;
            text_writer(std::wostream& stream);
            text_writer(std::wostream* const stream);
            text_writer(const std::wstring file_name, const file_encoding enc, const char* locale_name = nullptr);
            text_writer(const text_writer&) = delete;
            text_writer& operator=(const text_writer&) = delete;
            text_writer(text_writer&&) = delete;
            text_writer& operator=(text_writer&&) = delete;
            ~text_writer();
        public:
            text_writer& operator <<(const std::wstring s);
            std::wostream& stream() { return *m_stream; }
        protected:
            std::wostream* m_stream = nullptr;
            bool m_owns_stream = false;
        };
    }
}