#include <gtest/gtest.h>
#include "strutils.h"
#include "platforms.h"

using namespace std;

namespace stdext {
namespace strutils_test {

TEST(StrUtilsTest, TestFormat)
{
    const char s[] = "The value is";
    const int i = 123456789;
    const double f = 12345.6789;
    EXPECT_STREQ(s, str::format("%s", s).c_str());
    EXPECT_EQ("The value is 123456789 12345.6789", str::format("%s %d %5.4f", s, i, f));
    const wchar_t ws[] = L"The value is";
    EXPECT_STREQ(s, str::format("%S", ws).c_str());
    EXPECT_STREQ(ws, str::wformat(L"%ls", ws).c_str());
    EXPECT_STREQ(ws, str::wformat(L"%hs", s).c_str());
#if defined(__STDEXT_WINDOWS)
    // Safe versions of formatting functions (xxxprintf_s) are used on Windows
    EXPECT_STREQ(ws, strutils::wformat(L"%s", ws).c_str());
    EXPECT_STREQ(ws, strutils::wformat(L"%S", s).c_str());
#endif
    EXPECT_EQ(L"The value is 123456789 12345.6789", str::wformat(L"%ls %d %5.4f", ws, i, f));
}

TEST(StrUtilsTest, TestConvertion)
{
    wstring ws = L"123 %d %ls %i";
    EXPECT_EQ(str::to_string(ws), "123 %d %ls %i");
    ws = L"Aa\u263AE";
    EXPECT_EQ(str::to_string(ws), "Aa:E");
    string s1   =  "ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 abcdefghijklmnopqrstuvwxyz &#%*@^=+-/";
    wstring ws1 = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890 abcdefghijklmnopqrstuvwxyz &#%*@^=+-/";
    EXPECT_EQ(s1, str::to_string(ws1));
    EXPECT_EQ(ws1, str::to_wstring(s1));
    EXPECT_EQ(L"\xF1\xF2\xF0\xEE\xEA\xE0", str::to_wstring("\xF1\xF2\xF0\xEE\xEA\xE0"));
}

TEST(StrUtilsTest, TestReplace)
{
    EXPECT_EQ("", str::replace("", "", "", 0, false)) << L"Replace 0";
    string s = "123-456-789-456-789";
    EXPECT_EQ("123--789-456-789", str::replace(s, "456", "", 0, false)) << L"Replace 1";
    EXPECT_EQ("123--789-456-789", str::replace(s, "456", "")) << L"Replace 2";
    EXPECT_EQ("123--789-456-789", str::replace(s, "456", "", 0, false)) << L"Replace 3";
    EXPECT_EQ("123-456-789--789", str::replace(s, "456", "", 7, false)) << L"Replace 4";
    s = "\"123\"";
    EXPECT_EQ("123\"", str::replace(s, "\"", "")) << L"Replace 5.1";
    EXPECT_EQ("\"123", str::replace(s, "\"", "", s.length() - 1)) << L"Replace 5.2";
    wstring ws = L"123-456-789-456-789";
    EXPECT_EQ(L"123--789-456-789", str::replace(ws, L"456", L"", 0, false)) << L"WReplace 1";
    EXPECT_EQ(L"123--789-456-789", str::replace(ws, L"456", L"")) << L"WReplace 2";
    EXPECT_EQ(L"123--789-456-789", str::replace(ws, L"456", L"", 0, false)) << L"WReplace 3";
    EXPECT_EQ(L"123-456-789--789", str::replace(ws, L"456", L"", 7, false)) << L"WReplace 4";
}

TEST(StrUtilsTest, TestReplaceAll)
{
    string s = "123-456-789-456-789";
    EXPECT_EQ("123--789--789", str::replace_all(s, "456", "")) << L"Replace all 1";
    EXPECT_EQ("123-xx-789-xx-789", str::replace_all(s, "456", "xx")) << L"Replace all 2";
    EXPECT_EQ("abbc", str::replace_all("abbbc", "bb", "b")) << L"Replace all 3.1";
    EXPECT_EQ("\"\"", str::replace_all("\"\"\"", "\"\"", "\"")) << L"Replace all 3.2";
    wstring ws = L"123-456-789-456-789";
    EXPECT_EQ(L"123--789--789", str::replace_all(ws, L"456", L"")) << L"WReplace all 1";
    EXPECT_EQ(L"123-xx-789-xx-789", str::replace_all(ws, L"456", L"xx")) << L"WReplace all 2";
}

TEST(StrUtilsTest, TestTrim)
{
    EXPECT_EQ("123 456\t ", str::ltrim("\t 123 456\t ")) << L"LTrim 1";
    EXPECT_EQ(L"123 456\t ", str::ltrim(L"\t 123 456\t ")) << L"WLTrim 1";
    EXPECT_EQ("123 456abc", str::ltrim_chars("abc123 456abc", "abc")) << L"LTrimChars 1";
    EXPECT_EQ(L"123 456abc", str::ltrim_chars(L"abc123 456abc", L"abc")) << L"WLTrimChars 1";

    EXPECT_EQ("\t 123 456", str::rtrim("\t 123 456\t ")) << L"RTrim 1";
    EXPECT_EQ(L"\t 123 456", str::rtrim(L"\t 123 456\t ")) << L"WRTrim 1";
    EXPECT_EQ("abc123 456", str::rtrim_chars("abc123 456abc", "abc")) << L"RTrimChars 1";
    EXPECT_EQ(L"abc123 456", str::rtrim_chars(L"abc123 456abc", L"abc")) << L"WRTrimChars 1";

    EXPECT_EQ("123 456", str::trim("\t 123 456\t ")) << L"Trim 1";
    EXPECT_EQ(L"123 456", str::trim(L"\t 123 456\t ")) << L"WTrim 1";
    EXPECT_EQ("123 456", str::trim_chars("abc123 456abc", "abc")) << L"TrimChars 1";
    EXPECT_EQ(L"123 456", str::trim_chars(L"abc123 456abc", L"abc")) << L"WTrimChars 1";
}

TEST(StrUtilsTest, TestQuoted)
{
    EXPECT_EQ("''", str::quoted("")) << L"Quoted 1.1";
    EXPECT_EQ("'123'", str::quoted("123")) << L"Quoted 1.2";
    EXPECT_EQ(L"'123'", str::quoted(L"123")) << L"Quoted 1.3";
    EXPECT_EQ("\"\"", str::double_quoted("")) << L"DQuoted 2.1";
    EXPECT_EQ("\"123\"", str::double_quoted("123")) << L"DQuoted 2.2";
    EXPECT_EQ(L"\"123\"", str::double_quoted(L"123")) << L"DQuoted 2.3";
}

}
}
