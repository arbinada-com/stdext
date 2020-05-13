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
        Assert::IsTrue(result && !parser.has_errors(), (title2 + L"OK with errors:" + err_text).c_str());
        Assert::IsFalse(parser.has_errors(), (title2 + L"errors:" + err_text).c_str());
        bool doc_are_diff = json::equal(expected, doc);
        if (doc_are_diff)
        {
            json::dom_document_writer w(doc);
            w.write_to_file(L"current_json.txt");
            Assert::Fail((title2 + L"docs are different").c_str());
        }
    }

    TEST_METHOD(TestSimpleDoc)
    {
        json::dom_document doc;
        CheckParseText(L"", doc, L"Empty text 1");
        CheckParseText(L"\t\r\n\t", doc, L"Empty text 2");
        doc.clear();
        doc.root(doc.create_number(123));
        CheckParseText(L"123", doc, L"Num 1.1");
        CheckParseText(L"123\t\r\n", doc, L"Num 1.2");
        doc.clear();
        doc.root(doc.create_number(123.456));
        CheckParseText(L"123.456", doc, L"Num 2");
        doc.clear();
        doc.root(doc.create_number(-1.23456e8));
        CheckParseText(L"-1.23456e8", doc, L"Num 3");
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

    TEST_METHOD(TestSimpleDocErrors)
    {
        CheckError(L"null\nnull", json::parser_msg_kind::err_unexpected_lexeme_fmt, textpos(2,1), L"Unexpected token 1");
        CheckError(L"123\n456", json::parser_msg_kind::err_unexpected_lexeme_fmt, textpos(2, 1), L"Unexpected token 2");
        CheckError(L"\"Hello\" \"world\"", json::parser_msg_kind::err_unexpected_lexeme_fmt, textpos(2, 1), L"Unexpected token 3");
    }

};
