/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 Text parsing utils
 */
#pragma once

#include <iostream>
#include <string>
#include "strutils.h"
#include "ptr_vector.h"

namespace stdext
{
    namespace parsers
    {
        class textpos
        {
        public:
            typedef int pos_t;
        public:
            textpos() { }
            textpos(pos_t line, pos_t col)
                : m_line(line), m_col(col)
            { }
            textpos(const textpos&) = default;
            textpos& operator =(const textpos&) = default;
            textpos(textpos&&) = default;
            textpos& operator =(textpos&&) = default;
            ~textpos() { }
        public:
            void operator ++() noexcept;
            textpos& operator ++(pos_t) noexcept;
            bool operator ==(const textpos& rhs) const noexcept;
            void newline() noexcept;
            pos_t line() const noexcept { return m_line; }
            void line(const pos_t value) noexcept { m_line = value; }
            pos_t col() const noexcept { return m_col; }
            void col(const pos_t value) noexcept { m_col = value; }
            void reset() noexcept;
            std::string to_string() const;
            std::wstring to_wstring() const;
        private:
            pos_t m_line = 1;
            pos_t m_col = 1;
        };

        /*
         * Parsing messages
         * You need to define a custom enumeration of message kinds in the parser code to use these classes
         */
        enum class msg_origin
        {
            lexer,
            parser,
            generator,
            other
        };

        enum class msg_severity
        {
            info,
            hint,
            warning,
            error
        };
        std::wstring to_wstring(const msg_severity value);

        template <class MsgEnumT>
        class message
        {
        public:
            message() = delete;
            message(const msg_origin origin, const msg_severity severity, const MsgEnumT kind, const textpos pos, const std::wstring source)
                : message(origin, severity, kind, pos, source, L"")
            { }
            message(const msg_origin origin, const msg_severity severity, const MsgEnumT kind, const textpos pos, const std::wstring source, const std::wstring text)
                : m_origin(origin), m_severity(severity), m_kind(kind), m_pos(pos), m_source(source), m_text(text)
            { }
            message(const message&) = default;
            message& operator =(const message&) = default;
            message(message&&) = default;
            message& operator =(message&&) = default;
            virtual ~message() { }
        public:
            MsgEnumT kind() const noexcept { return m_kind; }
            msg_origin origin() const noexcept { return m_origin; }
            textpos pos() const noexcept { return m_pos; }
            msg_severity severity() const noexcept { return m_severity; }
            std::wstring source() const { return m_source; }
            void source(const std::wstring value) { m_source = value; }
            std::wstring text() const { return m_text; }
            std::wstring to_wstring() const 
            { 
                return strutils::format(L"%s%s: %s %d: %s", 
                    m_source.c_str(),
                    m_pos.to_wstring().c_str(),
                    parsers::to_wstring(m_severity).c_str(),
                    (int)m_kind,
                    m_text.c_str());
            }
        protected:
            msg_origin m_origin;
            msg_severity m_severity;
            MsgEnumT m_kind;
            textpos m_pos;
            std::wstring m_source;
            std::wstring m_text;
        };


        template <class MsgEnumT>
        class msg_collector
        {
        public:
            typedef message<MsgEnumT> message_t;
            typedef stdext::ptr_vector<message_t> container_t;
            typedef typename container_t::size_type size_type;
            typedef std::vector<message_t*> errors_t;
            typedef std::vector<message_t*> hints_t;
            typedef std::vector<message_t*> infos_t;
            typedef std::vector<message_t*> warnings_t;
            typedef typename container_t::const_iterator const_iterator;
        public:
            msg_collector() { }
            msg_collector(const msg_collector&) = delete;
            msg_collector& operator =(const msg_collector&) = delete;
            msg_collector(msg_collector&&) = delete;
            msg_collector& operator =(msg_collector&&) = delete;
            ~msg_collector() { }
        public:
            message_t* const at(const size_type i) const { return m_data.at(i); }
            message_t* const& operator [](const size_type i) const { return m_data.at(i); }
            const_iterator begin() const noexcept { return m_data.begin(); }
            const_iterator end() const noexcept { return m_data.end(); }
            void add(message_t* const msg)
            {
                m_data.push_back(msg);
                switch (msg->severity())
                {
                case msg_severity::error:
                    m_errors.push_back(msg);
                    break;
                case msg_severity::warning:
                    m_warnings.push_back(msg);
                    break;
                case msg_severity::hint:
                    m_hints.push_back(msg);
                    break;
                case msg_severity::info:
                    m_infos.push_back(msg);
                    break;
                }
            }
            void add_error(const msg_origin origin, const MsgEnumT kind, const textpos pos, const std::wstring source, const std::wstring text)
            {
                add(new message_t(origin, msg_severity::error, kind, pos, source, text));
            }
            void add_warning(const msg_origin origin, const MsgEnumT kind, const textpos pos, const std::wstring source, const std::wstring text)
            {
                add(new message_t(origin, msg_severity::warning, kind, pos, source, text));
            }
            void add_hint(const msg_origin origin, const MsgEnumT kind, const textpos pos, const std::wstring source, const std::wstring text)
            {
                add(new message_t(origin, msg_severity::hint, kind, pos, source, text));
            }
            void add_info(const msg_origin origin, const MsgEnumT kind, const textpos pos, const std::wstring source, const std::wstring text)
            {
                add(new message_t(origin, msg_severity::info, kind, pos, source, text));
            }
            inline bool has_errors() const noexcept { return m_errors.size() > 0; }
            inline bool has_hints() const noexcept { return m_hints.size() > 0; }
            inline bool has_infos() const noexcept { return m_infos.size() > 0; }
            inline bool has_warnings() const noexcept { return m_warnings.size() > 0; }
            inline bool has_messages() const noexcept { return m_data.size() > 0; }
            size_type size() const noexcept { return m_data.size(); }
        public:
            const hints_t& hints() const { return m_hints; }
            const warnings_t& warnings() const { return m_warnings; }
            const errors_t& errors() const { return m_errors; }
            const infos_t& infos() const { return m_infos; }
        private:
            container_t m_data;
            infos_t m_infos;
            hints_t m_hints;
            warnings_t m_warnings;
            errors_t m_errors;
        };
    }
}

