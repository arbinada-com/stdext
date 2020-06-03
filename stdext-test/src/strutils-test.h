#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "strutils.h"
#include "platforms.h"

using namespace std;
using namespace stdext;

TEST(StrUtilsTest, TestFormat)
{
    const char s[] = "The value is";
    const int i = 123456789;
    const double f = 12345.6789;
    EXPECT_STREQ(s, strutils::format("%s", s).c_str());
    EXPECT_EQ("The value is 123456789 12345.6789", strutils::format("%s %d %5.4f", s, i, f));
    const wchar_t ws[] = L"The value is";
    EXPECT_STREQ(s, strutils::format("%S", ws).c_str());
    EXPECT_STREQ(ws, strutils::wformat(L"%ls", ws).c_str());
    EXPECT_STREQ(ws, strutils::wformat(L"%hs", s).c_str());
#if defined(__STDEXT_WINDOWS)
    // Safe versions of formatting functions (xxxprintf_s) are used on Windows
    EXPECT_STREQ(ws, strutils::wformat(L"%s", ws).c_str());
    EXPECT_STREQ(ws, strutils::wformat(L"%S", s).c_str());
#endif
    EXPECT_EQ(L"The value is 123456789 12345.6789", strutils::wformat(L"%ls %d %5.4f", ws, i, f));
}

TEST(StrUtilsTest, TestConvertion)
{
    wstring ws = L"123 %d %ls %i";
    ASSERT_EQ(strutils::to_string(ws), "123 %d %ls %i");
    ws = L"Aa\u263AE";
    ASSERT_EQ(strutils::to_string(ws), "Aa:E");
    string s1   =  "ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 abcdefghijklmnopqrstuvwxyz &#%*@^=+-/";
    wstring ws1 = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 abcdefghijklmnopqrstuvwxyz &#%*@^=+-/";
    ASSERT_EQ(s1, strutils::to_string(ws1));
    ASSERT_EQ(ws1, strutils::to_wstring(s1));
}

TEST(StrUtilsTest, TestReplace)
{
    ASSERT_EQ("", strutils::replace("", "", "", 0, false)) << L"Replace 0";
    string s = "123-456-789-456-789";
    ASSERT_EQ("123--789-456-789", strutils::replace(s, "456", "", 0, false)) << L"Replace 1";
    ASSERT_EQ("123--789-456-789", strutils::replace(s, "456", "")) << L"Replace 2";
    ASSERT_EQ("123--789-456-789", strutils::replace(s, "456", "", 0, false)) << L"Replace 3";
    ASSERT_EQ("123-456-789--789", strutils::replace(s, "456", "", 7, false)) << L"Replace 4";
    s = "\"123\"";
    ASSERT_EQ("123\"", strutils::replace(s, "\"", "")) << L"Replace 5.1";
    ASSERT_EQ("\"123", strutils::replace(s, "\"", "", s.length() - 1)) << L"Replace 5.2";
    wstring ws = L"123-456-789-456-789";
    ASSERT_EQ(L"123--789-456-789", strutils::replace(ws, L"456", L"", 0, false)) << L"WReplace 1";
    ASSERT_EQ(L"123--789-456-789", strutils::replace(ws, L"456", L"")) << L"WReplace 2";
    ASSERT_EQ(L"123--789-456-789", strutils::replace(ws, L"456", L"", 0, false)) << L"WReplace 3";
    ASSERT_EQ(L"123-456-789--789", strutils::replace(ws, L"456", L"", 7, false)) << L"WReplace 4";
}

TEST(StrUtilsTest, TestReplaceAll)
{
    string s = "123-456-789-456-789";
    ASSERT_EQ("123--789--789", strutils::replace_all(s, "456", "")) << L"Replace all 1";
    ASSERT_EQ("123-xx-789-xx-789", strutils::replace_all(s, "456", "xx")) << L"Replace all 2";
    ASSERT_EQ("abbc", strutils::replace_all("abbbc", "bb", "b")) << L"Replace all 3.1";
    ASSERT_EQ("\"\"", strutils::replace_all("\"\"\"", "\"\"", "\"")) << L"Replace all 3.2";
    wstring ws = L"123-456-789-456-789";
    ASSERT_EQ(L"123--789--789", strutils::replace_all(ws, L"456", L"")) << L"WReplace all 1";
    ASSERT_EQ(L"123-xx-789-xx-789", strutils::replace_all(ws, L"456", L"xx")) << L"WReplace all 2";
}

TEST(StrUtilsTest, TestTrim)
{
    ASSERT_EQ("123 456\t ", strutils::ltrim("\t 123 456\t ")) << L"LTrim 1";
    ASSERT_EQ(L"123 456\t ", strutils::ltrim(L"\t 123 456\t ")) << L"WLTrim 1";
    ASSERT_EQ("123 456abc", strutils::ltrim_chars("abc123 456abc", "abc")) << L"LTrimChars 1";
    ASSERT_EQ(L"123 456abc", strutils::ltrim_chars(L"abc123 456abc", L"abc")) << L"WLTrimChars 1";

    ASSERT_EQ("\t 123 456", strutils::rtrim("\t 123 456\t ")) << L"RTrim 1";
    ASSERT_EQ(L"\t 123 456", strutils::rtrim(L"\t 123 456\t ")) << L"WRTrim 1";
    ASSERT_EQ("abc123 456", strutils::rtrim_chars("abc123 456abc", "abc")) << L"RTrimChars 1";
    ASSERT_EQ(L"abc123 456", strutils::rtrim_chars(L"abc123 456abc", L"abc")) << L"WRTrimChars 1";

    ASSERT_EQ("123 456", strutils::trim("\t 123 456\t ")) << L"Trim 1";
    ASSERT_EQ(L"123 456", strutils::trim(L"\t 123 456\t ")) << L"WTrim 1";
    ASSERT_EQ("123 456", strutils::trim_chars("abc123 456abc", "abc")) << L"TrimChars 1";
    ASSERT_EQ(L"123 456", strutils::trim_chars(L"abc123 456abc", L"abc")) << L"WTrimChars 1";
}

TEST(StrUtilsTest, TestQuoted)
{
    ASSERT_EQ("''", strutils::quoted("")) << L"Quoted 1.1";
    ASSERT_EQ("'123'", strutils::quoted("123")) << L"Quoted 1.2";
    ASSERT_EQ(L"'123'", strutils::quoted(L"123")) << L"Quoted 1.3";
    ASSERT_EQ("\"\"", strutils::double_quoted("")) << L"DQuoted 2.1";
    ASSERT_EQ("\"123\"", strutils::double_quoted("123")) << L"DQuoted 2.2";
    ASSERT_EQ(L"\"123\"", strutils::double_quoted(L"123")) << L"DQuoted 2.3";
}
