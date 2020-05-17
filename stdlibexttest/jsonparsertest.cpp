#include "CppUnitTest.h"
#include "json.h"
#include <fstream>
#include <limits>
#include "strutils.h"
#include "memchecker.h"
#include "parserstest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;
using namespace parsers;

TEST_CLASS(JsonParserTest)
{
    void CheckParseText(wstring input, json::dom_document& expected, wstring title)
    {
        wstring title2 = title + L": ";
        wstringstream ss(input);
        ioutils::text_reader r(ss);
        json::msg_collector_t mc;
        json::dom_document doc;
        json::parser parser(r, mc, doc);
        bool result = parser.run();
        wstring err_text;
        if (parser.has_errors())
        {
            for (json::message_t* err : parser.messages().errors())
            {
                err_text += L"\n" + err->to_wstring();
            }
        }
        if (result && parser.has_errors())
            Assert::Fail((title2 + L"OK with errors:" + err_text).c_str());
        if (!result && !parser.has_errors())
            Assert::Fail((title2 + L"failed without errors:" + err_text).c_str());
        Assert::IsFalse(parser.has_errors(), (title2 + L"errors:" + err_text).c_str());
        bool doc_are_equal = json::equal(expected, doc);
        if (!doc_are_equal)
        {
            json::dom_document_writer w1(doc);
            w1.write_to_file(L"parser_current.json");
            json::dom_document_writer w2(expected);
            w2.write_to_file(L"parser_expected.json");
            Assert::Fail((title2 + L"docs are different").c_str());
        }
    }

    void CheckError(const wstring input, const json::parser_msg_kind kind, const textpos& pos, const wstring title)
    {
        wstringstream ss(input);
        ioutils::text_reader r(ss, L"ChkErrStream");
        json::msg_collector_t mc;
        json::dom_document doc;
        json::parser parser(r, mc, doc);
        wstring title2 = title + L": ";
        Assert::IsFalse(parser.run(), (title2 + L"parsed OK").c_str());
        Assert::IsTrue(parser.has_errors(), (title2 + L"no errors").c_str());
        json::message_t* err = parser.messages().errors()[0];
        Assert::IsTrue(msg_origin::parser == err->origin(), (title2 + L"origin. " + err->text()).c_str());
        Assert::AreEqual((int)kind, (int)err->kind(), (title2 + L"kind. " + err->text()).c_str());
        Assert::IsFalse(err->text().empty(), (title2 + L"text is empty").c_str());
        Assert::AreEqual<parsers::textpos>(pos, err->pos(), (title2 + L"error pos. " + err->text()).c_str());
        Assert::AreEqual(r.source_name(), err->source(), (title2 + L"source. " + err->text()).c_str());
    }

    TEST_METHOD(TestSimpleDoc)
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

    TEST_METHOD(TestSimpleDocErrors)
    {
        CheckError(L"null\nnull", json::parser_msg_kind::err_unexpected_lexeme_fmt, textpos(2, 1), L"Unexpected lexeme 1");
        CheckError(L"123\n456", json::parser_msg_kind::err_unexpected_lexeme_fmt, textpos(2, 1), L"Unexpected lexeme 2");
        CheckError(L"\"Hello\" \"world\"", json::parser_msg_kind::err_unexpected_lexeme_fmt, textpos(1, 9), L"Unexpected lexeme 3");
    }

    TEST_METHOD(TestArrays)
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

    TEST_METHOD(TestArrayErrors)
    {
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

    TEST_METHOD(TestObjects)
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

    TEST_METHOD(TestObjectErrors)
    {
        CheckError(L"{", json::parser_msg_kind::err_unclosed_object, textpos(1, 1), L"1.1");
        CheckError(L"{null", json::parser_msg_kind::err_expected_member_name, textpos(1, 2), L"2.1");
        CheckError(L"{\"Member1\"", json::parser_msg_kind::err_expected_name_separator, textpos(1, 10), L"3.1");
        CheckError(L"{\"Member1\":", json::parser_msg_kind::err_expected_value, textpos(1, 11), L"4.1");
    }
};
