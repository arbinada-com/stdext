#include <gtest/gtest.h>
#include "jsoncommon.h"
#include "locutils.h"
#include "testutils.h"

using namespace std;
using namespace stdext;
using namespace locutils;
using namespace testutils;

namespace jsoncommon_test
{

TEST(JsonCommonTest, TestIsUnescaped)
{
    for (wchar_t c = 0; c < 0x20; c++)
        EXPECT_FALSE(json::is_unescaped(c));
    EXPECT_FALSE(json::is_unescaped(L'\x22'));
    EXPECT_FALSE(json::is_unescaped(L'\\'));
    EXPECT_FALSE(json::is_unescaped(L'\x5C'));
}

TEST(JsonCommonTest, TestToEscaped)
{
    EXPECT_EQ(L"ABC", json::to_escaped(L"ABC"));
    EXPECT_EQ(L"\\u0041\\u0042\\u0043", json::to_escaped(L"ABC", true));
    EXPECT_EQ(L"\\\" \\\\ / \\b \\f \\n \\r \\t \\u0001", json::to_escaped(L"\" \\ / \b \f \n \r \t \x1"));
    //
    EXPECT_EQ(L"", json::to_escaped(L"")) << "1";
    EXPECT_EQ(L"\\\"", json::to_escaped(L"\"")) << "2.1";
    EXPECT_EQ(L"\\\\", json::to_escaped(L"\\")) << "2.2";
    EXPECT_EQ(L"/", json::to_escaped(L"/")) << "2.3";
    EXPECT_EQ(L"\\b", json::to_escaped(L"\b")) << "2.4";
    EXPECT_EQ(L"\\f", json::to_escaped(L"\f")) << "2.5";
    EXPECT_EQ(L"\\n", json::to_escaped(L"\n")) << "2.6";
    EXPECT_EQ(L"\\r", json::to_escaped(L"\r")) << "2.7";
    EXPECT_EQ(L"\\t", json::to_escaped(L"\t")) << "2.8";
    //
    EXPECT_EQ(L"\\u0001", json::to_escaped(L"\x0001")) << "3.1";
    EXPECT_EQ(L"\\u001F", json::to_escaped(L"\x001F")) << "3.2";
    wstring ws = {utf16::replacement_character};
    ASSERT_EQ(ws, json::to_escaped(ws)) << "3.3";
    EXPECT_TRUE(utf16::is_noncharacter(L'\xFDD0'));
    ASSERT_EQ(L"\\uFDD0", json::to_escaped(L"\xFDD0")) << "3.4";
    //
    EXPECT_EQ(L"\xD834\xDD1E", json::to_escaped(L"\xD834\xDD1E")) << "Surrogate pair 'G clef' 1";
    EXPECT_EQ(L"==\xD834\xDD1E==", json::to_escaped(L"==\xD834\xDD1E==")) << "Surrogate pair 'G clef' 2";
    EXPECT_EQ(L"\xD834", json::to_escaped(L"\xD834")) << "Incomplete surrogate pair";
    EXPECT_EQ(L"\xD834-\x1234", json::to_escaped(L"\xD834-\x1234")) << "Invalid surrogate pair 1";
    EXPECT_EQ(L"\xD834-\xDD1E", json::to_escaped(L"\xD834-\xDD1E")) << "Invalid surrogate pair 2";
}

TEST(JsonCommonTest, TestToUnescaped)
{
    EXPECT_EQ(L"ABC", json::to_unescaped(L"ABC"));
    EXPECT_EQ(L"ABC", json::to_unescaped(L"\\u0041\\u0042\\u0043"));
    EXPECT_EQ(L"\x0001\x0019\x0022\\", json::to_unescaped(L"\\u0001\\u0019\\u0022\\u005C"));
    EXPECT_EQ(L"\\u123X", json::to_unescaped(L"\\u123X"));
}

}
