#include "CppUnitTest.h"
#include "json.h"
#include "jsontools.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <limits>
#include "locutils.h"
#include "strutils.h"
#include "memchecker.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;
using namespace locutils;

TEST_CLASS(JsonToolsTest)
{
    const wstring default_test_file_name = L"current.json";

    void SaveDoc(json::dom_document& doc, const wstring file_name)
    {
        json::dom_document_writer w(doc);
        w.conf().pretty_print(true);
        w.write_to_file(file_name, ioutils::text_io_options_utf8());
    }

    void CheckDocFileWriter(json::dom_document& doc, wstring expected, wstring title)
    {
        const wstring file_name = default_test_file_name;
        SaveDoc(doc, file_name);
        ioutils::text_reader r(file_name, ioutils::text_io_options_utf8());
        wstring s;
        r.read_all(s);
        Assert::AreEqual<wstring>(expected, s, (title + L": content").c_str());
    }

    void CheckDocStringWriter(json::dom_document& doc, wstring expected, wstring title)
    {
        json::dom_document_writer w(doc);
        w.conf().pretty_print(false);
        wstring s;
        w.write(s);
        Assert::AreEqual<wstring>(expected, s, (title + L": output").c_str());
    }

    TEST_METHOD(TestDomDocumentWriter_Literal)
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

    TEST_METHOD(TestDomDocumentWriter_Number)
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

    TEST_METHOD(TestStringEscape)
    {
        json::dom_document doc;
        json::dom_document_writer w(doc);
        Assert::AreEqual<wstring>(L"", w.escape(L""), L"1");
        Assert::AreEqual<wstring>(L"\\\"", w.escape(L"\""), L"2.1");
        Assert::AreEqual<wstring>(L"\\\\", w.escape(L"\\"), L"2.2");
        Assert::AreEqual<wstring>(L"/", w.escape(L"/"), L"2.3");
        Assert::AreEqual<wstring>(L"\\b", w.escape(L"\b"), L"2.4");
        Assert::AreEqual<wstring>(L"\\f", w.escape(L"\f"), L"2.5");
        Assert::AreEqual<wstring>(L"\\n", w.escape(L"\n"), L"2.6");
        Assert::AreEqual<wstring>(L"\\r", w.escape(L"\r"), L"2.7");
        Assert::AreEqual<wstring>(L"\\t", w.escape(L"\t"), L"2.8");
        //
        Assert::AreEqual<wstring>(L"\\u0001", w.escape(L"\x0001"), L"3.1");
        Assert::AreEqual<wstring>(L"\\u001F", w.escape(L"\x001F"), L"3.2");
        //
        Assert::AreEqual<wstring>(L"\\uD834\\uDD1E", w.escape(L"\xD834\xDD1E"), L"Surrogate pair 'G clef' 1");
        Assert::AreEqual<wstring>(L"==\\uD834\\uDD1E==", w.escape(L"==\xD834\xDD1E=="), L"Surrogate pair 'G clef' 2");
        wstring expected;
        expected += utf16::replacement_character;
        expected += L"-\xDD1E";
        Assert::AreEqual<wstring>(expected, w.escape(L"\xD834-\xDD1E"), L"Surrogate pair 2");
    }

    TEST_METHOD(TestDomDocumentWriter_String)
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
        wstring expected = strutils::format(L"\"\\u0001 \\u0002 \\u%0.4X \\u%0.4X\"",
            utf16::replacement_character, utf16::replacement_character);
        CheckDocStringWriter(doc, expected, L"Str 4");
        CheckDocFileWriter(doc, expected, L"Doc 4");
        //
        wstring s;
        expected.clear();
        for (int i = 0x1; i <= 0xFFFF; i++)
        {
            wchar_t c = static_cast<wchar_t>(i);
            if (utf16::is_noncharacter(c) || utf16::is_high_surrogate(c))
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
    }

    TEST_METHOD(TestDomDocumentWriter_Array)
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

    TEST_METHOD(TestDomDocumentWriter_Object)
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

    TEST_METHOD(TestDomDocumentWriter_Doc)
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
        CheckDocFileWriter(doc,
            L"[\n\
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

    TEST_METHOD(TestDomDocumentGeneratorConfig)
    {
        json::dom_document doc;
        json::dom_document_generator gen(doc);
        json::dom_document_generator::config& conf = gen.conf();
        using config_t = json::dom_document_generator::config;
        //
        Assert::AreEqual(1, conf.min_depth(), L"min_depth 1");
        Assert::AreEqual(1, conf.max_depth(), L"max_depth 1");
        Assert::AreEqual(1, conf.min_values_by_level(), L"min_values_by_level 1");
        Assert::AreEqual(1, conf.max_values_by_level(), L"max_values_by_level 1");
        Assert::AreEqual(config_t::default_avg_array_items, conf.avg_array_items(), L"avg_array_items 1");
        Assert::AreEqual(config_t::default_avg_object_members, conf.avg_object_members(), L"avg_object_members 1");
        Assert::AreEqual(config_t::default_avg_string_length, conf.avg_string_length(), L"avg_string_length 1");
        //
        conf.max_depth(conf.min_depth() - 1);
        Assert::AreEqual(conf.min_depth(), conf.max_depth(), L"max_depth 2");
        conf.max_values_by_level(conf.min_values_by_level() - 1);
        Assert::AreEqual(conf.min_values_by_level(), conf.max_values_by_level(), L"max_values_by_level 2");
        conf.avg_array_items(0);
        Assert::AreEqual(config_t::default_avg_array_items, conf.avg_array_items(), L"avg_array_items 2");
        conf.avg_object_members(-1);
        Assert::AreEqual(config_t::default_avg_object_members, conf.avg_object_members(), L"avg_object_members 2");
        conf.avg_string_length(-2);
        Assert::AreEqual(config_t::default_avg_string_length, conf.avg_string_length(), L"avg_string_length 2");
        //
        conf.min_depth(-1);
        Assert::AreEqual(1, conf.min_depth(), L"min_depth 2");
        conf.min_values_by_level(-1);
        Assert::AreEqual(1, conf.min_values_by_level(), L"min_values_by_level 2");
        //
        conf.min_depth(2);
        Assert::AreEqual(2, conf.min_depth(), L"min_depth 3");
        Assert::AreEqual(2, conf.max_depth(), L"max_depth 3");
        conf.min_values_by_level(4);
        Assert::AreEqual(4, conf.min_values_by_level(), L"min_values_by_level 3");
        Assert::AreEqual(4, conf.max_values_by_level(), L"max_values_by_level 3");
        //
        conf.max_depth(7);
        Assert::AreEqual(2, conf.min_depth(), L"min_depth 4");
        Assert::AreEqual(7, conf.max_depth(), L"max_depth 4");
        conf.max_values_by_level(9);
        Assert::AreEqual(4, conf.min_values_by_level(), L"min_values_by_level 4");
        Assert::AreEqual(9, conf.max_values_by_level(), L"max_values_by_level 4");
    }

    TEST_METHOD(TestDomDocumentGenerator)
    {
        json::dom_document doc;
        json::dom_document_generator gen(doc);
        gen.conf().min_depth(5);
        gen.conf().max_values_by_level(2);
        gen.run();
        SaveDoc(doc, default_test_file_name);
        int count = 0;
        for (const auto value : doc)
        {
            count++;
        }
        Assert::IsTrue(count >= gen.conf().min_depth() * gen.conf().min_values_by_level(), L"Count 1");
        Assert::IsTrue(count <= gen.conf().max_depth() * gen.conf().max_values_by_level(), L"Count 2");
    }

};
