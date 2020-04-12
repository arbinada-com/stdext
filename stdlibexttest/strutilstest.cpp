#include "stdafx.h"
#include "CppUnitTest.h"
#include "strutils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;

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
        Assert::AreEqual("", strutils::replace("", "", "", 0, false).c_str(), L"Replace 0");
        string s = "123-456-789-456-789";
        Assert::AreEqual("123--789-456-789", strutils::replace(s, "456", "", 0, false).c_str(), L"Replace 1");
        Assert::AreEqual("123--789-456-789", strutils::replace(s, "456", "").c_str(), L"Replace 2");
        Assert::AreEqual("123--789-456-789", strutils::replace(s, "456", "", 0, false).c_str(), L"Replace 3");
        Assert::AreEqual("123-456-789--789", strutils::replace(s, "456", "", 7, false).c_str(), L"Replace 4");
        s = "\"123\"";
        Assert::AreEqual("123\"", strutils::replace(s, "\"", "").c_str(), L"Replace 5.1");
        Assert::AreEqual("\"123", strutils::replace(s, "\"", "", s.length() - 1).c_str(), L"Replace 5.2");
        wstring ws = L"123-456-789-456-789";
        Assert::AreEqual(L"123--789-456-789", strutils::replace(ws, L"456", L"", 0, false).c_str(), L"WReplace 1");
        Assert::AreEqual(L"123--789-456-789", strutils::replace(ws, L"456", L"").c_str(), L"WReplace 2");
        Assert::AreEqual(L"123--789-456-789", strutils::replace(ws, L"456", L"", 0, false).c_str(), L"WReplace 3");
        Assert::AreEqual(L"123-456-789--789", strutils::replace(ws, L"456", L"", 7, false).c_str(), L"WReplace 4");
    }

    TEST_METHOD(TestReplaceAll)
    {
        string s = "123-456-789-456-789";
        Assert::AreEqual("123--789--789", strutils::replace_all(s, "456", "").c_str(), L"Replace all 1");
        Assert::AreEqual("123-xx-789-xx-789", strutils::replace_all(s, "456", "xx").c_str(), L"Replace all 2");
        Assert::AreEqual("abbc", strutils::replace_all("abbbc", "bb", "b").c_str(), L"Replace all 3.1");
        Assert::AreEqual("\"\"", strutils::replace_all("\"\"\"", "\"\"", "\"").c_str(), L"Replace all 3.2");
        wstring ws = L"123-456-789-456-789";
        Assert::AreEqual(L"123--789--789", strutils::replace_all(ws, L"456", L"").c_str(), L"WReplace all 1");
        Assert::AreEqual(L"123-xx-789-xx-789", strutils::replace_all(ws, L"456", L"xx").c_str(), L"WReplace all 2");
    }

    TEST_METHOD(TestTrim)
    {
        Assert::AreEqual("123 456\t ", strutils::ltrim("\t 123 456\t ").c_str(), L"LTrim 1");
        Assert::AreEqual(L"123 456\t ", strutils::ltrim(L"\t 123 456\t ").c_str(), L"WLTrim 1");
        Assert::AreEqual("123 456abc", strutils::ltrim_chars("abc123 456abc", "abc").c_str(), L"LTrimChars 1");
        Assert::AreEqual(L"123 456abc", strutils::ltrim_chars(L"abc123 456abc", L"abc").c_str(), L"WLTrimChars 1");

        Assert::AreEqual("\t 123 456", strutils::rtrim("\t 123 456\t ").c_str(), L"RTrim 1");
        Assert::AreEqual(L"\t 123 456", strutils::rtrim(L"\t 123 456\t ").c_str(), L"WRTrim 1");
        Assert::AreEqual("abc123 456", strutils::rtrim_chars("abc123 456abc", "abc").c_str(), L"RTrimChars 1");
        Assert::AreEqual(L"abc123 456", strutils::rtrim_chars(L"abc123 456abc", L"abc").c_str(), L"WRTrimChars 1");

        Assert::AreEqual("123 456", strutils::trim("\t 123 456\t ").c_str(), L"Trim 1");
        Assert::AreEqual(L"123 456", strutils::trim(L"\t 123 456\t ").c_str(), L"WTrim 1");
        Assert::AreEqual("123 456", strutils::trim_chars("abc123 456abc", "abc").c_str(), L"TrimChars 1");
        Assert::AreEqual(L"123 456", strutils::trim_chars(L"abc123 456abc", L"abc").c_str(), L"WTrimChars 1");
    }


};
