/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 I/O utils
 */
#pragma once

#include <iostream>
#include "locutils.h"

namespace stdext
{
    namespace ioutils
    {
        enum class text_encoding
        {
            ansi,
            utf8,
            utf16
        };

        enum class file_encoding_utf16
        {
            autodetect,
            big_endian,
            little_endian
        };

        class text_io_options
        {
        public:
            text_io_options() {}
            text_io_options(const text_io_options&) = default;
            text_io_options& operator=(const text_io_options&) = default;
            text_io_options(text_io_options&&) = default;
            text_io_options& operator=(text_io_options&&) = default;
            virtual ~text_io_options() {}
        protected:
            text_io_options(const text_encoding enc)
                : m_enc(enc)
            { }
        public:
            text_encoding encoding() const noexcept { return m_enc; }
            void encoding(text_encoding enc) noexcept { m_enc = enc; }
            virtual void set_imbue_read(std::wistream&) const = 0;
            virtual void set_imbue_write(std::wostream&) const = 0;
        protected:
            text_encoding m_enc = text_encoding::utf8;
        };

        class text_io_options_ansi : public text_io_options
        {
        public:
            text_io_options_ansi()
                : text_io_options(text_encoding::ansi)
            { }
            text_io_options_ansi(const std::string locale_name)
                : text_io_options(text_encoding::ansi), m_locale_name(locale_name)
            { }
        public:
            bool locale_default() const noexcept { return m_locale_name.empty(); }
            std::string locale_name() const { return m_locale_name; }
            virtual void set_imbue_read(std::wistream& stream) const override;
            virtual void set_imbue_write(std::wostream& stream) const override;
        protected:
            std::string m_locale_name;
        };

        class text_io_options_utf8 : public text_io_options
        {
        public:
            text_io_options_utf8()
                : text_io_options(text_encoding::utf8), m_cvt_mode(locutils::codecvt_mode_utf8())
            { }
            text_io_options_utf8(const locutils::codecvt_mode_utf8& cvt_mode)
                : text_io_options(text_encoding::utf8), m_cvt_mode(cvt_mode)
            { }
        public:
            locutils::codecvt_mode_utf8& cvt_mode() noexcept { return m_cvt_mode; }
            virtual void set_imbue_read(std::wistream& stream) const override;
            virtual void set_imbue_write(std::wostream& stream) const override;
        protected:
            locutils::codecvt_mode_utf8 m_cvt_mode;
        };

        class text_io_options_utf16 : public text_io_options
        {
        public:
            text_io_options_utf16()
                : text_io_options(text_encoding::utf16), m_cvt_mode(locutils::codecvt_mode_utf16())
            { }
            text_io_options_utf16(const locutils::codecvt_mode_utf16& cvt_mode)
                : text_io_options(text_encoding::utf16), m_cvt_mode(cvt_mode)
            { }
        public:
            locutils::codecvt_mode_utf16& cvt_mode() noexcept { return m_cvt_mode; }
            virtual void set_imbue_read(std::wistream& stream) const override;
            virtual void set_imbue_write(std::wostream& stream) const override;
        protected:
            locutils::codecvt_mode_utf16 m_cvt_mode;
        };

        /*
         * text_reader class
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
            text_reader(const std::wstring file_name, const text_io_options& options);
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
        public:
            inline bool is_next_char(wchar_t c) const { return m_stream->peek() == c; }
            bool is_next_char(std::initializer_list<wchar_t> chars) const;
        protected:
            std::wistream* m_stream = nullptr;
            bool m_owns_stream = false;
            std::wstring m_source_name;
        };

        /*
         * text_writer class
         * Designed to write wide strings to streams or to text files (both ANSI and unicode)
         */
        class text_writer
        {
        public:
            text_writer() = delete;
            text_writer(std::wostream& stream);
            text_writer(std::wostream* const stream);
            text_writer(const std::wstring file_name, const text_io_options& options);
            text_writer(const text_writer&) = delete;
            text_writer& operator=(const text_writer&) = delete;
            text_writer(text_writer&&) = delete;
            text_writer& operator=(text_writer&&) = delete;
            ~text_writer();
        public:
            text_writer& write(const std::wstring& s);
            text_writer& write(const wchar_t* s);
            text_writer& write_endl();
        protected:
            std::wostream* m_stream = nullptr;
            bool m_owns_stream = false;
        };
    }
}