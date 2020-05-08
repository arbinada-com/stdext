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
            dom_value* const parent() const noexcept { return m_parent; }
            virtual std::wstring text() const noexcept { return m_text; }
            virtual void text(const std::wstring value) noexcept(false) { m_text = value; }
            dom_value_type type() const noexcept { return m_type; }
        protected:
            void parent(dom_value* const value) noexcept;
        protected:
            dom_document* m_doc = nullptr;
            dom_value* m_parent = nullptr;
            std::wstring m_text;
            dom_value_type m_type = dom_value_type::vt_unknown;
        };

        class dom_literal : public dom_value
        { 
        public:
            dom_literal(dom_document* const doc, const std::wstring text);
            virtual std::wstring text() const noexcept override { return dom_value::text(); }
            void text(const std::wstring value) noexcept(false) override;
        };

        class dom_number : public dom_value
        {
        public:
            dom_number(dom_document* const doc, const std::wstring text);
        };

        class dom_string : public dom_value
        {
        public:
            dom_string(dom_document* const doc, const std::wstring text);
        };


        class dom_object_member
        {
        public:
            typedef std::wstring name_t;
        public:
            dom_object_member(dom_object_members* const owner, const name_t name, dom_value* const value);
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
            dom_value* const find(const name_t name) const noexcept;
            size_type size() const { return m_data.size(); }
        protected:
            void check_name(const name_t name) const noexcept(false);
        private:
            dom_object* m_owner = nullptr;
            data_t m_data;
            data_index_t m_index;
        };


        class dom_object : public dom_value
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
        public: // some facade of members() collection
            dom_object_member* const operator [](const size_type i) { return m_members->at(i); }
            void append_member(const name_t name, dom_value* const value) noexcept(false) { m_members->append(name, value); }
            dom_value* const find(const name_t name) const noexcept { return m_members->find(name); }
            size_type size() const { return m_members->size(); }
        private:
            dom_object_members* m_members = nullptr;
        };

        class dom_array : public dom_value
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
            size_type size() const { return m_data->size(); }
        private:
            data_t* m_data = nullptr;
        };


        class dom_document
        {
        public:
            ~dom_document();
        public:
            dom_array* const create_array();
            dom_literal* const create_literal(const std::wstring text);
            dom_number* const create_number(const std::wstring text);
            dom_object* const create_object();
            dom_string* const create_string(const std::wstring text);
            dom_value* const root() const noexcept { return m_root; }
            void root(dom_value* const value) noexcept(false);
        protected:
            void clear();
            dom_value* const create_value(const dom_value_type type, const std::wstring text);
        private:
            dom_value* m_root = nullptr;
        };
    }
}
