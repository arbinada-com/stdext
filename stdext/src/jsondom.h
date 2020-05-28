/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)

 JSON (JavaScript Object Notation) tools
 Based on RFC 8259 (https://tools.ietf.org/html/rfc8259)
 */
#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <cstdint>
#include <limits>
#include "jsonexceptions.h"
#include "ptr_vector.h"

namespace stdext
{
    namespace json
    {
        /*
            DOM class hierarchy
            ---
            dom_value
            |-- dom_array
            |-- dom_literal
            |-- dom_number
            |-- dom_object
            |-- dom_string
            
            DOM object hierarchy
            ---
            document (dom_document)
            |-- root (array, object or other dom_value)

            array (dom_array)
            |-- values[] 
                |-- value1 (dom_value)
                ...
                |-- valueN

            object (dom_object)
            |-- members[] (dom_object_members)
                |-- member1 (dom_object_member)
                |   |-- name (string)
                |   |-- value (dom_value)
                ...
                |-- memberN
        */
        enum class dom_error
        {
            document_is_not_same,
            document_is_null,
            duplicate_name,
            invalid_literal,
            owner_is_null,
            parent_is_not_null,
            usupported_value_type,
        };

        class dom_exception : public json::exception
        {
        public:
            dom_exception(const std::wstring msg, const dom_error error)
                : json::exception(msg), m_error(error)
            { }
            dom_error error() const { return m_error; }
        protected:
            dom_error m_error;
        };


        enum class dom_value_type
        {
            vt_unknown,
            vt_array,
            vt_literal,
            vt_number,
            vt_object,
            vt_string
        };
        const dom_value_type first_value_type = dom_value_type::vt_unknown;
        const dom_value_type last_value_type = dom_value_type::vt_string;
        std::string to_string(const dom_value_type type);
        std::wstring to_wstring(const dom_value_type type);

        class dom_array;
        class dom_object;
        class dom_document;
        class dom_object_member;
        class dom_object_members;

        class dom_value abstract
        {
            friend class dom_array;
            friend class dom_object_members;
        public:
            dom_value(dom_document* const doc, const dom_value_type type);
            dom_value() = delete;
            dom_value(const dom_value&) = delete;
            dom_value& operator =(const dom_value&) = delete;
            dom_value(dom_value&&) = delete;
            dom_value& operator =(dom_value&&) = delete;
            virtual ~dom_value();
        public:
            void assert_same_doc(dom_document* doc) const noexcept(false);
            void assert_no_parent() const noexcept(false);
            void assert_same_doc_no_parent(dom_document* doc) const noexcept(false);
            virtual void clear() { m_text.clear(); }
            dom_document* const document() const noexcept { return m_doc; }
            bool is_container() const noexcept { return m_type == dom_value_type::vt_array || m_type == dom_value_type::vt_object; }
            dom_object_member* member() const noexcept { return m_member; }
            dom_value* const parent() const noexcept { return m_parent; }
            virtual std::wstring text() const noexcept { return m_text; }
            virtual void text(const std::wstring value) noexcept(false) { m_text = value; }
            dom_value_type type() const noexcept { return m_type; }
        public:
            friend bool operator ==(const dom_value& v1, const dom_value& v2);
            friend bool operator !=(const dom_value& v1, const dom_value& v2);
        protected:
            void parent(dom_value* const value) noexcept(false);
        protected:
            dom_document* m_doc = nullptr;
            dom_object_member* m_member = nullptr;
            dom_value* m_parent = nullptr;
            std::wstring m_text;
            dom_value_type m_type = dom_value_type::vt_unknown;
        };

        enum class dom_literal_value_type
        {
            lvt_false,
            lvt_null,
            lvt_true
        };

        class dom_literal : public dom_value
        { 
        public:
            dom_literal(dom_document* const doc, const std::wstring text);
        public:
            dom_literal_value_type subtype() const noexcept { return m_subtype; }
            virtual std::wstring text() const noexcept override 
            { return dom_value::text(); } // prevent error C2660 function does not take 0 arguments
            void text(const std::wstring value) noexcept(false) override;
        protected:
            dom_literal_value_type m_subtype;
        };

        enum class dom_number_value_type
        {
            nvt_float,
            nvt_int
        };

        class dom_number : public dom_value
        {
        public:
            dom_number(dom_document* const doc, const std::wstring& text, const dom_number_value_type subtype);
            dom_number(dom_document* const doc, const int32_t value);
            dom_number(dom_document* const doc, const int64_t value);
            dom_number(dom_document* const doc, const double value);
        public:
            dom_number_value_type subtype() const noexcept { return m_subtype; }
            void subtype(const dom_number_value_type value) noexcept { m_subtype = value; }
        protected:
            dom_number_value_type m_subtype = dom_number_value_type::nvt_float; // most generic number type
        };

        class dom_string : public dom_value
        {
        public:
            dom_string(dom_document* const doc, const wchar_t* text);
            dom_string(dom_document* const doc, const std::wstring& text);
        };


        class dom_object_member
        {
        public:
            typedef std::wstring name_t;
        public:
            dom_object_member(dom_object_members* const owner, const name_t& name, dom_value* const value);
            dom_object_member() = delete;
            dom_object_member(const dom_object_member&) = delete;
            dom_object_member& operator =(const dom_object_member&) = delete;
            dom_object_member(dom_object_member&&) = delete;
            dom_object_member& operator =(dom_object_member&&) = delete;
            ~dom_object_member();
        public:
            name_t name() const noexcept { return m_name; }
            dom_object_members* const owner() { return m_owner; }
            dom_value* const value() const noexcept { return m_value; }
        private:
            name_t m_name;
            dom_value* m_value = nullptr;
            dom_object_members* m_owner = nullptr;
        };

        class dom_object_members
        {
            friend class dom_object;
        public:
            typedef typename dom_object_member::name_t name_t;
            typedef stdext::ptr_vector<dom_object_member> data_t;
            typedef std::map<name_t, dom_object_member*> data_index_t;
            typedef typename data_t::size_type size_type;
            typedef typename data_t::const_iterator const_iterator;
            typedef typename data_t::iterator iterator;
        public:
            dom_object_members() = delete;
            dom_object_members(const dom_object_members&) = delete;
            dom_object_members& operator =(const dom_object_members&) = delete;
            dom_object_members(dom_object_members&&) = delete;
            dom_object_members& operator =(dom_object_members&&) = delete;
            ~dom_object_members() { }
        protected:
            dom_object_members(dom_object* const owner);
        public:
            void append(const name_t name, dom_value* const value) noexcept(false);
            void clear() noexcept;
            dom_object_member* const at(const size_type i) { return m_data.at(i); }
            dom_object_member* const operator [](const size_type i) { return m_data.at(i); }
            iterator begin() noexcept { return m_data.begin(); }
            const_iterator begin() const noexcept { return m_data.begin(); }
            iterator end() noexcept { return m_data.end(); }
            const_iterator end() const noexcept { return m_data.end(); }
            inline bool contains_name(const name_t name) const noexcept { return find(name) != nullptr; }
            bool empty() const noexcept { return m_data.empty(); }
            dom_value* const find(const name_t name) const noexcept;
            size_type size() const { return m_data.size(); }
        protected:
            void check_name(const name_t name) const noexcept(false);
        private:
            dom_object* m_owner = nullptr;
            data_t m_data;
            data_index_t m_index;
        };

        class indexer_intf abstract
        {
        public:
            virtual dom_value* const get_value(const std::size_t i) = 0;
            virtual std::size_t value_count() const = 0;
        };

        class dom_object : public dom_value, public indexer_intf
        {
        public:
            typedef dom_object_member::name_t name_t;
            typedef dom_object_members::size_type size_type;
        public:
            dom_object(dom_document* const doc);
            ~dom_object() override;
        public:
            void clear() override;
            dom_object_members* const members() { return m_members; }
            const dom_object_members* cmembers() const { return m_members; }
        public: // some facade of members() collection
            dom_object_member* const operator [](const size_type i) { return m_members->at(i); }
            void append_member(const name_t name, dom_value* const value) noexcept(false) { m_members->append(name, value); }
            inline bool contains_member(const name_t name) const noexcept { return m_members->contains_name(name); }
            inline dom_value* const find(const name_t name) const noexcept { return m_members->find(name); }
            size_type size() const { return m_members->size(); }
        public: // indexer_intf implementation
            dom_value* const get_value(const std::size_t i) override { return m_members->at(i)->value(); };
            std::size_t value_count() const override { return m_members->size(); }
        private:
            dom_object_members* m_members = nullptr;
        };

        class dom_array : public dom_value, public indexer_intf
        {
        public:
            typedef stdext::ptr_vector<dom_value> data_t;
            typedef typename data_t::size_type size_type;
            typedef typename data_t::const_iterator const_iterator;
            typedef typename data_t::iterator iterator;
        public:
            dom_array(dom_document* const doc);
            ~dom_array() override;
        public:
            void clear() noexcept { m_data->clear(); }
            dom_value* const at(const size_type i) { return m_data->at(i); }
            dom_value* const operator [](const size_type i) { return m_data->at(i); }
            iterator begin() noexcept { return m_data->begin(); }
            const_iterator begin() const noexcept { return m_data->begin(); }
            iterator end() noexcept { return m_data->end(); }
            const_iterator end() const noexcept { return m_data->end(); }
            void append(dom_value* const value) noexcept;
            bool empty() const noexcept { return m_data->empty(); }
            size_type size() const { return m_data->size(); }
        public: // indexer_intf implementation
            dom_value* const get_value(const std::size_t i) override { return m_data->at(i); };
            std::size_t value_count() const override { return m_data->size(); }
        private:
            data_t* m_data = nullptr;
        };


        class dom_document
        {
        public:
            dom_document() { }
            dom_document(const dom_document&) = delete;
            dom_document& operator =(const dom_document&) = delete;
            dom_document(dom_document&& source);
            dom_document& operator =(dom_document&& source);
            ~dom_document();
        public:
            void clear();
            dom_array* const create_array();
            dom_literal* const create_literal(const std::wstring text);
            dom_number* const create_number(const std::wstring text, const json::dom_number_value_type numtype);
            dom_number* const create_number(const int32_t value);
            dom_number* const create_number(const int64_t value);
            dom_number* const create_number(const double value);
            dom_object* const create_object();
            dom_string* const create_string(const wchar_t* text);
            dom_string* const create_string(const std::wstring& text);
            dom_value* const root() const noexcept { return m_root; }
            void root(dom_value* const value) noexcept(false);
        public:
            class const_iterator
            {
                friend class dom_document;
            public:
                typedef std::vector<std::size_t> path_t;
            public:
                const_iterator(const dom_document& doc);
                const_iterator(const dom_document* doc);
                const_iterator() = delete;
                const_iterator(const const_iterator&) = default;
                const_iterator& operator =(const const_iterator&) = default;
                const_iterator(const_iterator&&) = default;
                const_iterator& operator =(const_iterator&&) = default;
                virtual ~const_iterator() {}
            public:
                dom_value* next();
                inline void operator ++() { next(); }
                inline const_iterator& operator ++(int) { next(); return *this; }
                inline bool operator ==(const const_iterator& rhs) 
                { 
                    return (m_doc == rhs.m_doc) &&
                        ((is_end() && rhs.is_end()) || m_current == rhs.m_current); 
                }
                inline bool operator !=(const const_iterator& rhs) { return !(*this == rhs); }
                inline const dom_value* operator *() const { return m_current; }
                inline const dom_value* value() const noexcept { return m_current; }
                inline std::size_t level() const noexcept { return m_path.size(); }
                bool has_prev_sibling() const noexcept;
                inline bool is_end() const noexcept { return m_current == nullptr; }
                inline const path_t path() const { return m_path; }
            protected:
                const dom_document* m_doc;
                dom_value*          m_current;
                path_t              m_path;
            };

            class iterator : public const_iterator
            { 
            public:
                iterator(const dom_document& doc) : const_iterator(doc) {}
                iterator(const dom_document* doc) : const_iterator(doc) {}
            public:
                inline dom_value* operator *() { return m_current; }
                inline dom_value* value() noexcept { return m_current; }
            };

        public:
            iterator begin();
            const_iterator begin() const;
            iterator end();
            const_iterator end() const;
        private:
            dom_value* m_root = nullptr;
        };

        bool equal(const dom_value& v1, const dom_value& v2);
        bool equal(const dom_document& doc1, const dom_document& doc2);

    }
}

