#include "CppUnitTest.h"
#include "json.h"
#include <fstream>
#include <limits>
#include "strutils.h"
#include "memchecker.h"

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
        Assert::IsTrue(json::equal(expected, doc), (title2 + L"doc difference").c_str());
    }

    TEST_METHOD(TestSimpleDoc)
    {
        const wstring input;
        json::dom_document doc;
        CheckParseText(L"", doc, L"Empty text");
        doc.clear();
        doc.root(doc.create_number(123));
        CheckParseText(L"123", doc, L"Num 1");
    }

};
