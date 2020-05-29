/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "jsontools.h"
#include <stack>
#include <sstream>

using namespace std;
using namespace stdext;
using namespace json;
using namespace locutils;

/*
 * dom_document_writer class
 */
dom_document_writer::dom_document_writer(const json::dom_document& doc)
    : m_doc(doc)
{ }

std::wstring dom_document_writer::escape(const std::wstring s) const
{
    wstring es;
    es.reserve(s.length());
    wchar_t high_surrogate = 0;
    for (const wchar_t& c : s)
    {
        if (high_surrogate != 0)
        {
            if (utf16::is_low_surrogate(c))
                es += json::to_escaped(high_surrogate, true) + json::to_escaped(c, true);
            else
            {
                es += utf16::replacement_character; // invalid high surrogate detected
                es += c;
            }
            high_surrogate = 0;
        }
        else
        {
            if (json::is_unescaped(c))
            {
                if (utf16::is_high_surrogate(c))
                    high_surrogate = c;
                else
                    es += c;
            }
            else
            {
                if (utf16::is_noncharacter(c))
                    es += json::to_escaped(utf16::replacement_character);
                else
                    es += json::to_escaped(c);
            }
        }
    }
    if (high_surrogate != 0)
        es += high_surrogate;
    return es;
}

void dom_document_writer::write(ioutils::text_writer& w)
{
    auto indent = [](size_t n) -> wstring
    {
        wstring s;
        if (n > 1)
            s.insert(s.begin(), n - 1, L'\t');
        return s;
    };
    stack<wstring> endings;
    dom_document::const_iterator doc_begin = m_doc.begin();
    dom_document::const_iterator doc_end = m_doc.end();
    dom_document::const_iterator it = doc_begin;
    while (it != doc_end)
    {
        if (it.has_prev_sibling())
            w.write(L",");
        if (m_conf.pretty_print() && it != doc_begin)
            w.write_endl();
        if (m_conf.pretty_print())
            w.write(indent(it.level()));
        const dom_value* v = *it;
        if (v->member() != nullptr)
            w.write(L"\"").write(v->member()->name()).write(L"\"").write(m_conf.pretty_print() ? L": " : L":");
        switch (v->type())
        {
        case dom_value_type::vt_literal:
        case dom_value_type::vt_number:
            w.write(v->text());
            break;
        case dom_value_type::vt_string:
            w.write(L"\"").write(escape(v->text())).write(L"\"");
            break;
        case dom_value_type::vt_array:
            w.write(L"[");
            if (dynamic_cast<const json::dom_array*>(v)->empty())
                w.write(L"]");
            else
                endings.push(L"]");
            break;
        case dom_value_type::vt_object:
            w.write(L"{");
            if (dynamic_cast<const json::dom_object*>(v)->cmembers()->empty())
                w.write(L"}");
            else
                endings.push(L"}");
            break;
        }
        dom_document::const_iterator::path_t prev_path = it.path();
        ++it;
        size_t curr_level = it.level();
        for (size_t i = prev_path.size(); i > curr_level; i--)
        {
            if (!endings.empty())
            {
                if (m_conf.pretty_print())
                    w.write_endl().write(indent(i - 1));
                w.write(endings.top());
                endings.pop();
            }
        }
    }
}

void dom_document_writer::write(std::wostream& stream)
{
    ioutils::text_writer w(stream);
    write(w);
}

void dom_document_writer::write(std::wstring& s)
{
    wstringstream ss;
    write(ss);
    s = ss.str();
}

void dom_document_writer::write_to_file(const std::wstring file_name, const ioutils::text_io_options& options)
{
    ioutils::text_writer w(file_name, options);
    write(w);
}


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
