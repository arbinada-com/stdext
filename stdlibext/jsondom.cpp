/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "jsondom.h"
#include <memory>
#include "strutils.h"

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
    case dom_value_type::vt_unknown: return "unknown";
    default:
        return "unsupported";
    }
}
std::wstring json::to_wstring(const dom_value_type type)
{
    return strutils::to_wstring(json::to_string(type));
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

void dom_value::parent(dom_value* const value) noexcept 
{ 
    assert_same_doc(value->document());
    m_parent = value;
}

/*
 * dom_literal class
 */
dom_literal::dom_literal(dom_document* const doc, const std::wstring text)
    : dom_value(doc, dom_value_type::vt_literal)
{
    this->text(text);
}

void dom_literal::text(const std::wstring value) noexcept(false)
{
    if (!(value == L"true" || value == L"false" || value == L"null"))
        throw dom_exception(strutils::format(L"Invalid literal value '%s'", value.c_str()), dom_error::invalid_literal);
    dom_value::text(value);
}

/*
 * dom_number class
 */
dom_number::dom_number(dom_document* const doc, const std::wstring text)
    : dom_value(doc, dom_value_type::vt_number)
{
    this->text(text);
}

/*
 * dom_string class
 */
dom_string::dom_string(dom_document* const doc, const std::wstring text)
    : dom_value(doc, dom_value_type::vt_string)
{
    this->text(text);
}

/*
 * dom_object_member class
 */
dom_object_member::dom_object_member(dom_object_members* const owner, const name_t name, dom_value* const value)
    : m_owner(owner), m_name(name), m_value(value)
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
    m_index[member->name()] = member.get();
    m_data.push_back(member.release());
}

void dom_object_members::check_name(const name_t name) const noexcept(false)
{
    if (m_index.find(name) != m_index.end())
        throw dom_exception(strutils::format(L"Duplicate name '%s'", name.c_str()), dom_error::duplicate_name);
}

void dom_object_members::clear() noexcept
{
    m_index.clear();
    m_data.clear();
}

dom_value* const dom_object_members::find(const name_t name) const noexcept
{
    data_index_t::const_iterator it = m_index.find(name);
    if (it == m_index.end())
        return nullptr;
    return it->second->value();
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

void stdext::json::dom_object::clear()
{
    m_members->clear();
    dom_value::clear();
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

dom_value* const dom_document::create_value(const dom_value_type type, const std::wstring text)
{
    unique_ptr<dom_value> value;
    switch (type)
    {
    case dom_value_type::vt_array:
        value.reset(new dom_array(this));
        break;
    case dom_value_type::vt_literal:
        value.reset(new dom_literal(this, text));
        break;
    case dom_value_type::vt_number:
        value.reset(new dom_number(this, text));
        break;
    case dom_value_type::vt_object:
        value.reset(new dom_object(this));
        break;
    case dom_value_type::vt_string:
        value.reset(new dom_string(this, text));
        break;
    default:
        throw dom_exception(
            strutils::format(L"Unsupported value type: %s (%d)", json::to_wstring(type).c_str(), static_cast<int>(type)), 
            dom_error::usupported_value_type);
    }
    return value.release();
}

dom_array* const dom_document::create_array()
{
    return dynamic_cast<dom_array*>(create_value(dom_value_type::vt_array, L""));
}

dom_literal* const dom_document::create_literal(const std::wstring text)
{
    return dynamic_cast<dom_literal*>(create_value(dom_value_type::vt_literal, text));
}

dom_number* const dom_document::create_number(const std::wstring text)
{
    return dynamic_cast<dom_number*>(create_value(dom_value_type::vt_number, text));
}

dom_object* const dom_document::create_object()
{
    return dynamic_cast<dom_object*>(create_value(dom_value_type::vt_object, L""));
}

dom_string* const dom_document::create_string(const std::wstring text)
{
    return dynamic_cast<dom_string*>(create_value(dom_value_type::vt_string, text));
}

void dom_document::root(dom_value* const value) noexcept(false)
{
    value->assert_same_doc_no_parent(this);
    clear();
    m_root = value;
}