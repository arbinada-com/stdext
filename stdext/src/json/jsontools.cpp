/*
 C++ standard library extensions
 (c) 2001-2020 Serguei Tarassov (see license.txt)
 */

#include "jsontools.h"
#include "jsonparser.h"
#include <stack>
#include <sstream>

using namespace std;
using namespace stdext;
using namespace json;
using namespace locutils;


/*
 * dom_append_child_visitor class
 */
class dom_append_child_visitor : public dom_value_visitor
{
public:
    typedef dom_object_member::name_t name_t;
public:
    dom_append_child_visitor(dom_value* child)
        : dom_value_visitor(), m_child(child)
    {}
    dom_append_child_visitor(name_t name, dom_value* child)
        : dom_value_visitor(), m_name(name), m_child(child)
    {}
public:
    virtual void visit(json::dom_literal&) override {}
    virtual void visit(json::dom_number&) override {}
    virtual void visit(json::dom_string&) override {}
    void visit(dom_array& value) override
    {
        value.append(m_child);
    }
    void visit(dom_object& value) override
    {
        value.append_member(m_name, m_child);
    }
private:
    name_t m_name;
    dom_value* m_child;
};

/*
 * dom_document_writer_visitor class
 */
class dom_document_writer_visitor : public dom_value_visitor
{
public:
    typedef stack<wstring> endings_t;
public:
    dom_document_writer_visitor(ioutils::text_writer& text_writer,
                                endings_t& endings)
        : dom_value_visitor(),
          m_text_writer(text_writer),
          m_endings(endings)
    {}
public:
    virtual void visit(json::dom_literal& value) override { m_text_writer.write(value.text()); }
    virtual void visit(json::dom_number& value) override { m_text_writer.write(value.text()); }
    virtual void visit(json::dom_string& value) override
    {
        m_text_writer.write(L"\"").write(json::to_escaped(value.text())).write(L"\"");
    }
    void visit(dom_array& value) override
    {
        m_text_writer.write(L"[");
        if (value.empty())
            m_text_writer.write(L"]");
        else
            m_endings.push(L"]");
    }
    void visit(dom_object& value) override
    {
        m_text_writer.write(L"{");
        if (value.cmembers()->empty())
            m_text_writer.write(L"}");
        else
            m_endings.push(L"}");
    }
private:
    ioutils::text_writer& m_text_writer;
    endings_t& m_endings;
};

/*
 * dom_document_reader class
 */
dom_document_reader::dom_document_reader(json::dom_document& doc)
    : m_doc(doc)
{}

bool dom_document_reader::read(ioutils::text_reader& reader)
{
    m_messages.clear();
    m_doc.clear();
    json::parser parser(reader, m_messages, m_doc);
    bool result = parser.run();
    return result;
}

bool dom_document_reader::read(std::wistream& stream, const std::wstring& source_name)
{
    ioutils::text_reader reader(stream, source_name);
    return read(reader);
}

bool dom_document_reader::read(const std::wstring& ws, const std::wstring& source_name)
{
    wstringstream ss;
    ss << ws;
    return read(ss, source_name);
}

bool dom_document_reader::read_from_file(const std::wstring file_name, const ioutils::text_io_options& options)
{
    ioutils::text_reader reader(file_name, options);
    return read(reader);
}


/*
 * dom_document_writer class
 */
dom_document_writer::dom_document_writer(json::dom_document& doc)
    : m_doc(doc)
{ }

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
    dom_document::iterator doc_begin = m_doc.begin();
    dom_document::iterator doc_end = m_doc.end();
    dom_document::iterator it = doc_begin;
    while (it != doc_end)
    {
        if (it.has_prev_sibling())
            w.write(L",");
        if (m_conf.pretty_print() && it != doc_begin)
            w.write_endl();
        if (m_conf.pretty_print())
            w.write(indent(it.level()));
        dom_value* v = *it;
        if (v->member() != nullptr)
            w.write(L"\"").write(json::to_escaped(v->member()->name())).write(L"\"").write(m_conf.pretty_print() ? L": " : L":");
        dom_document_writer_visitor visitor(w, endings);
        v->accept(visitor);
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

void dom_document_writer::write(std::wstring& ws)
{
    wstringstream ss;
    write(ss);
    ws = ss.str();
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
    int level_count = conf().depth();
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
    bool is_last_level = curr_level == level_count;
    bool no_more_containers = is_last_level;
    bool container_required = !is_last_level;
    for (dom_value* parent : parents)
    {
        containers_t conts;
        int value_count = m_rnd.random_range(1, conf().avg_children() * 2);
        for (int i = 0; i < value_count; i++)
        {
            dom_value* child;
            if (i == value_count - 1 && conts.empty() && container_required)
                child = generate_value(random_value_container_type());
            else
            {
                if (no_more_containers)
                    child = generate_value(random_value_scalar_type());
                else
                    child = generate_value(random_value_type());
            }
            if (child->is_container())
                conts.push_back(child);
            wstring name = m_rnd.random_wstring(3, 32, m_config.name_char_range());
            dom_append_child_visitor creator(name, child);
            parent->accept(creator);
        }
        if (!is_last_level)
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
        return m_doc.create_number(std::pow(10.0, m_rnd.random_range(-20, 20)) * (m_rnd.random_float() - 0.5));
    case dom_value_type::vt_object:
        return generate_object();
    case dom_value_type::vt_string:
    default:
        return m_doc.create_string(m_rnd.random_wstring(conf().avg_string_length(), conf().value_char_range()));
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

json::dom_value_type dom_document_generator::random_value_scalar_type()
{
    switch (m_rnd.random_range(1, 3))
    {
    case 1: return dom_value_type::vt_literal;
    case 2: return dom_value_type::vt_number;
    default: return dom_value_type::vt_string;
    }
}

json::dom_value_type dom_document_generator::random_value_container_type()
{
    if (m_rnd.random_range(1, 2) == 1)
        return dom_value_type::vt_array;
    return dom_value_type::vt_object;
}

/*
 * Document diff
 */
std::string json::to_string(const dom_document_diff_kind value)
{
    switch(value)
    {
    case dom_document_diff_kind::count_diff: return "count";
    case dom_document_diff_kind::path_diff: return "path";
    case dom_document_diff_kind::type_diff: return "type";
    case dom_document_diff_kind::value_diff: return "value";
    default: return "unknown";
    }
}

std::string json::dom_document_diff_item::to_string() const
{
    return strutils::to_string(this->to_wstring());
}

std::wstring json::dom_document_diff_item::to_wstring() const
{
    return strutils::wformat(L"Kind: %s\nLeft value:\n\tlength: %d\n\ttext: %ls\n\tencoded:%ls\nRight value:\n\tlength: %d\n\ttext: %ls\n\tencoded:%ls",
                             json::to_string(m_kind).c_str(),
                             m_lval->text().length(),
                             m_lval->text().c_str(),
                             json::to_escaped(m_lval->text(), true).c_str(),
                             m_rval->text().length(),
                             m_rval->text().c_str(),
                             json::to_escaped(m_rval->text(), true).c_str());
}


json::dom_document_diff json::make_diff(const json::dom_document& ldoc, const json::dom_document& rdoc)
{
    json::dom_document_diff_options options;
    return json::make_diff(ldoc, rdoc, options);
}

json::dom_document_diff json::make_diff(const json::dom_document& ldoc,
                                        const json::dom_document& rdoc,
                                        const dom_document_diff_options& options)
{
    json::dom_document_diff diff;
    dom_document::const_iterator l_it = ldoc.begin();
    dom_document::const_iterator l_end = ldoc.end();
    dom_document::const_iterator r_it = rdoc.begin();
    dom_document::const_iterator r_end = rdoc.end();
    while (l_it != l_end && r_it != r_end && (!diff.has_differences() || options.compare_all()))
    {
        if (l_it->type() != r_it->type())
            diff.append(dom_document_diff_item(dom_document_diff_kind::type_diff, l_it.value(), r_it.value()));
        if (l_it.path() != r_it.path())
            diff.append(dom_document_diff_item(dom_document_diff_kind::path_diff, l_it.value(), r_it.value()));
        if (l_it->is_container())
        {
            if (l_it->as_container()->count() != r_it->as_container()->count())
                diff.append(dom_document_diff_item(dom_document_diff_kind::count_diff, l_it.value(), r_it.value()));
        }
        if (l_it->member() != nullptr)
        {
            bool are_equal = options.case_sensitive() ?
                (l_it->member()->name() == r_it->member()->name()) :
                utf16::equal_ci(l_it->member()->name(), r_it->member()->name());
            if (!are_equal)
                diff.append(dom_document_diff_item(dom_document_diff_kind::member_name_diff, l_it.value(), r_it.value()));
        }
        bool are_equal = options.case_sensitive() ?
                    (l_it->text() == r_it->text()) :
                    utf16::equal_ci(l_it->text(), r_it->text());
        if (!are_equal)
            diff.append(dom_document_diff_item(dom_document_diff_kind::value_diff, l_it.value(), r_it.value()));
        ++l_it;
        ++r_it;
    }
    return diff;
}
