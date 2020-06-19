/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 I/O utils
 */
#pragma once

#include <iostream>
#include <deque>
#include "locutils.h"

namespace stdext
{
    namespace ioutils
    {

        typedef std::deque<wchar_t> text_buffer_t;
        class text_reader;
        class text_writer;

        class text_io_policy
        {
            friend class text_reader;
            friend class text_writer;
        public:
            text_io_policy() {}
            text_io_policy(const text_io_policy&) = default;
            text_io_policy& operator=(const text_io_policy&) = default;
            text_io_policy(text_io_policy&&) = default;
            text_io_policy& operator=(text_io_policy&&) = default;
            virtual ~text_io_policy() {}
        public:
            std::size_t max_text_buf_size() const noexcept { return m_max_text_buf_size; }
            void max_text_buf_size(const std::size_t value) noexcept { m_max_text_buf_size = value; }
        protected:
            virtual void do_read_chars(mbstate_t& mbstate, std::wistream& stream, text_buffer_t& buf) const = 0;
            virtual void set_imbue_read(std::wistream&) const = 0;
            virtual void set_imbue_write(std::wostream&) const = 0;
        protected:
            void push_back_chars(const std::wstring& ws, text_buffer_t& buf) const;
        protected:
            std::size_t m_max_text_buf_size = 1024;
        };


        class text_io_policy_plain : public text_io_policy
        {
        protected:
            void do_read_chars(mbstate_t&, std::wistream& stream, text_buffer_t& buf) const override;
            void set_imbue_read(std::wistream&) const override {}
            void set_imbue_write(std::wostream&) const override {}
        };


        class text_io_policy_ansi : public text_io_policy
        {
        public:
            typedef text_io_policy base_t;
        public:
            text_io_policy_ansi();
            text_io_policy_ansi(const locutils::ansi_encoding encoding);
            text_io_policy_ansi(const char* encoding_name);
            text_io_policy_ansi(const locutils::codecvt_mode_ansi& cvt_mode);
            ~text_io_policy_ansi() override;
        public:
            inline bool use_default_encoding() const noexcept { return !m_cvt_mode.encoding_assigned(); }
            const locutils::codecvt_mode_ansi& cvt_mode() const noexcept { return m_cvt_mode; }
        protected:
            void do_read_chars(mbstate_t& state, std::wistream& stream, text_buffer_t& buf) const override;
            void set_imbue_read(std::wistream& stream) const override;
            void set_imbue_write(std::wostream& stream) const override;
        private:
            bool read_bytes(std::wistream& stream, std::string& bytes, const size_t max_len) const;
        protected:
            locutils::codecvt_mode_ansi m_cvt_mode;
            locutils::codecvt_ansi_utf16_wchar_t* m_cvt = nullptr;
        };


        class text_io_policy_utf8 : public text_io_policy
        {
        public:
            typedef text_io_policy base_t;
        public:
            text_io_policy_utf8();
            text_io_policy_utf8(const locutils::codecvt_mode_utf8& cvt_mode);
            ~text_io_policy_utf8() override;
        public:
            const locutils::codecvt_mode_utf8& cvt_mode() const noexcept { return m_cvt_mode; }
        protected:
            void do_read_chars(mbstate_t& state, std::wistream& stream, text_buffer_t& buf) const override;
            void set_imbue_read(std::wistream& stream) const override;
            void set_imbue_write(std::wostream& stream) const override;
        private:
            bool read_bytes(std::wistream& stream, std::string& bytes, const size_t count) const;
        protected:
            locutils::codecvt_mode_utf8 m_cvt_mode;
            locutils::codecvt_utf8_wchar_t* m_cvt;
        };


        class text_io_policy_utf16 : public text_io_policy
        {
        public:
            typedef text_io_policy base_t;
        public:
            text_io_policy_utf16();
            text_io_policy_utf16(const locutils::codecvt_mode_utf16& cvt_mode);
            ~text_io_policy_utf16() override;
        public:
            const locutils::codecvt_mode_utf16& cvt_mode() const noexcept { return m_cvt_mode; }
        protected:
            void do_read_chars(mbstate_t& state, std::wistream& stream, text_buffer_t& buf) const override;
            void set_imbue_read(std::wistream& stream) const override;
            void set_imbue_write(std::wostream& stream) const override;
        private:
            bool read_bytes(std::wistream& stream, std::string& bytes, const size_t max_len) const;
        protected:
            locutils::codecvt_mode_utf16 m_cvt_mode;
            locutils::codecvt_utf16_wchar_t* m_cvt;
        };

        /**
         * @brief The text_reader class
         * Designed for use in lexer/parser
         * Reads wide chars sequentely from text streams or from text files (both ANSI and Unicode)
         */
        class text_reader
        {
        public:
            text_reader() = delete;
            text_reader(std::wistream& stream);
            text_reader(std::wistream& stream, const text_io_policy& policy);
            text_reader(const std::wstring& file_name, const text_io_policy& policy);
            text_reader(std::wifstream& stream, const text_io_policy& policy);
            text_reader(const text_reader&) = delete;
            text_reader& operator=(const text_reader&) = delete;
            text_reader(text_reader&&) = delete;
            text_reader& operator=(text_reader&&) = delete;
            virtual ~text_reader();
        public:
            bool next_char(wchar_t& wc);
            bool is_next_char(wchar_t wc);
            bool is_next_char(std::initializer_list<wchar_t> wchars);
            std::streamsize count() const;
            bool eof() const;
            bool peek(wchar_t& wc);
            virtual void read_all(std::wstring& ws);
            std::wstring source_name() const noexcept { return m_source_name; }
            void source_name(const std::wstring& value) { m_source_name = value; }
        protected:
            void read_chars();
        protected:
            std::wistream* m_stream = nullptr;
            bool m_owns_stream = false;
            std::wstring m_source_name;
            text_buffer_t m_chars;
            bool m_use_file_io = false;
            text_io_policy_plain m_default_policy;
            const text_io_policy& m_policy = m_default_policy;
            mbstate_t m_mbstate = {};
        };


        /**
         * @brief The text_writer class
         * Designed to write wide strings to streams or to text files (both ANSI and Unicode)
         */
        class text_writer
        {
        public:
            text_writer() = delete;
            text_writer(std::wostream& stream);
            text_writer(std::wostream* const stream);
            text_writer(const std::wstring file_name, const text_io_policy& policy);
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
