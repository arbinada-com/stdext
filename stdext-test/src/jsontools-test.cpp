#include <gtest/gtest.h>
#include "jsontools.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include "locutils.h"
#include "strutils.h"
#include "testutils.h"
#include "jsondom-test.h"

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

    void SaveDoc(json::dom_document& doc, const wstring file_name, const ioutils::text_io_options& options)
    {
        json::dom_document_writer w(doc);
        w.conf().pretty_print(true);
        w.write_to_file(file_name, options);
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
};

/*
 * JSON document reader tests
 */
class JsonDocumentReaderTest : public JsonToolsTest
{
};

TEST_F(JsonDocumentReaderTest, TestGeneratedDoc)
{
    const int max_test_count = 100;
    for (int i = 1; i <= max_test_count; i++)
    {
        json::dom_document doc1;
        json::dom_document_generator gen(doc1);
        gen.conf().depth(7);
        gen.conf().avg_children(5);
        gen.run();
        ioutils::text_io_options_utf8 options = ioutils::text_io_options_utf8();
        SaveDoc(doc1, default_test_file_name, options);
        json::dom_document doc2;
        json::dom_document_reader reader(doc2);
        bool result = reader.read_from_file(default_test_file_name, options);
        EXPECT_TRUE(result);
        EXPECT_FALSE(reader.messages().has_errors());
        if (reader.messages().has_errors())
        {
            wstring msg;
            for (auto error : reader.messages().errors())
                msg += error->to_wstring() + L"\n";
            FAIL() << msg;
        }
        if (!result)
            break;
    }
}

/*
 * JSON document writer tests
 */
class JsonDocumentWriterTest : public JsonToolsTest
{
protected:
    void CheckDocFileWriter(json::dom_document& doc, const wstring& expected, const wstring& title)
    {
        const wstring file_name = default_test_file_name;
        SaveDoc(doc, file_name, ioutils::text_io_options_utf8());
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

TEST_F(JsonDocumentWriterTest, TestLiteral)
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

TEST_F(JsonDocumentWriterTest, TestNumber)
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

TEST_F(JsonDocumentWriterTest, TestString)
{
    json::dom_document doc;
    doc.clear();
    doc.root(doc.create_string(L"Hello world!"));
    CheckDocStringWriter(doc, L"\"Hello world!\"", L"Str 1");
    CheckDocFileWriter(doc, L"\"Hello world!\"", L"Doc 1");
    doc.clear();
    doc.root(doc.create_string(L"Quick\r\nbrown\tfox\t\xD834\xDD1E\n"));
    CheckDocStringWriter(doc, L"\"Quick\\r\\nbrown\\tfox\\t\xD834\xDD1E\\n\"", L"Str 2");
    CheckDocFileWriter(doc, L"\"Quick\\r\\nbrown\\tfox\\t\xD834\xDD1E\\n\"", L"Doc 2");
    doc.clear();
    doc.root(doc.create_string(L"ABC été déjà строка"));
    CheckDocStringWriter(doc, L"\"ABC été déjà строка\"", L"Str 3");
    CheckDocFileWriter(doc, L"\"ABC été déjà строка\"", L"Doc 3");
    doc.clear();
    doc.root(doc.create_string(L"\x1 \x2 \xFFFE \xFFFF"));
    wstring expected = L"\"\\u0001 \\u0002 \\uFFFE \\uFFFF\"";
    CheckDocStringWriter(doc, expected, L"Str 4");
    CheckDocFileWriter(doc, expected, L"Doc 4");
    // Non-existing escape sequences
    doc.clear();
    doc.root(doc.create_string(L"\\a\n\\x\\y"));
    expected = L"\"\\\\a\\n\\\\x\\\\y\"";
    CheckDocStringWriter(doc, expected, L"Str 5");
    CheckDocFileWriter(doc, expected, L"Doc 5");
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
    expected = {L'"', utf16::high_surrogate_min, L'"'};
    doc.clear();
    doc.root(doc.create_string(s));
    CheckDocStringWriter(doc, expected, L"Str 6.1");
    CheckDocFileWriter(doc, expected, L"Doc 6.1");
    s = {L'\xFDD0', L'A'};
    expected = L"\"\\uFDD0A\"";
    doc.clear();
    doc.root(doc.create_string(s));
    CheckDocStringWriter(doc, expected, L"Str 6.2");
    CheckDocFileWriter(doc, expected, L"Doc 6.2");
}

TEST_F(JsonDocumentWriterTest, TestArray)
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
    a->append(doc.create_number(12345));
    CheckDocStringWriter(doc, L"[12345]", L"Str 2");
    CheckDocFileWriter(doc, L"[\n\t12345\n]", L"Doc 2");
    //
    doc.clear();
    a = doc.create_array();
    doc.root(a);
    a->append(doc.create_array());
    CheckDocStringWriter(doc, L"[[]]", L"Str 3.1");
    CheckDocFileWriter(doc, L"[\n\t[]\n]", L"Doc 3.1");
    a->append(doc.create_array());
    CheckDocStringWriter(doc, L"[[],[]]", L"Str 3.2");
    CheckDocFileWriter(doc, L"[\n\t[],\n\t[]\n]", L"Doc 3.2");
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

TEST_F(JsonDocumentWriterTest, TestObject)
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
    o->append_member(L"Single member", doc.create_number(12345));
    CheckDocStringWriter(doc, L"{\"Single member\":12345}", L"Str 2.1");
    CheckDocFileWriter(doc, L"{\n\t\"Single member\": 12345\n}", L"Doc 2.1");
    o->clear();
    o->append_member(L"Single member", doc.create_literal(L"null"));
    CheckDocStringWriter(doc, L"{\"Single member\":null}", L"Str 2.2");
    CheckDocFileWriter(doc, L"{\n\t\"Single member\": null\n}", L"Doc 2.2");
    //
    doc.clear();
    o = doc.create_object();
    doc.root(o);
    o->append_member(L"obj 1", doc.create_object());
    CheckDocStringWriter(doc, L"{\"obj 1\":{}}", L"Str 3.1");
    CheckDocFileWriter(doc, L"{\n\t\"obj 1\": {}\n}", L"Doc 3.1");
    o->append_member(L"obj 2", doc.create_object());
    CheckDocStringWriter(doc, L"{\"obj 1\":{},\"obj 2\":{}}", L"Str 3.2");
    CheckDocFileWriter(doc, L"{\n\t\"obj 1\": {},\n\t\"obj 2\": {}\n}", L"Doc 3.2");
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

TEST_F(JsonDocumentWriterTest, TestObjectMemberNames)
{
    json::dom_document doc;
    json::dom_object* o = doc.create_object();
    doc.root(o);
    o->append_member(L"\x1 \xFDD0 \\x \n \\y", doc.create_literal(L"null"));
    CheckDocStringWriter(doc, L"{\"\\u0001 \\uFDD0 \\\\x \\n \\\\y\":null}", L"Str 1");
    CheckDocFileWriter(doc, L"{\n\t\"\\u0001 \\uFDD0 \\\\x \\n \\\\y\": null\n}", L"Doc 1");
}

TEST_F(JsonDocumentWriterTest, TestDoc)
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

/*
 * DOM document generator tests
 */

class DomDocumentGeneratorTest : public JsonToolsTest
{

};

TEST_F(DomDocumentGeneratorTest, TestConfig)
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

TEST_F(DomDocumentGeneratorTest, TestGenerationLimits)
{
    json::dom_document doc;
    json::dom_document_generator gen(doc);
    gen.conf().depth(5);
    gen.conf().avg_children(3);
    gen.run();
    SaveDoc(doc, default_test_file_name, ioutils::text_io_options_utf8());
    unsigned int item_count = 0;
    size_t level_count = 0, max_children = gen.conf().avg_children() * 2;
    for (json::dom_document::const_iterator it = doc.begin(); it != doc.end(); it++)
    {
        if (it.level() > level_count)
            level_count = it.level();
        if (it->type() == json::dom_value_type::vt_string)
        {
            EXPECT_TRUE(gen.conf().value_char_range().contains_all(it->text()));
        }
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

/*
 * Diff tests
 */
class JsonDomDocumentDiffTest : public testing::Test
{
protected:
    void CheckNoDiff(const json::dom_document& ldoc,
                     const json::dom_document& rdoc,
                     const json::dom_document_diff_options& options,
                     const string title)
    {
        json::dom_document_diff diff = json::make_diff(ldoc, rdoc, options);
        EXPECT_FALSE(diff.has_differences()) << title;
        if (diff.has_differences())
        {
            string msg = title + ": ";
            for (const json::dom_document_diff_item& item : diff.items())
            {
                msg += item.to_string() + "\n";
            }
            FAIL() << msg;
        }
    }

    void CheckHasDiffOfKind(const json::dom_document_diff_kind expected_kind,
                            const json::dom_document& ldoc,
                            const json::dom_document& rdoc,
                            const json::dom_value* lv,
                            const json::dom_value* rv,
                            const json::dom_document_diff_options& options,
                            const string title)
    {
        json::dom_document_diff diff = json::make_diff(ldoc, rdoc, options);
        EXPECT_TRUE(diff.has_differences());
        bool has_tested_diff = false;
        string msg = "Diff kinds: ";
        for (const json::dom_document_diff_item& item : diff.items())
        {
            if (item.kind() == expected_kind)
            {
                EXPECT_EQ(item.lval(), lv) << title;
                EXPECT_EQ(item.rval(), rv) << title;
                has_tested_diff = true;
                break;
            }
            else
                msg += json::to_string(item.kind()) + ",";
        }
        EXPECT_TRUE(has_tested_diff) << title + ". " + msg;
    }
};


TEST_F(JsonDomDocumentDiffTest, TestFullEquals)
{
    json::dom_document doc1, doc2;
    jsondom_test::JsonDomTest::FillTestDoc(doc1);
    jsondom_test::JsonDomTest::FillTestDoc(doc2);
    json::dom_document_diff_options options;
    options.case_sensitive(true);
    options.compare_all(false);
    CheckNoDiff(doc1, doc2, options, "TestDoc");
}

TEST_F(JsonDomDocumentDiffTest, TestType)
{
    json::dom_document ldoc, rdoc;
    json::dom_value* lv = ldoc.create_literal(L"null");
    ldoc.root(lv);
    json::dom_value* rv = rdoc.create_string(L"null");
    rdoc.root(rv);
    CheckHasDiffOfKind(json::dom_document_diff_kind::type_diff, ldoc, rdoc, lv, rv, json::dom_document_diff_options(), "Type");
}

TEST_F(JsonDomDocumentDiffTest, TestPath)
{
    json::dom_document ldoc, rdoc;
    // ldoc is [[],[]]
    json::dom_array* arr = ldoc.create_array();
    ldoc.root(arr);
    arr->append(ldoc.create_array());
    json::dom_value* lv = ldoc.create_array();
    arr->append(lv);
    // rdoc is [[[]]]
    arr = rdoc.create_array();
    rdoc.root(arr);
    json::dom_array* arr2 = rdoc.create_array();
    arr->append(arr2);
    json::dom_value* rv = rdoc.create_array();
    arr2->append(rv);
    arr->append(rdoc.create_array());
    json::dom_document_diff_options options;
    options.compare_all(true);
    CheckHasDiffOfKind(json::dom_document_diff_kind::path_diff, ldoc, rdoc, lv, rv, options, "Path");
}

TEST_F(JsonDomDocumentDiffTest, TestCount)
{
    json::dom_document ldoc, rdoc;
    // ldoc is [12,34]
    json::dom_array* lv = ldoc.create_array();
    ldoc.root(lv);
    lv->append(ldoc.create_number(12));
    lv->append(ldoc.create_number(34));
    // rdoc is [1,2,3]
    json::dom_array* rv = rdoc.create_array();
    rdoc.root(rv);
    rv->append(rdoc.create_number(1));
    rv->append(rdoc.create_number(2));
    rv->append(rdoc.create_number(3));
    CheckHasDiffOfKind(json::dom_document_diff_kind::count_diff, ldoc, rdoc, lv, rv, json::dom_document_diff_options(), "Count");
}

TEST_F(JsonDomDocumentDiffTest, TestMemberName)
{
    json::dom_document ldoc, rdoc;
    // ldoc
    json::dom_object* lobj = ldoc.create_object();
    ldoc.root(lobj);
    json::dom_value* lv = ldoc.create_number(123);
    lobj->append_member(L"VALUE", lv);
    json::dom_object* robj = rdoc.create_object();
    // rdoc
    rdoc.root(robj);
    json::dom_value* rv = rdoc.create_number(123);
    robj->append_member(L"value", rv);
    json::dom_document_diff_options options;
    options.case_sensitive(true);
    CheckHasDiffOfKind(json::dom_document_diff_kind::member_name_diff, ldoc, rdoc, lv, rv, options, "Member 1");
    options.case_sensitive(false);
    CheckNoDiff(ldoc, rdoc, options, "Member 2");
}

TEST_F(JsonDomDocumentDiffTest, TestValue)
{
    json::dom_document ldoc, rdoc;
    json::dom_value* lv = ldoc.create_string(L"Value 1");
    ldoc.root(lv);
    json::dom_value* rv = rdoc.create_string(L"value 1");
    rdoc.root(rv);
    json::dom_document_diff_options options;
    options.case_sensitive(true);
    CheckHasDiffOfKind(json::dom_document_diff_kind::value_diff, ldoc, rdoc, lv, rv, json::dom_document_diff_options(), "Value 1");
    options.case_sensitive(false);
    CheckNoDiff(ldoc, rdoc, options, "Value 2");
}

}
