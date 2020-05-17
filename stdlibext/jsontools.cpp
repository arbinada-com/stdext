/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "jsontools.h"

using namespace std;
using namespace stdext;
using namespace json;

/*
 * dom_document_generator class
 */
dom_document_generator::dom_document_generator(json::dom_document& doc)
    : m_doc(doc)
{ }

void dom_document_generator::run()
{
    int level_count = m_rnd.random_range(conf().min_depth(), conf().max_depth());
    if (level_count < 1)
        level_count = 1;
    if (level_count == 1)
    {
        m_doc.root(generate_value(random_value_type()));
    }
    else
    {
        containers_t conts;
        conts.push_back(generate_value(random_value_container_type()));
        m_doc.root(conts.back());
        generate_level(2, level_count, conts);
    }
}

void dom_document_generator::generate_level(const int curr_level, const int level_count, const containers_t& parents)
{
    if (curr_level >= level_count)
        return;
    bool container_required = (curr_level < level_count);
    for (dom_value* cont : parents)
    {
        containers_t conts;
        int value_count = m_rnd.random_range(conf().min_depth(), conf().max_depth());
        for (int i = 0; i < value_count; i++)
        {
            dom_value_type type = random_value_type();
            if (container_required && conts.empty() && i == value_count - 1)
                type = random_value_container_type();
            dom_value* value = generate_value(type);
            if (value->is_container())
                conts.push_back(value);
            if (cont->type() == dom_value_type::vt_array)
                dynamic_cast<dom_array*>(cont)->append(value);
            else
            { 
                wstring name = m_rnd.random_wstring(3, 32, testutils::rnd_helper::char_range(0x21, 0xFF));
                dynamic_cast<dom_object*>(cont)->append_member(name, value);
            }
        }
        generate_level(curr_level + 1, level_count, conts);
    }
}

json::dom_value* dom_document_generator::generate_array()
{
    dom_array* arr = m_doc.create_array();
    return arr;
}

json::dom_value* dom_document_generator::generate_object()
{
    dom_object* obj = m_doc.create_object();
    return obj;
}

json::dom_value* dom_document_generator::generate_value(json::dom_value_type type)
{
    switch (type)
    {
    case dom_value_type::vt_array:
        return generate_array();
    case dom_value_type::vt_literal:
        return m_doc.create_literal(random_literal_name());
    case dom_value_type::vt_number:
        if (m_rnd.random_bool())
            return m_doc.create_number(m_rnd.random_range(numeric_limits<int>::min(), numeric_limits<int>::max()));
        return m_doc.create_number(1E10 * (m_rnd.random_float() - 0.5));
    case dom_value_type::vt_object:
        return generate_object();
    default: //dom_value_type::vt_string
        return m_doc.create_string(m_rnd.random_wstring(conf().avg_string_length()));
    }
}

std::wstring dom_document_generator::random_literal_name()
{
    switch (m_rnd.random_range(1, 3))
    {
    case 1: return L"false";
    case 2: return L"null";
    default: return L"true";
    }
}

json::dom_value_type dom_document_generator::random_value_type()
{
    switch (m_rnd.random_range(1, 5))
    {
    case 1: return dom_value_type::vt_array;
    case 2: return dom_value_type::vt_literal;
    case 3: return dom_value_type::vt_number;
    case 4: return dom_value_type::vt_object;
    default: return dom_value_type::vt_string;
    }
}

json::dom_value_type dom_document_generator::random_value_container_type()
{
    if (m_rnd.random_range(1, 2) == 1)
        return dom_value_type::vt_array;
    return dom_value_type::vt_object;
}
