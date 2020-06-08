#include <gtest/gtest.h>
#include "json.h"
#include "jsontools.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include "locutils.h"
#include "strutils.h"
#include "testutils.h"

using namespace std;
using namespace stdext;
using namespace locutils;
using namespace testutils;

namespace json_tools_test
{

class JsonToolsTest : public testing::Test
{
protected:
    const wstring default_test_file_name = L"current.json";

    void SaveDoc(json::dom_document& doc, const wstring file_name)
    {
        json::dom_document_writer w(doc);
        w.conf().pretty_print(true);
        w.write_to_file(file_name, ioutils::text_io_options_utf8());
    }

    void CompareDocStrings(const wstring ws, const wstring& expected, const wstring& title)
    {
        EXPECT_EQ(ws.length(), expected.length()) << title + L": length";
        if (ws.length() < 100)
            EXPECT_EQ(expected, ws) << title + L": content";
        for (size_t i = 0; i < ws.length() && i < expected.length(); i++)
        {
            ASSERT_EQ(ws[i], expected[i]) << strutils::wformat(L"%ls: char[%d]", title.c_str(), i);
        }
    }

    void CheckDocFileWriter(json::dom_document& doc, const wstring& expected, const wstring& title)
    {
        const wstring file_name = default_test_file_name;
        SaveDoc(doc, file_name);
        ioutils::text_reader r(file_name, ioutils::text_io_options_utf8());
        wstring s;
        r.read_all(s);
        CompareDocStrings(s, expected, title);
    }

    void CheckDocStringWriter(json::dom_document& doc, const wstring& expected, const wstring& title)
    {
        json::dom_document_writer w(doc);
        w.conf().pretty_print(false);
        wstring s;
        w.write(s);
        CompareDocStrings(s, expected, title);
    }
};

TEST_F(JsonToolsTest, TestDomDocumentWriter_Literal)
{
    json::dom_document doc;
    doc.clear();
    doc.root(doc.create_literal(L"false"));
    CheckDocStringWriter(doc, L"false", L"Str 1");
    CheckDocFileWriter(doc, L"false", L"Doc 1");
    doc.clear();
    doc.root(doc.create_literal(L"null"));
    CheckDocStringWriter(doc, L"null", L"Str 2");
    CheckDocFileWriter(doc, L"null", L"Doc 2");
    doc.clear();
    doc.root(doc.create_literal(L"true"));
    CheckDocStringWriter(doc, L"true", L"Str 3");
    CheckDocFileWriter(doc, L"true", L"Doc 3");
}

TEST_F(JsonToolsTest, TestDomDocumentWriter_Number)
{
    json::dom_document doc;
    doc.clear();
    doc.root(doc.create_number(123456));
    CheckDocStringWriter(doc, L"123456", L"Str 1");
    CheckDocFileWriter(doc, L"123456", L"Doc 2");
    doc.clear();
    doc.root(doc.create_number(123.456));
    CheckDocStringWriter(doc, L"123.456", L"Str 2");
    CheckDocFileWriter(doc, L"123.456", L"Doc 2");
}

TEST_F(JsonToolsTest, TestStringEscape)
{
    json::dom_document doc;
    json::dom_document_writer w(doc);
    EXPECT_EQ(L"", w.escape(L"")) << "1";
    EXPECT_EQ(L"\\\"", w.escape(L"\"")) << "2.1";
    EXPECT_EQ(L"\\\\", w.escape(L"\\")) << "2.2";
    EXPECT_EQ(L"/", w.escape(L"/")) << "2.3";
    EXPECT_EQ(L"\\b", w.escape(L"\b")) << "2.4";
    EXPECT_EQ(L"\\f", w.escape(L"\f")) << "2.5";
    EXPECT_EQ(L"\\n", w.escape(L"\n")) << "2.6";
    EXPECT_EQ(L"\\r", w.escape(L"\r")) << "2.7";
    EXPECT_EQ(L"\\t", w.escape(L"\t")) << "2.8";
    //
    EXPECT_EQ(L"\\u0001", w.escape(L"\x0001")) << "3.1";
    EXPECT_EQ(L"\\u001F", w.escape(L"\x001F")) << "3.2";
    wstring ws = {utf16::replacement_character};
    ASSERT_EQ(L"\xFFFD", w.escape(ws)) << "3.3";
    //
    EXPECT_EQ(L"\\uD834\\uDD1E", w.escape(L"\xD834\xDD1E")) << "Surrogate pair 'G clef' 1";
    EXPECT_EQ(L"==\\uD834\\uDD1E==", w.escape(L"==\xD834\xDD1E==")) << "Surrogate pair 'G clef' 2";
    wstring expected = {utf16::replacement_character};
    EXPECT_EQ(expected, w.escape(L"\xD834")) << "Surrogate pair 2.1";
    expected += L"-\x1234";
    EXPECT_EQ(expected, w.escape(L"\xD834-\x1234")) << "Surrogate pair 2.2";
    expected = {utf16::replacement_character, L'-', utf16::replacement_character};
    EXPECT_EQ(expected, w.escape(L"\xD834-\xDD1E")) << "Surrogate pair 2.3";
}

TEST_F(JsonToolsTest, TestDomDocumentWriter_String)
{
    json::dom_document doc;
    doc.clear();
    doc.root(doc.create_string(L"Hello world!"));
    CheckDocStringWriter(doc, L"\"Hello world!\"", L"Str 1");
    CheckDocFileWriter(doc, L"\"Hello world!\"", L"Doc 1");
    doc.clear();
    doc.root(doc.create_string(L"Quick\r\nbrown\tfox\t\xD834\xDD1E\n"));
    CheckDocStringWriter(doc, L"\"Quick\\r\\nbrown\\tfox\\t\\uD834\\uDD1E\\n\"", L"Str 2");
    CheckDocFileWriter(doc, L"\"Quick\\r\\nbrown\\tfox\\t\\uD834\\uDD1E\\n\"", L"Doc 2");
    doc.clear();
    doc.root(doc.create_string(L"ABC été déjà строка"));
    CheckDocStringWriter(doc, L"\"ABC été déjà строка\"", L"Str 3");
    CheckDocFileWriter(doc, L"\"ABC été déjà строка\"", L"Doc 3");
    doc.clear();
    doc.root(doc.create_string(L"\x1 \x2 \xFFFE \xFFFF"));
    wstring expected = strutils::wformat(L"\"\\u0001 \\u0002 \\u%0.4X \\u%0.4X\"",
                                        utf16::replacement_character, utf16::replacement_character);
    CheckDocStringWriter(doc, expected, L"Str 4");
    CheckDocFileWriter(doc, expected, L"Doc 4");
    //
    wstring s;
    expected.clear();
    for (int i = 0x1; i < 0xFFFF; i++)
    {
        wchar_t c = static_cast<wchar_t>(i);
        if (utf16::is_high_surrogate(c) || utf16::is_low_surrogate(c))
            continue;
        if (utf16::is_noncharacter(c))
            c = utf16::replacement_character;
        s += c;
        if (json::is_unescaped(c))
            expected += c;
        else
            expected += json::to_escaped(c);
    }
    expected = strutils::double_quoted(expected);
    doc.clear();
    doc.root(doc.create_string(s));
    CheckDocStringWriter(doc, expected, L"Str 5");
    CheckDocFileWriter(doc, expected, L"Doc 5");
    //
    s = {utf16::high_surrogate_min};
    expected = {L'"', utf16::replacement_character, L'"'};
    doc.clear();
    doc.root(doc.create_string(s));
    CheckDocStringWriter(doc, expected, L"Str 6.1");
    CheckDocFileWriter(doc, expected, L"Doc 6.1");
    s = {utf16::high_surrogate_min, L'A'};
    expected = {L'"', utf16::replacement_character, L'A', L'"'};
    doc.clear();
    doc.root(doc.create_string(s));
    CheckDocStringWriter(doc, expected, L"Str 6.2");
    CheckDocFileWriter(doc, expected, L"Doc 6.2");
}

TEST_F(JsonToolsTest, TestDomDocumentWriter_Array)
{
    json::dom_document doc;
    doc.clear();
    doc.root(doc.create_array());
    CheckDocStringWriter(doc, L"[]", L"Str 1");
    CheckDocFileWriter(doc, L"[]", L"Doc 1");
    //
    doc.clear();
    json::dom_array* a = doc.create_array();
    doc.root(a);
    a->append(doc.create_array());
    CheckDocStringWriter(doc, L"[[]]", L"Str 2");
    CheckDocFileWriter(doc, L"[\n\t[]\n]", L"Doc 2");
    a->append(doc.create_array());
    CheckDocStringWriter(doc, L"[[],[]]", L"Str 3");
    CheckDocFileWriter(doc, L"[\n\t[],\n\t[]\n]", L"Doc 3");
    //
    doc.clear();
    a = doc.create_array();
    doc.root(a);
    a->append(doc.create_literal(L"null"));
    a->append(doc.create_string(L"Hello world"));
    a->append(doc.create_number(12345));
    CheckDocStringWriter(doc, L"[null,\"Hello world\",12345]", L"Str 4");
    CheckDocFileWriter(doc, L"[\n\tnull,\n\t\"Hello world\",\n\t12345\n]", L"Doc 4");
}

TEST_F(JsonToolsTest, TestDomDocumentWriter_Object)
{
    json::dom_document doc;
    doc.clear();
    doc.root(doc.create_object());
    CheckDocStringWriter(doc, L"{}", L"Str 1");
    CheckDocFileWriter(doc, L"{}", L"Doc 1");
    //
    doc.clear();
    json::dom_object* o = doc.create_object();
    doc.root(o);
    o->append_member(L"obj 1", doc.create_object());
    CheckDocStringWriter(doc, L"{\"obj 1\":{}}", L"Str 2");
    CheckDocFileWriter(doc, L"{\n\t\"obj 1\": {}\n}", L"Doc 2");
    o->append_member(L"obj 2", doc.create_object());
    CheckDocStringWriter(doc, L"{\"obj 1\":{},\"obj 2\":{}}", L"Str 3");
    CheckDocFileWriter(doc, L"{\n\t\"obj 1\": {},\n\t\"obj 2\": {}\n}", L"Doc 3");
    //
    doc.clear();
    o = doc.create_object();
    doc.root(o);
    o->append_member(L"Literal 1", doc.create_literal(L"null"));
    o->append_member(L"String 2", doc.create_string(L"Hello world"));
    o->append_member(L"Number 3", doc.create_number(12345));
    CheckDocStringWriter(doc, L"{\"Literal 1\":null,\"String 2\":\"Hello world\",\"Number 3\":12345}", L"Str 4");
    CheckDocFileWriter(doc, L"{\n\t\"Literal 1\": null,\n\t\"String 2\": \"Hello world\",\n\t\"Number 3\": 12345\n}", L"Doc 4");
    json::dom_object* o2 = doc.create_object();
    o->append_member(L"Object 4", o2);
    CheckDocStringWriter(doc, L"{\"Literal 1\":null,\"String 2\":\"Hello world\",\"Number 3\":12345,\"Object 4\":{}}", L"Str 5");
    CheckDocFileWriter(doc, L"{\n\t\"Literal 1\": null,\n\t\"String 2\": \"Hello world\",\n\t\"Number 3\": 12345,\n\t\"Object 4\": {}\n}", L"Doc 5");
    o2->append_member(L"Number 4.1", doc.create_number(123.456));
    CheckDocStringWriter(doc, L"{\"Literal 1\":null,\"String 2\":\"Hello world\",\"Number 3\":12345,\"Object 4\":{\"Number 4.1\":123.456}}", L"Str 6");
    CheckDocFileWriter(doc,
                       L"{\n\t\"Literal 1\": null,\n\t\"String 2\": \"Hello world\",\n\t\"Number 3\": 12345,\n\t\"Object 4\": {\n\t\t\"Number 4.1\": 123.456\n\t}\n}",
                       L"Doc 6");
}

TEST_F(JsonToolsTest, TestDomDocumentWriter_Doc)
{
    json::dom_document doc;
    json::dom_array* a1 = doc.create_array();
    doc.root(a1);
    a1->append(doc.create_string(L"Hello"));
    a1->append(doc.create_literal(L"null"));
    json::dom_object* o1 = doc.create_object();
    a1->append(o1);
    o1->append_member(L"Str 1", doc.create_string(L"World"));
    o1->append_member(L"Num 1", doc.create_number(123));
    json::dom_array* a2 = doc.create_array();
    o1->append_member(L"Arr 1", a2);
    o1->append_member(L"Literal 1", doc.create_literal(L"false"));
    json::dom_array* a3 = doc.create_array();
    o1->append_member(L"Arr 2", a3);
    a3->append(doc.create_number(456.78));
    CheckDocStringWriter(doc,
                         L"[\"Hello\",null,{\"Str 1\":\"World\",\"Num 1\":123,\"Arr 1\":[],\"Literal 1\":false,\"Arr 2\":[456.78]}]",
                         L"Str 1");
    CheckDocFileWriter(doc, L"[\n\
\t\"Hello\",\n\
\tnull,\n\
\t{\n\
\t\t\"Str 1\": \"World\",\n\
\t\t\"Num 1\": 123,\n\
\t\t\"Arr 1\": [],\n\
\t\t\"Literal 1\": false,\n\
\t\t\"Arr 2\": [\n\
\t\t\t456.78\n\
\t\t]\n\
\t}\n\
]", L"Doc 1");
}

TEST_F(JsonToolsTest, TestDomDocumentGeneratorConfig)
{
    json::dom_document doc;
    json::dom_document_generator gen(doc);
    json::dom_document_generator::config& conf = gen.conf();
    using config_t = json::dom_document_generator::config;
    //
    ASSERT_EQ(1u, conf.depth()) << "depth 1";
    ASSERT_EQ(config_t::default_avg_children(), conf.avg_children()) << "avg_array_items 1";
    ASSERT_EQ(config_t::default_avg_string_length(), conf.avg_string_length()) << "avg_string_length 1";
    //
    conf.avg_children(0);
    ASSERT_EQ(config_t::default_avg_children(), conf.avg_children()) << "avg_array_items 2";
    conf.avg_string_length(0);
    ASSERT_EQ(config_t::default_avg_string_length(), conf.avg_string_length()) << "avg_string_length 2";
    //
    conf.depth(0);
    ASSERT_EQ(1u, conf.depth()) << "depth 2";
    conf.depth(7);
    ASSERT_EQ(7u, conf.depth()) << "depth 3";
}

TEST_F(JsonToolsTest, TestDomDocumentGenerator)
{
    json::dom_document doc;
    json::dom_document_generator gen(doc);
    gen.conf().depth(5);
    gen.conf().avg_children(3);
    gen.run();
    SaveDoc(doc, default_test_file_name);
    unsigned int item_count = 0;
    size_t level_count = 0, max_children = gen.conf().avg_children() * 2;
    for (json::dom_document::const_iterator it = doc.begin(); it != doc.end(); it++)
    {
        if (it.level() > level_count)
            level_count = it.level();
        if (it->type() == json::dom_value_type::vt_string)
            EXPECT_TRUE(gen.conf().value_char_range().contains_all(it->text()));
        if (it->member() != nullptr)
            EXPECT_TRUE(gen.conf().name_char_range().contains_all(it->member()->name()));
        if (it->is_container())
            EXPECT_LE(it->as_container()->count(), max_children);
        EXPECT_EQ(it->document(), &doc);
        item_count++;
    }
    EXPECT_GE(level_count, 1u) << "Min depth";
    EXPECT_LE(level_count, gen.conf().depth()) << "Max depth";
    EXPECT_GE(item_count, gen.conf().depth()) << "Count min";
    EXPECT_LE(item_count, std::pow(gen.conf().avg_children(), gen.conf().depth())) << "Count max";
}

}
