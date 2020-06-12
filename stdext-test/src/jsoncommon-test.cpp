#include <gtest/gtest.h>
#include "jsoncommon.h"
#include "testutils.h"

using namespace std;
using namespace stdext;
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
}

TEST(JsonCommonTest, TestToUnescaped)
{
    EXPECT_EQ(L"ABC", json::to_unescaped(L"ABC"));
    EXPECT_EQ(L"ABC", json::to_unescaped(L"\\u0041\\u0042\\u0043"));
    EXPECT_EQ(L"\x0001\x0019\x0022\\", json::to_unescaped(L"\\u0001\\u0019\\u0022\\u005C"));
    EXPECT_EQ(L"\\u123X", json::to_unescaped(L"\\u123X"));
}

}
