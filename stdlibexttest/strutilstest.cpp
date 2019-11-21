#include "stdafx.h"
#include "CppUnitTest.h"
#include "strutils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;

namespace stdexttest
{
    TEST_CLASS(StrUtilsTest)
    {
    public:

        TEST_METHOD(TestFormat)
        {
            const char s[] = "The value is";
            const int i = 123456789;
            const double f = 12345.6789;
            Assert::AreEqual("The value is 123456789 12345.6789", strutils::format("%s %d %5.4f", s, i, f).c_str());

            const wchar_t ws[] = L"The value is";
            Assert::AreEqual(L"The value is 123456789 12345.6789", strutils::format(L"%s %d %5.4f", ws, i, f).c_str());
        }

        TEST_METHOD(TestConvertion)
        {
            wstring ws = L"123 %d %s %i";
            Assert::AreEqual("123 %d %s %i", strutils::to_string(ws).c_str());
            ws = L"Aa""\u263A""E";
            Assert::AreEqual("Aa?E", strutils::to_string(ws).c_str());
        }

        TEST_METHOD(TestReplace)
        {
            wstring ws = L"123-456-789-456-789";
            Assert::AreEqual(L"123--789--789", strutils::replace_all(ws, L"456", L"").c_str(), L"Replace all 1");
            Assert::AreEqual(L"123-xx-789-xx-789", strutils::replace_all(ws, L"456", L"xx").c_str(), L"Replace all 2");
        }

    };
}