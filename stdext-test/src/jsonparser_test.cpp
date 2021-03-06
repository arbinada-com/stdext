﻿#include <gtest/gtest.h>
#include "json.h"
#include <fstream>
#include <limits>
#include <stack>
#include "strutils.h"
#include "testutils.h"

using namespace std;

namespace stdext
{
namespace json_parser_test
{

class Handler : public json::sax_handler_intf//, public testing::Test
{
public:
    typedef std::stack<json::dom_object*> object_stack_t;
    typedef std::stack<json::dom_array*> array_stack_t;
public:
    Handler(json::dom_document& expected, const std::wstring& title)
        : m_expected(expected), m_it(m_expected.begin()), m_title(title)
    {}
public:
    virtual void on_literal(const json::dom_literal_type type, const std::wstring& text) override
    {
        wstring title = m_title + L": on_literal";
        ASSERT_TRUE(m_it != m_expected.end()) << title;
        EXPECT_EQ(m_it->type(), json::dom_value_type::vt_literal) << title;
        EXPECT_EQ(m_it->text(), text) << title;
        EXPECT_EQ(dynamic_cast<json::dom_literal*>(*m_it)->literal_type(), type) << title;
        m_it++;
    }
    virtual void on_number(const json::dom_number_type type, const std::wstring& text) override
    {
        wstring title = m_title + L": on_number";
        ASSERT_TRUE(m_it != m_expected.end()) << title;
        title += L" " + text;
        ASSERT_EQ(m_it->type(), json::dom_value_type::vt_number) << title;
        EXPECT_EQ(m_it->text(), text) << title;
        if (dynamic_cast<json::dom_number*>(*m_it)->numtype() != type)
        {
            EXPECT_TRUE(dynamic_cast<json::dom_number*>(*m_it)->numtype() == type) << title;
        }
        m_it++;
    }
    virtual void on_string(const std::wstring& text) override
    {
        wstring title = m_title + L": on_string";
        ASSERT_TRUE(m_it != m_expected.end()) << title;
        EXPECT_EQ(m_it->type(), json::dom_value_type::vt_string) << title;
        EXPECT_EQ(m_it->text(), text) << title;
        m_it++;
    }
    virtual void on_begin_object() override
    {
        wstring title = m_title + L": on_begin_object";
        ASSERT_TRUE(m_it != m_expected.end()) << title;
        ASSERT_EQ(m_it->type(), json::dom_value_type::vt_object) << title;
        EXPECT_EQ(m_it->text(), L"") << title;
        m_objs.push(dynamic_cast<json::dom_object*>(*m_it));
        m_it++;
    }
    virtual void on_member_name(const std::wstring& text) override
    {
        wstring title = m_title + L": on_member_name";
        ASSERT_TRUE(m_it != m_expected.end()) << title;
        ASSERT_TRUE(m_it->member() != nullptr) << title;
        EXPECT_EQ(m_it->member()->name(), text) << title;
    }
    virtual void on_end_object(const std::size_t member_count) override
    {
        wstring title = m_title + L": on_end_object";
        ASSERT_FALSE(m_objs.empty()) << title;
        EXPECT_EQ(m_objs.top()->members()->size(), member_count);
        m_objs.pop();
    }
    virtual void on_begin_array() override
    {
        wstring title = m_title + L": on_begin_array";
        ASSERT_TRUE(m_it != m_expected.end()) << title;
        ASSERT_EQ(m_it->type(), json::dom_value_type::vt_array) << title;
        EXPECT_EQ(m_it->text(), L"") << title;
        m_arrays.push(dynamic_cast<json::dom_array*>(*m_it));
        m_it++;
    }
    virtual void on_end_array(const std::size_t element_count) override
    {
        wstring title = m_title + L": on_end_array";
        ASSERT_FALSE(m_arrays.empty()) << title;
        EXPECT_EQ(m_arrays.top()->size(), element_count);
        m_arrays.pop();
    }
    virtual void textpos_changed(const parsers::textpos&) override {}
private:
    json::dom_document& m_expected;
    json::dom_document::iterator m_it;
    std::wstring m_title;
    object_stack_t m_objs;
    array_stack_t m_arrays;
};


/*
 * SAX and DOM parsers tests
 */
class JsonParserTest : public testing::Test
{
protected:
    void CheckParseText(wstring input, json::dom_document& expected, wstring title)
    {
        CheckParseTextSax(input, expected, title + L" [SAX]");
        CheckParseTextDom(input, expected, title + L" [DOM]");
    }

    void CheckParseTextSax(wstring input, json::dom_document& expected, wstring title)
    {
        wstring title2 = title + L": ";
        wstringstream ss(input);
        ioutils::text_reader r(ss);
        json::msg_collector_t mc;
        Handler handler(expected, title);
        json::sax_parser parser(r, mc, handler);
        bool result = parser.run();
        wstring err_text;
        if (parser.has_errors())
        {
            for (json::message_t* err : parser.messages().errors())
                err_text += L"\n" + err->to_wstring();
        }
        if (parser.has_errors() || this->HasFailure())
        {
            ioutils::text_writer w1(L"parser_current.json", ioutils::text_io_policy_utf8());
            w1.write(input);
            json::dom_document_writer w2(expected);
            w2.conf().pretty_print(true);
            w2.write_to_file(L"parser_expected.json", ioutils::text_io_policy_utf8());
        }
        if (result && parser.has_errors())
            FAIL() << title2 + L"OK with errors:" + err_text;
        if (!result && !parser.has_errors())
            FAIL() << title2 + L"failed without errors:" + err_text;
        EXPECT_FALSE(parser.has_errors()) << title2 + L"errors:" + err_text;
    }

    void CheckParseTextDom(wstring input, json::dom_document& expected, wstring title)
    {
        wstring title2 = title + L": ";
        wstringstream ss(input);
        ioutils::text_reader r(ss);
        json::msg_collector_t mc;
        json::dom_document doc;
        json::dom_parser parser(r, mc, doc);
        bool result = parser.run();
        wstring err_text;
        if (parser.has_errors())
        {
            for (json::message_t* err : parser.messages().errors())
                err_text += L"\n" + err->to_wstring();
        }
        if (result && parser.has_errors())
            FAIL() << title2 + L"OK with errors:" + err_text;
        if (!result && !parser.has_errors())
            FAIL() << title2 + L"failed without errors:" + err_text;
        EXPECT_FALSE(parser.has_errors()) << title2 + L"errors:" + err_text;
        json::dom_document_diff diff = json::make_diff(doc, expected);
        if (diff.has_differences())
        {
            json::dom_document_writer w1(doc);
            w1.conf().pretty_print(true);
            w1.write_to_file(L"parser_current.json", ioutils::text_io_policy_utf8());
            json::dom_document_writer w2(expected);
            w2.conf().pretty_print(true);
            w2.write_to_file(L"parser_expected.json", ioutils::text_io_policy_utf8());
            wstring msg = L"Docs are different. ";
            for (const json::dom_document_diff_item& item : diff.items())
            {
                msg += item.to_wstring() + L"\n";
            }
            FAIL() << title2 + msg;
        }
    }


    void CheckError(const wstring input, const json::parser_msg_kind kind, const parsers::textpos& pos, const wstring title)
    {
        wstringstream ss(input);
        ioutils::text_reader r(ss);
        r.source_name(L"ChkErrStream");
        json::msg_collector_t mc;
        json::dom_document doc;
        json::dom_parser parser(r, mc, doc);
        wstring title2 = title + L": ";
        ASSERT_FALSE(parser.run()) << title2 + L"parsed OK";
        ASSERT_TRUE(parser.has_errors()) << title2 + L"no errors";
        json::message_t* err = parser.messages().errors()[0];
        ASSERT_TRUE(parsers::msg_origin::parser == err->origin()) << title2 + L"origin. " + err->text();
        ASSERT_EQ((int)kind, (int)err->kind()) << title2 + L"kind. " + err->text();
        ASSERT_FALSE(err->text().empty()) << title2 + L"text is empty";
        ASSERT_EQ(pos, err->pos()) << title2 + L"error pos. " + err->text();
        ASSERT_EQ(r.source_name(), err->source()) << title2 + L"source. " + err->text();
    }
};

TEST_F(JsonParserTest, TestSimpleDoc)
{
    json::dom_document doc;
    CheckParseText(L"", doc, L"Empty text 1");
    CheckParseText(L"\t\r\n\t", doc, L"Empty text 2");
    //
    doc.clear();
    doc.root(doc.create_number(123));
    CheckParseText(L"123", doc, L"Num 1.1");
    CheckParseText(L"123\t\r\n", doc, L"Num 1.2");
    doc.clear();
    doc.root(doc.create_number(123.456));
    CheckParseText(L"123.456", doc, L"Num 2.1");
    CheckParseText(L"\n\t123.456\n\t\n", doc, L"Num 2.2");
    doc.clear();
    doc.root(doc.create_number(-214659));
    CheckParseText(L"-214659", doc, L"Num 2.3");
    doc.clear();
    doc.root(doc.create_number(-1.23456e8));
    CheckParseText(L"-1.23456e+08", doc, L"Num 3");
    //
    doc.clear();
    doc.root(doc.create_literal(L"false"));
    CheckParseText(L"false", doc, L"Literal 1.1");
    CheckParseText(L"\r\n\t\t false \n\n\t", doc, L"Literal 1.2");
    doc.clear();
    doc.root(doc.create_literal(L"null"));
    CheckParseText(L"null", doc, L"Literal 2");
    doc.clear();
    doc.root(doc.create_literal(L"true"));
    CheckParseText(L"true", doc, L"Literal 3");
    //
    doc.clear();
    doc.root(doc.create_string(L"true"));
    CheckParseText(L"\"true\"", doc, L"Str 1");
    doc.clear();
    doc.root(doc.create_string(L"Hello\nworld"));
    CheckParseText(L"\"Hello\\nworld\"", doc, L"Str 2");
    doc.clear();
    doc.root(doc.create_string(L"\x0001\"\\/\b\f\n\r\t\xD834\xDD1E"));
    CheckParseText(L"\"\\u0001\\\"\\\\/\\b\\f\\n\\r\\t\\uD834\\uDD1E\"", doc, L"Str 3");
    doc.clear();
    doc.root(doc.create_string(L"ABC été déjà строка"));
    CheckParseText(L"\"ABC été déjà строка\"", doc, L"Str 4");
}

TEST_F(JsonParserTest, TestSimpleDocErrors)
{
    using namespace parsers;
    CheckError(L"null\nnull", json::parser_msg_kind::err_unexpected_lexeme_fmt, textpos(2, 1), L"Unexpected lexeme 1");
    CheckError(L"123\n456", json::parser_msg_kind::err_unexpected_lexeme_fmt, textpos(2, 1), L"Unexpected lexeme 2");
    CheckError(L"\"Hello\" \"world\"", json::parser_msg_kind::err_unexpected_lexeme_fmt, textpos(1, 9), L"Unexpected lexeme 3");
}

TEST_F(JsonParserTest, TestArrays)
{
    json::dom_document doc;
    json::dom_array* a1 = doc.create_array();
    doc.root(a1);
    CheckParseText(L"[]", doc, L"1");
    a1->append(doc.create_literal(L"null"));
    a1->append(doc.create_number(12345));
    CheckParseText(L"[null,12345]", doc, L"2");
    json::dom_array* a2 = doc.create_array();
    a1->append(a2);
    CheckParseText(L"[null,12345,[]]", doc, L"3");
    a2->append(doc.create_literal(L"true"));
    a2->append(doc.create_number(123.456));
    CheckParseText(L"[null,12345,[true,123.456]]", doc, L"4");
    json::dom_array* a3 = doc.create_array();
    a2->append(a3);
    CheckParseText(L"[null,12345,[true,123.456,[]]]", doc, L"5");
    a1->append(doc.create_string(L"Hello"));
    CheckParseText(L"[null,12345,[true,123.456,[]],\"Hello\"]", doc, L"6");
}

TEST_F(JsonParserTest, TestArrayErrors)
{
    using namespace parsers;
    // Unclosed array error position is at the end of the last array item or at the start of unexpected lexeme
    CheckError(L"[", json::parser_msg_kind::err_unclosed_array, textpos(1, 1), L"1.1");
    CheckError(L"[null", json::parser_msg_kind::err_unclosed_array, textpos(1, 5), L"1.2");
    CheckError(L"[null,null", json::parser_msg_kind::err_unclosed_array, textpos(1, 10), L"1.3");
    CheckError(L"[null :", json::parser_msg_kind::err_unclosed_array, textpos(1, 7), L"1.4");
    CheckError(L"[null,]", json::parser_msg_kind::err_expected_value_but_found_fmt, textpos(1, 7), L"2.1");
    CheckError(L"[null,:", json::parser_msg_kind::err_expected_value_but_found_fmt, textpos(1, 7), L"2.2");
    CheckError(L"[,", json::parser_msg_kind::err_expected_value_but_found_fmt, textpos(1, 2), L"2.3");
    CheckError(L"[null,", json::parser_msg_kind::err_expected_array_item, textpos(1, 6), L"3.1");
    CheckError(L"[][", json::parser_msg_kind::err_unexpected_lexeme_fmt, textpos(1, 3), L"4.1");
}

TEST_F(JsonParserTest, TestObjects)
{
    json::dom_document doc;
    json::dom_object* o1 = doc.create_object();
    doc.root(o1);
    CheckParseText(L"{}", doc, L"1");
    o1->append_member(L"Literal 1", doc.create_literal(L"null"));
    CheckParseText(L"{\"Literal 1\":null}", doc, L"2.1");
    o1->append_member(L"Number 1", doc.create_number(12345));
    CheckParseText(L"{\"Literal 1\":null,\"Number 1\":12345}", doc, L"2.2");
    json::dom_object* o2 = doc.create_object();
    o1->append_member(L"Obj 1", o2);
    CheckParseText(L"{\"Literal 1\":null,\"Number 1\":12345,\"Obj 1\":{}}", doc, L"3");
    o2->append_member(L"Literal 2", doc.create_literal(L"true"));
    CheckParseText(L"{\"Literal 1\":null,\"Number 1\":12345,\"Obj 1\":{\"Literal 2\":true}}", doc, L"4");
    o1->append_member(L"Str 1", doc.create_string(L"Hello"));
    CheckParseText(L"{\"Literal 1\":null,\"Number 1\":12345,\"Obj 1\":{\"Literal 2\":true},\"Str 1\":\"Hello\"}", doc, L"5");
}

TEST_F(JsonParserTest, TestObjectErrors)
{
    using namespace parsers;
    CheckError(L"{", json::parser_msg_kind::err_unclosed_object, textpos(1, 1), L"1.1");
    CheckError(L"{null", json::parser_msg_kind::err_expected_member_name, textpos(1, 2), L"2.1");
    CheckError(L"{\"Member1\"", json::parser_msg_kind::err_expected_name_separator, textpos(1, 10), L"3.1");
    CheckError(L"{\"Member1\":", json::parser_msg_kind::err_expected_value, textpos(1, 11), L"4.1");
}

TEST_F(JsonParserTest, TestGeneratedDocs)
{
    const int max_test_count = 100;
    for (int i = 1; i <= max_test_count; i++)
    {
        json::dom_document doc;
        json::dom_document_generator gen(doc);
        gen.conf().depth(7);
        gen.conf().avg_children(5);
        gen.run();
        json::dom_document_writer w(doc);
        w.conf().pretty_print(true);
        wstring s;
        w.write(s);
        CheckParseText(s, doc, str::wformat(L"Test_%d", i));
        if (this->HasFailure())
            break;
    }
}

}
}
