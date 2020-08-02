/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "jsondom.h"
#include <memory>
#include <sstream>
#include <stack>
#include <cmath>
#include "../strutils.h"
#include "../locutils.h"
#include "../testutils.h"
#include "jsoncommon.h"

using namespace std;
using namespace stdext;
using namespace json;

std::string json::to_string(const dom_value_type type)
{
    switch (type)
    {
    case dom_value_type::vt_array: return "array";
    case dom_value_type::vt_literal: return "literal";
    case dom_value_type::vt_number: return "number";
    case dom_value_type::vt_object: return "object";
    case dom_value_type::vt_string: return "string";
    default:
        return "unsupported";
    }
}
std::wstring json::to_wstring(const dom_value_type type)
{
    return strutils::to_wstring(json::to_string(type));
}

bool stdext::json::operator ==(const dom_value& v1, const dom_value& v2)
{
    return equal(v1, v2);
}

bool stdext::json::operator !=(const dom_value& v1, const dom_value& v2)
{
    return !equal(v1, v2);
}

bool json::equal(const dom_value& v1, const dom_value& v2)
{
    return
        v1.type() == v2.type() &&
        v1.text() == v2.text() &&
        (
        (v1.member() == nullptr && v2.member() == nullptr)
            ||
            (v1.member() != nullptr && v2.member() != nullptr && v1.member()->name() == v2.member()->name())
            );
}

bool json::equal(const dom_document& doc1, const dom_document& doc2)
{
    dom_document::const_iterator it1 = doc1.begin();
    dom_document::const_iterator it1_end = doc1.end();
    dom_document::const_iterator it2 = doc2.begin();
    dom_document::const_iterator it2_end = doc2.end();
    while (it1 != it1_end && it2 != it2_end)
    {
        if (!(**it1 == **it2 && it1.path() == it2.path()))
            return false;
        ++it1;
        ++it2;
    }
    return it1 == it1_end && it2 == it2_end;
}

/*
 * dom_value class
 */
dom_value::dom_value(dom_document* const doc, const dom_value_type type)
    : m_doc(doc), m_type(type), m_text(L"")
{
    if (m_doc == nullptr)
        throw dom_exception(L"Value should be created in document scope", dom_error::document_is_null);
}

dom_value::~dom_value()
{
    clear();
}

void dom_value::assert_same_doc(dom_document* doc) const noexcept(false)
{
    if (m_doc != doc)
        throw dom_exception(L"Value is referenced by other document", dom_error::document_is_not_same);
}

void dom_value::assert_no_parent() const noexcept(false)
{
    if (m_parent != nullptr)
        throw dom_exception(L"Value already has a parent", dom_error::parent_is_not_null);
}

void dom_value::assert_same_doc_no_parent(dom_document* doc) const noexcept(false)
{
    assert_same_doc(doc);
    assert_no_parent();
}

void dom_value::parent(dom_value* const value) noexcept (false)
{
    assert_same_doc(value->document());
    m_parent = value;
}

std::wstring dom_value::to_wstring() const
{
    return strutils::wformat(L"Length: %d\ntext: \"%ls\"\nencoded: \"%ls\"",
                                  text().length(),
                                  text().c_str(),
                                  json::to_escaped(text(), true).c_str());
}

/*
 * dom_literal class
 */
dom_literal_type json::to_literal_type(const std::wstring& value) noexcept(false)
{
    if (value == L"false")
        return dom_literal_type::lvt_false;
    else if (value == L"null")
        return dom_literal_type::lvt_null;
    else if (value == L"true")
        return dom_literal_type::lvt_true;
    else
        throw dom_exception(strutils::wformat(L"Invalid literal value '%ls'", value.c_str()), dom_error::invalid_literal);
}

dom_literal::dom_literal(dom_document* const doc, const std::wstring text)
    : dom_value(doc, dom_value_type::vt_literal)
{
    this->text(text);
}

void dom_literal::text(const std::wstring value) noexcept(false)
{
    m_literal_type = json::to_literal_type(value);
    dom_value::text(value);
}

/*
 * dom_number class
 */
std::string json::to_string(const dom_number_type numtype)
{
    switch(numtype)
    {
    case dom_number_type::nvt_float: return "nvt_float";
    case dom_number_type::nvt_int: return "nvt_int";
    default:
        return "unknown";
    };
}

std::wstring json::to_wstring(const dom_number_type numtype)
{
    return strutils::to_wstring(json::to_string(numtype));
}

dom_number::dom_number(dom_document* const doc, const std::wstring& text, const dom_number_type numtype)
    : dom_value(doc, dom_value_type::vt_number)
{
    this->text(text);
    m_numtype = numtype;
}

dom_number::dom_number(dom_document* const doc, const int32_t value)
    : dom_value(doc, dom_value_type::vt_number)
{
    locutils::locale_guard lg(LC_NUMERIC, "C");
    this->text(std::to_wstring(value));
    m_numtype = dom_number_type::nvt_int;
}

dom_number::dom_number(dom_document* const doc, const int64_t value)
    : dom_value(doc, dom_value_type::vt_number)
{
    locutils::locale_guard lg(LC_NUMERIC, "C");
    this->text(std::to_wstring(value));
    m_numtype = dom_number_type::nvt_int;
}

dom_number::dom_number(dom_document* const doc, const double value)
    : dom_value(doc, dom_value_type::vt_number)
{
    this->text(dom_number::to_text(value));
    m_numtype = dom_number_type::nvt_float;
}

std::wstring dom_number::to_text(const double value)
{
    locutils::locale_guard lg(LC_NUMERIC, "C");
    wstring s = strutils::wformat(L"%g", value);
    if (s.find(L'.') == s.npos && s.find(L'e') == s.npos && s.find(L'E') == s.npos)
        s += L".0";
    return s;
}

std::wstring dom_number::to_wstring() const
{
    return strutils::wformat(L"%ls\nnumtype: %ls",
                             dom_value::to_wstring().c_str(),
                             json::to_wstring(numtype()).c_str());
}

/*
 * dom_string class
 */
dom_string::dom_string(dom_document* const doc, const wchar_t* text)
    : dom_value(doc, dom_value_type::vt_string)
{
    this->text(text);
}

dom_string::dom_string(dom_document* const doc, const std::wstring& text)
    : dom_string(doc, text.c_str())
{ }

/*
 * dom_object_member class
 */
dom_object_member::dom_object_member(dom_object_members* const owner, const name_t& name, dom_value* const value)
    : m_owner(owner), m_name(json::to_unescaped(name)), m_value(value)
{
    if (m_owner == nullptr)
        throw dom_exception(L"Member owner is null", dom_error::owner_is_null);
}

dom_object_member::~dom_object_member()
{
    if (m_value != nullptr)
        delete m_value;
}

/*
 * dom_object_members class
 */

dom_object_members::dom_object_members(dom_object* const owner)
    : m_owner(owner)
{
    if (m_owner == nullptr)
        throw dom_exception(L"Member list should be owned by an object", dom_error::owner_is_null);
}

void dom_object_members::append(const name_t name, dom_value* const value) noexcept(false)
{
    value->assert_same_doc(m_owner->document());
    check_name(name);
    unique_ptr<dom_object_member> member(new dom_object_member(this, name, value));
    value->parent(m_owner);
    value->m_member = member.get();
    m_index[member->name()] = member.get();
    m_data.push_back(member.release());
}

void dom_object_members::check_name(const name_t name) const noexcept(false)
{
    if (m_index.find(name) != m_index.end())
        throw dom_exception(strutils::wformat(L"Duplicate name '%ls'", name.c_str()), dom_error::duplicate_name);
}

void dom_object_members::clear() noexcept
{
    m_index.clear();
    m_data.clear();
}

dom_object_member* dom_object_members::find(const name_t name) const noexcept
{
    data_index_t::const_iterator it = m_index.find(json::to_unescaped(name));
    if (it == m_index.end())
        return nullptr;
    return it->second;
}

dom_object_member* dom_object_members::get(const name_t name) const noexcept(false)
{
    dom_object_member* member = this->find(name);
    if (member == nullptr)
        throw json::dom_member_not_found(name);
    return member;
}

/*
 * dom_object class
 */
dom_object::dom_object(dom_document* const doc)
    : dom_value(doc, dom_value_type::vt_object),
    m_members(new json::dom_object_members(this))
{ }

dom_object::~dom_object()
{
    if (m_members != nullptr)
        delete m_members;
}

void dom_object::clear()
{
    m_members->clear();
    dom_value::clear();
}

dom_value* dom_object::find_value(const name_t name) const noexcept
{
    dom_object_member* member = this->find(name);
    if (member != nullptr)
        return member->value();
    return nullptr;
}

/*
 * dom_array class
 */
dom_array::dom_array(dom_document* const doc)
    : dom_value(doc, dom_value_type::vt_array),
    m_data(new data_t())
{ }

dom_array::~dom_array()
{
    if (m_data != nullptr)
        delete m_data;
}

void dom_array::append(dom_value* const value) noexcept
{
    value->parent(this);
    m_data->push_back(value);
}

/*
 * dom_document class
 */
dom_document::dom_document(dom_document&& source)
{
    *this = std::move(source);
}

dom_document& dom_document::operator =(dom_document&& source)
{
    m_root = source.m_root;
    source.m_root = nullptr;
    return *this;
}


dom_document::~dom_document()
{
    clear();
}

void dom_document::clear()
{
    if (m_root != nullptr)
        delete m_root;
    m_root = nullptr;
}

dom_array* dom_document::create_array()
{
    return new dom_array(this);
}

dom_literal* dom_document::create_literal(const std::wstring text)
{
    return new dom_literal(this, text);
}

dom_number* dom_document::create_number(const std::wstring text, const json::dom_number_type numtype)
{
    return new dom_number(this, text, numtype);
}

dom_number* dom_document::create_number(const int32_t value)
{
    return new dom_number(this, value);
}

dom_number* dom_document::create_number(const int64_t value)
{
    return new dom_number(this, value);
}

dom_number* dom_document::create_number(const double value)
{
    return new dom_number(this, value);
}

dom_object* dom_document::create_object()
{
    return new dom_object(this);
}

dom_string* dom_document::create_string(const wchar_t* text)
{
    return new dom_string(this, text);
}

dom_string* dom_document::create_string(const std::wstring& text)
{
    return new dom_string(this, text);
}

void dom_document::root(dom_value* const value) noexcept(false)
{
    value->assert_same_doc_no_parent(this);
    clear();
    m_root = value;
}

dom_document::iterator dom_document::begin()
{
    return iterator(this);
}
dom_document::const_iterator dom_document::begin() const
{
    return const_iterator(this);
}

dom_document::iterator dom_document::end()
{
    iterator it(this);
    it.m_current = nullptr;
    return it;
}
dom_document::const_iterator dom_document::end() const
{
    const_iterator it(this);
    it.m_current = nullptr;
    return it;
}

/*
* dom_document::iterator class
*/
dom_document::const_iterator::const_iterator(const dom_document& doc)
    : const_iterator(&doc)
{ }

dom_document::const_iterator::const_iterator(const dom_document* doc)
    : m_doc(doc)
{
    m_current = m_doc->root();
    if (m_current != nullptr)
        m_path.push_back(0);
}

bool dom_document::const_iterator::has_prev_sibling() const noexcept
{
    return !m_path.empty() && m_path.back() > 0;
}

dom_value* dom_document::const_iterator::next()
{
    if (m_current != nullptr)
    {
        container_intf* idx = dynamic_cast<container_intf*>(m_current);
        if (idx != nullptr && idx->count() > 0)
        {
            m_path.push_back(0);
            m_current = idx->get_value(0);
        }
        else
        {
            m_current = m_current->parent();
            while (m_current != nullptr)
            {
                std::size_t next_index = m_path.back() + 1;
                m_path.pop_back();
                idx = dynamic_cast<container_intf*>(m_current);
                if (idx != nullptr && next_index < idx->count())
                {
                    m_path.push_back(next_index);
                    m_current = idx->get_value(next_index);
                    break;
                }
                m_current = m_current->parent();
            }
        }
        if (m_current == nullptr)
            m_path.clear();
    }
    return m_current;
}

