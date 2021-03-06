﻿#include <gtest/gtest.h>
#include <sstream>
#include "locutils_test.h"
#include "testutils.h"
#include "strutils.h"

using namespace std;

namespace stdext
{
namespace locutils_test
{

class LocaleUtilsTest : public testing::Test
{
protected:
    void CheckDetectEndianess(const wstring ws, wstring title)
    {
        wstring title2 = title + L": ";
        endianess::byte_order order;
        //
        string test = locutils::utf16::wchar_to_multibyte(ws, endianess::platform_value());
        locutils::utf16::add_bom(test, endianess::platform_value());
        ASSERT_TRUE(locutils::utf16::try_detect_byte_order(test.c_str(), test.length(), order)) << title2 + L"detect failed 2";
        EXPECT_TRUE(endianess::platform_value() == order) << title2 + L"unexpected (BOM, default)";
        //
        test = locutils::utf16::wchar_to_multibyte(ws, endianess::inverse_platform_value());
        locutils::utf16::add_bom(test, endianess::inverse_platform_value());
        ASSERT_TRUE(locutils::utf16::try_detect_byte_order(test.c_str(), test.length(), order)) << title2 + L"detect failed 3";
        EXPECT_TRUE(endianess::inverse_platform_value() == order) << title2 + L"unexpected (BOM, inverse)";
        //
        test = locutils::utf16::wchar_to_multibyte(ws, endianess::platform_value());
        ASSERT_TRUE(locutils::utf16::try_detect_byte_order(test.c_str(), test.length(), order)) << title2 + L"detect failed 1";
        EXPECT_TRUE(endianess::platform_value() == order) << title2 + L"unexpected (NoBOM, default)";
    }

    void CheckConverter_Utf8_8to16(const wstring expected, const string utf8str, const locutils::codecvt_mode_utf8& mode, const wstring title)
    {
        locutils::codecvt_utf8_wchar_t cvt(mode);
        wstring ws;
        int result = cvt.to_utf16(utf8str, ws);
        EXPECT_EQ(result, cvt.ok) << title + L": result";
        EXPECT_EQ(expected.length(), ws.length()) << title + L": length";
        EXPECT_EQ(expected, ws) << title + L": compare";
    }

    void CheckConverter_Utf8_16to8(const string expected, const wstring utf16str, const locutils::codecvt_mode_utf8& mode, const wstring title)
    {
        locutils::codecvt_utf8_wchar_t cvt(mode);
        string s;
        int result = cvt.to_utf8(utf16str, s);
        EXPECT_EQ(result, cvt.ok) << title + L": result";
        EXPECT_EQ(expected.length(), s.length()) << title + L": length";
        EXPECT_EQ(expected, s) << title + L": compare";
    }

    void CheckConverter_Utf16_Mbto16(const wstring expected, const string mbstr, const locutils::codecvt_mode_utf16& mode, const wstring title)
    {
        locutils::codecvt_utf16_wchar_t cvt(mode);
        wstring ws;
        int result = cvt.mb_to_utf16(mbstr, ws);
        EXPECT_EQ(result, cvt.ok) << title + L": result";
        EXPECT_EQ(expected.length(), ws.length()) << title + L": length";
        EXPECT_EQ(expected, ws) << title + L": compare";
    }

    void CheckConverter_Utf16_16toMb(const string expected, const wstring ws, const locutils::codecvt_mode_utf16& mode, const wstring title)
    {
        locutils::codecvt_utf16_wchar_t cvt(mode);
        string mbs;
        int result = cvt.utf16_to_mb(ws, mbs);
        EXPECT_EQ(result, cvt.ok) << title + L": result";
        EXPECT_EQ(expected.length(), mbs.length()) << title + L": length";
        EXPECT_EQ(expected, mbs) << title + L": compare";
    }

    void CheckConverter_Ansi_Utf16ToAnsi(const string expected, const wstring ws, const locutils::codecvt_mode_ansi& mode, const wstring title)
    {
        locutils::codecvt_ansi_utf16_wchar_t cvt(mode);
        string ansi;
        wstring title2 = title + L" - " + str::to_wstring(mode.encoding_name_iconv()) + L": ";
        int result = cvt.utf16_to_ansi(ws, ansi);
        EXPECT_EQ(result, cvt.ok) << title2 + L"result";
        EXPECT_EQ(expected.length(), ansi.length()) << title2 + L"length";
        EXPECT_EQ(expected, ansi) << title2 + L"compare";
    }

    void CheckConverter_Ansi_AnsiToUtf16(const wstring expected, const string s, const locutils::codecvt_mode_ansi& mode, const wstring title)
    {
        locutils::codecvt_ansi_utf16_wchar_t cvt(mode);
        wstring ws;
        wstring title2 = title + L" - " + str::to_wstring(mode.encoding_name_iconv()) + L": ";
        int result = cvt.ansi_to_utf16(s, ws);
        EXPECT_EQ(result, cvt.ok) << title2 + L"result";
        EXPECT_EQ(expected.length(), ws.length()) << title2 + L"length";
        EXPECT_EQ(expected, ws) << title2 + L"compare";
    }
};

TEST_F(LocaleUtilsTest, TestSurrogatePair)
{
    using namespace locutils;
    EXPECT_LT(utf16::high_surrogate_min, utf16::high_surrogate_max);
    ASSERT_TRUE(utf16::is_high_surrogate(utf16::high_surrogate_min));
    ASSERT_TRUE(utf16::is_high_surrogate(utf16::high_surrogate_max));
    ASSERT_TRUE(utf16::is_high_surrogate(utf16::high_surrogate_min + 1));
    ASSERT_TRUE(utf16::is_low_surrogate(utf16::low_surrogate_min));
    ASSERT_TRUE(utf16::is_low_surrogate(utf16::low_surrogate_max));
    ASSERT_TRUE(utf16::is_low_surrogate(utf16::low_surrogate_min + 1));
    wchar_t high = 0, low = 0;
    char32_t c = 0x10437;
    ASSERT_TRUE(utf16::is_surrogate_pair(c)) << L"1.1";
    ASSERT_TRUE(utf16::to_surrogate_pair(c, high, low)) << L"1.2";
    EXPECT_EQ((wchar_t)0xD801u, high) << L"1.3";
    EXPECT_EQ((wchar_t)0xDC37u, low) << L"1.4";
    EXPECT_TRUE(utf16::is_surrogate_pair(high, low)) << L"1.5";
    EXPECT_FALSE(utf16::is_surrogate_pair(low, high)) << L"1.6";
    c = 0;
    ASSERT_TRUE(utf16::from_surrogate_pair(high, low, c)) << L"2.1";
    EXPECT_EQ(0x10437u, c) << L"2.2";
    c = 0xFFFF;
    ASSERT_FALSE(utf16::to_surrogate_pair(c, high, low)) << L"3.1";
    high = utf16::high_surrogate_min - 1;
    low = 0xDC37u;
    ASSERT_FALSE(utf16::from_surrogate_pair(high, low, c)) << L"3.2";
    high = 0xD801u;
    low = utf16::low_surrogate_min - 1;
    ASSERT_FALSE(utf16::from_surrogate_pair(high, low, c)) << L"3.3";
}

TEST_F(LocaleUtilsTest, TestUtf16_BOM)
{
    using namespace locutils;
    EXPECT_TRUE(utf16::is_bom(0xFEFFu)) << L"is_bom 1";
    EXPECT_FALSE(utf16::is_bom(0xFFFEu)) << L"is_bom 2";
    //
    string mbs_be;
    utf16::add_bom(mbs_be, endianess::byte_order::big_endian);
    ASSERT_EQ(2u, mbs_be.length()) << L"BOM BE length";
    EXPECT_EQ(utf16::bom_be[0], mbs_be[0]) << L"BOM BE [0]";
    EXPECT_EQ(utf16::bom_be[1], mbs_be[1]) << L"BOM BE [1]";
    utf16::add_bom(mbs_be, endianess::byte_order::big_endian);
    ASSERT_EQ(2u, mbs_be.length()) << L"BOM BE length 2";
    //
    string mbs_le;
    utf16::add_bom(mbs_le, endianess::byte_order::little_endian);
    ASSERT_EQ(2u, mbs_le.length()) << L"BOM LE length";
    EXPECT_EQ(utf16::bom_le[0], mbs_le[0]) << L"BOM LE [0]";
    EXPECT_EQ(utf16::bom_le[1], mbs_le[1]) << L"BOM LE [1]";
}

TEST_F(LocaleUtilsTest, TestUtf16_DetectEndianess)
{
    CheckDetectEndianess(L"Test string", L"1");
    CheckDetectEndianess(L"ç'a été déjà detecté", L"2");
    CheckDetectEndianess(L"строка для теста", L"3");
    CheckDetectEndianess(L"Mix déjà строка", L"4");
}

TEST_F(LocaleUtilsTest, TestUtf16_ChangeCase)
{
    using namespace locutils;
    EXPECT_EQ(L"", utf16::to_upper(L""));
    EXPECT_EQ(L"", utf16::to_lower(L""));
    EXPECT_EQ(L"ABC1", utf16::to_upper(L"abc1"));
    EXPECT_EQ(L"abc1", utf16::to_lower(L"ABC1"));
    // French  ÀÇÈÉ <-> àçèé
    EXPECT_EQ(L"ABC \x00C0\x00C7\x00C8\x00C9", utf16::to_upper(L"abc \x00E0\x00E7\x00E8\x00E9"));
    EXPECT_EQ(L"abc \x00E0\x00E7\x00E8\x00E9", utf16::to_lower(L"ABC \x00C0\x00C7\x00C8\x00C9"));
    // Cyrillic ЭЮЯ <-> эюя
    EXPECT_EQ(L"ABC \x042D\x042E\x042F", utf16::to_upper(L"abc \x044D\x044E\x044F"));
    EXPECT_EQ(L"abc \x044D\x044E\x044F", utf16::to_lower(L"ABC \x042D\x042E\x042F"));
}

TEST_F(LocaleUtilsTest, TestUtf16_EqualCaseInsensitive)
{
    using namespace locutils;
    EXPECT_TRUE(utf16::equal_ci(L"", L""));
    EXPECT_TRUE(utf16::equal_ci(L"ABC123", L"abc123"));
    // French  ÀÇÈÉ == àçèé
    EXPECT_TRUE(utf16::equal_ci(L"\x00C0\x00C7\x00C8\x00C9", L"\x00E0\x00E7\x00E8\x00E9"));
    // Cyrillic ЭЮЯ == эюя
    EXPECT_TRUE(utf16::equal_ci(L"\x042D\x042E\x042F", L"\x044D\x044E\x044F"));
}

TEST_F(LocaleUtilsTest, TestUtf8_BOM)
{
    using namespace locutils;
    string s1;
    utf8::add_bom(s1);
    ASSERT_EQ(3u, s1.length()) << L"BOM length 1.1";
    EXPECT_EQ(utf8::bom[0], s1[0]) << L"BOM char [0]";
    EXPECT_EQ(utf8::bom[1], s1[1]) << L"BOM char [1]";
    EXPECT_EQ(utf8::bom[2], s1[2]) << L"BOM char [2]";
    utf8::add_bom(s1);
    ASSERT_EQ(3u, s1.length()) << L"BOM length 1.2";
    //
    string s11 = u8"ABC";
    string s12 = s11;
    utf8::add_bom(s12);
    EXPECT_EQ(s11.length() + 3, s12.length()) << L"BOM length 2.1";
    EXPECT_EQ(utf8::bom[0], s12[0]) << L"BOM char [0]";
    EXPECT_EQ(utf8::bom[1], s12[1]) << L"BOM char [1]";
    EXPECT_EQ(utf8::bom[2], s12[2]) << L"BOM char [2]";
    utf8::add_bom(s12);
    EXPECT_EQ(s11.length() + 3, s12.length()) << L"BOM length 2.2";
}

TEST_F(LocaleUtilsTest, TestSwapByteOrder)
{
    using namespace locutils;
    EXPECT_EQ(0x0u, utf16::swap_byte_order16(0x0u)) << L"1.1";
    EXPECT_EQ(0xFF00u, utf16::swap_byte_order16(0xFFu)) << L"1.2";
    EXPECT_EQ(0xFFu, utf16::swap_byte_order16(0xFF00u)) << L"1.3";
    EXPECT_EQ(0x3412u, utf16::swap_byte_order16(0x1234u)) << L"1.4";
    //
    EXPECT_EQ(0x0u, utf16::swap_byte_order32(0x0u)) << L"2.1";
    EXPECT_EQ(0xFF000000u, utf16::swap_byte_order32(0x000000FFu)) << L"2.2";
    EXPECT_EQ(0x00FF0000u, utf16::swap_byte_order32(0x0000FF00u)) << L"2.3";
    EXPECT_EQ(0x0000FF00u, utf16::swap_byte_order32(0x00FF0000u)) << L"2.4";
    EXPECT_EQ(0x000000FFu, utf16::swap_byte_order32(0xFF000000u)) << L"2.5";
    EXPECT_EQ(0x12345678u, utf16::swap_byte_order32(0x78563412u)) << L"2.6";
    EXPECT_EQ(0x78563412u, utf16::swap_byte_order32(0x12345678u)) << L"2.7";
    //
    wstring s1 = locutils_test::string_01_utf16;
    EXPECT_EQ(s1, utf16::swap_byte_order(utf16::swap_byte_order(s1))) << L"3";
}

TEST_F(LocaleUtilsTest, TestCharRanges)
{
    locutils::wchar_range wcr1;
    EXPECT_EQ(wcr1.min(), 0x0001);
    EXPECT_EQ(wcr1.max(), 0xFFFF);
    EXPECT_TRUE(wcr1.contains(1));
    EXPECT_TRUE(wcr1.contains(100));
    EXPECT_TRUE(wcr1.contains(0xFFFF - 1));
    EXPECT_TRUE(wcr1.contains(0xFFFF));
    wstring ws;
    for (int i = wcr1.min(); i <= wcr1.max(); i++)
        ws += (wchar_t)i;
    EXPECT_TRUE(wcr1.contains_all(ws.c_str(), ws.length()));
    EXPECT_TRUE(wcr1.contains_all(ws));
    wcr1.min(32);
    wcr1.max(127);
    EXPECT_EQ(wcr1.min(), 32);
    EXPECT_EQ(wcr1.max(), 127);
    EXPECT_TRUE(wcr1.contains(33));
    EXPECT_FALSE(wcr1.contains(255));
    ws.clear();
    for (int i = wcr1.min(); i <= wcr1.max(); i++)
        ws += (wchar_t)i;
    EXPECT_TRUE(wcr1.contains_all(ws.c_str(), ws.length()));
    EXPECT_TRUE(wcr1.contains_all(ws));
}

TEST_F(LocaleUtilsTest, TestStringConvert)
{
    wstring ws = locutils_test::string_01_utf16;
    EXPECT_EQ(locutils_test::string_01_utf8, locutils::utf16::to_utf8string(ws));
    EXPECT_EQ(ws, locutils::utf8::to_utf16string(locutils::utf16::to_utf8string(ws)));
}

/*
 * Converters
 */
TEST_F(LocaleUtilsTest, TestUtf16_WCharToMbytes)
{
    wstring ws1 = locutils_test::string_01_utf16;
    string expected_le, expected_be;
    expected_le.assign("\x41\x00\x42\x00\x43\x00\x20\x00\x64\x00\xe9\x00\x6a\x00\xe0\x00\x20\x00\x41\x04\x42\x04\x40\x04\x3e\x04\x3a\x04\x30\x04",
                       ws1.length() * locutils::utf16::bytes_per_character);
    expected_be.assign("\x00\x41\x00\x42\x00\x43\x00\x20\x00\x64\x00\xe9\x00\x6a\x00\xe0\x00\x20\x04\x41\x04\x42\x04\x40\x04\x3e\x04\x3a\x04\x30",
                       ws1.length() * locutils::utf16::bytes_per_character);
    wstring ws1_bom = ws1;
    locutils::utf16::add_bom(ws1_bom);
    string expected_le_bom = "\xFF\xFE" + expected_le;
    string expected_be_bom = "\xFE\xFF" + expected_be;
    //
    string s1 = locutils::utf16::wchar_to_multibyte(ws1, endianess::platform_value());
    string s2 = locutils::utf16::wchar_to_multibyte(ws1, endianess::inverse_platform_value());
    string s1_bom = locutils::utf16::wchar_to_multibyte(ws1_bom, endianess::platform_value());
    string s2_bom = locutils::utf16::wchar_to_multibyte(ws1_bom, endianess::inverse_platform_value());
    if (endianess::platform_value() == endianess::byte_order::little_endian)
    {
        EXPECT_EQ(expected_le, s1) << L"platform LE 1";
        EXPECT_EQ(expected_be, s2) << L"platform LE 2";
        EXPECT_EQ(expected_le_bom, s1_bom) << L"platform LE BOM 1";
        EXPECT_EQ(expected_be_bom, s2_bom) << L"platform LE BOM 2";
    }
    else
    {
        EXPECT_EQ(expected_be, s1) << L"platform BE 1";
        EXPECT_EQ(expected_le, s2) << L"platform BE 2";
        EXPECT_EQ(expected_be_bom, s1_bom) << L"platform BE BOM 1";
        EXPECT_EQ(expected_le_bom, s2_bom) << L"platform BE BOM 2";
    }

}


TEST_F(LocaleUtilsTest, TestConverterMode_Utf8)
{
    {
        locutils::codecvt_mode_utf8 cm;
        ASSERT_TRUE(cm.consume_header()) << L"Utf8 consume_header 1";
        ASSERT_FALSE(cm.generate_header()) << L"Utf8 generate_header 1";
    }
    {
        locutils::codecvt_mode_utf8 cm;
        cm.headers_mode(locutils::codecvt_headers::consume);
        ASSERT_TRUE(cm.consume_header()) << L"Utf8 consume_header 2";
        ASSERT_FALSE(cm.generate_header()) << L"Utf8 generate_header 2";
    }
    {
        locutils::codecvt_mode_utf8 cm;
        cm.headers_mode(locutils::codecvt_headers::generate);
        ASSERT_FALSE(cm.consume_header()) << L"Utf8 consume_header 3";
        ASSERT_TRUE(cm.generate_header()) << L"Utf8 generate_header 3";
    }
}

TEST_F(LocaleUtilsTest, TestConverterMode_Utf16)
{
    {
        locutils::codecvt_mode_utf16 cm1;
        ASSERT_FALSE(cm1.is_byte_order_assigned()) << L"default is_byte_order_assigned";
        ASSERT_TRUE(cm1.byte_order() == endianess::platform_value()) << L"default byte_order";
        ASSERT_TRUE(cm1.headers_mode() == locutils::codecvt_headers::consume) << L"default headers_mode";
        ASSERT_TRUE(cm1.consume_header()) << L"default consume_header";
        ASSERT_FALSE(cm1.generate_header()) << L"default generate_header";
    }
    endianess::byte_order inverse_order = endianess::byte_order::big_endian;
    if (inverse_order == endianess::platform_value())
        inverse_order = endianess::byte_order::little_endian;
    {
        locutils::codecvt_mode_utf16 cm2(inverse_order, locutils::codecvt_headers::generate);
        ASSERT_TRUE(cm2.is_byte_order_assigned()) << L"is_byte_order_assigned 2";
        ASSERT_TRUE(cm2.byte_order() == inverse_order) << L"byte_order 2";
        ASSERT_FALSE(cm2.consume_header()) << L"consume_header 2";
        ASSERT_TRUE(cm2.generate_header()) << L"generate_header 2";
    }
    {
        locutils::codecvt_mode_utf16 cm3;
        cm3.byte_order(inverse_order);
        ASSERT_TRUE(cm3.is_byte_order_assigned()) << L"is_byte_order_assigned 3";
        cm3.headers_mode(locutils::codecvt_headers::generate);
        ASSERT_TRUE(cm3.byte_order() == inverse_order) << L"byte_order 3";
        ASSERT_FALSE(cm3.consume_header()) << L"consume_header 3";
        ASSERT_TRUE(cm3.generate_header()) << L"generate_header 3";
    }
}

TEST_F(LocaleUtilsTest, TestStreamConverter_Utf8)
{
    wstring s1 = locutils_test::string_01_utf16;
    wstring s1bom = s1;
    locutils::utf16::add_bom(s1bom);
    string s2 = locutils_test::string_01_utf8;
    string s2bom = s2;
    locutils::utf8::add_bom(s2bom);
    {
        locutils::codecvt_mode_utf8 cm(locutils::codecvt_headers::consume);
        CheckConverter_Utf8_8to16(s1, s2, cm, L"1.1 - NoBOM (consume)");
        CheckConverter_Utf8_8to16(s1, s2bom, cm, L"1.2 - BOM (consume)");
        //
        CheckConverter_Utf8_16to8(s2, s1, cm, L"1.3 - NoBOM (consume)");
        CheckConverter_Utf8_16to8(s2, s1bom, cm, L"1.4 - BOM (consume)");
    }
    {
        locutils::codecvt_mode_utf8 cm(locutils::codecvt_headers::generate);
        CheckConverter_Utf8_8to16(s1bom, s2, cm, L"2.1 - NoBOM (generate)");
        CheckConverter_Utf8_8to16(s1bom, s2bom, cm, L"2.2 - BOM (generate)");
        //
        CheckConverter_Utf8_16to8(s2bom, s1, cm, L"2.3 - NoBOM (generate)");
        CheckConverter_Utf8_16to8(s2bom, s1bom, cm, L"2.4 - BOM (generate)");
    }
}

TEST_F(LocaleUtilsTest, TestStreamConverter_Utf8_Partial)
{
    wstring ws1 = locutils_test::string_01_utf16;
    string s1 = locutils_test::string_01_utf8;
    size_t expected_count = s1.length() - 2; // last character is two-byte
    string partial_str = s1.substr(0, s1.length() - 1);
    wstring expected = ws1.substr(0, ws1.length() - 1);
    {
        locutils::codecvt_mode_utf8 cm(locutils::codecvt_headers::consume);
        locutils::codecvt_utf8_wchar_t cvt(cm);
        mbstate_t state = {};
        size_t count;
        wstring ws;
        int result = cvt.to_utf16(state, partial_str, ws, count);
        EXPECT_EQ(result, cvt.partial) << L"Partial: result";
        EXPECT_EQ(expected_count, count) << L"Partial: expected_count";
        EXPECT_EQ(expected, ws) << L"Partial: compare";
    }
}

TEST_F(LocaleUtilsTest, TestStreamConverter_Utf8_SurrogatePair)
{
    // 'G clef' symbol (U+1D11E)
    // UTF-8: 0xF0 0x9D 0x84 0x9E
    // UTF-16: 0xD834 0xDD1E
    wstring ws1 = L"ABC \xD834\xDD1E";
    string s1 = "ABC \xF0\x9D\x84\x9E";
    locutils::codecvt_mode_utf8 cm(locutils::codecvt_headers::consume);
    CheckConverter_Utf8_8to16(ws1, s1, cm, L"1.1 - NoBOM (consume)");
    CheckConverter_Utf8_16to8(s1, ws1, cm, L"1.2 - NoBOM (consume)");
    wstring ws2 = L"\xD834 ABC \xDD1E";
    string s2 = "\xED\xA0\xB4 ABC \xED\xB4\x9E";
    CheckConverter_Utf8_8to16(ws2, s2, cm, L"2.1 - NoBOM (consume)");
    CheckConverter_Utf8_16to8(s2, ws2, cm, L"2.2 - NoBOM (consume)");
}

TEST_F(LocaleUtilsTest, TestStreamConverter_Utf8_Memory)
{
    string s1 = locutils_test::string_01_utf8;
    string s1_bom = s1;
    locutils::utf8::add_bom(s1_bom);
    wstring ws1 = locutils_test::string_01_utf16;
    testutils::memchecker chk;
    {
        string s2;
        wstring ws2;
        locutils::codecvt_utf8_wchar_t cvt;
        int result = cvt.to_utf16(s1, ws2);
        ASSERT_TRUE(result == cvt.ok);
        result = cvt.to_utf16(s1_bom, ws2);
        ASSERT_TRUE(result == cvt.ok);
        result = cvt.to_utf8(ws1, s2);
        ASSERT_TRUE(result == cvt.ok);
    }
    chk.checkpoint();
    ASSERT_FALSE(chk.has_leaks()) << chk.wreport();
}


TEST_F(LocaleUtilsTest, TestStreamConverter_Utf16)
{
    wstring ws1 = locutils_test::string_01_utf16;
    wstring ws1_bom = ws1;
    locutils::utf16::add_bom(ws1_bom);
    string s1_le = locutils::utf16::wchar_to_multibyte(ws1, endianess::byte_order::little_endian);
    string s1_le_bom = s1_le;
    locutils::utf16::add_bom(s1_le_bom, endianess::byte_order::little_endian);
    {
        locutils::codecvt_mode_utf16 cm(locutils::codecvt_headers::consume);
        CheckConverter_Utf16_Mbto16(ws1, s1_le, cm, L"1.1 - NoBOM (default, consume)");
        CheckConverter_Utf16_Mbto16(ws1, s1_le_bom, cm, L"1.2 - BOM (default, consume)");
        CheckConverter_Utf16_16toMb(s1_le, ws1, cm, L"2.1 - NoBOM (default, consume)");
        CheckConverter_Utf16_16toMb(s1_le, ws1_bom, cm, L"2.2 - BOM (default, consume)");
    }
    {
        locutils::codecvt_mode_utf16 cm(locutils::codecvt_headers::generate);
        CheckConverter_Utf16_Mbto16(ws1_bom, s1_le, cm, L"3.1 - NoBOM (default, generate)");
        CheckConverter_Utf16_Mbto16(ws1_bom, s1_le_bom, cm, L"3.2 - BOM (default, generate)");
        CheckConverter_Utf16_16toMb(s1_le_bom, ws1, cm, L"4.1 - NoBOM (default, generate)");
        CheckConverter_Utf16_16toMb(s1_le_bom, ws1_bom, cm, L"4.2 - BOM (default, generate)");
    }
    {
        locutils::codecvt_mode_utf16 cm(endianess::byte_order::little_endian, locutils::codecvt_headers::consume);
        CheckConverter_Utf16_Mbto16(ws1, s1_le, cm, L"5.1 - NoBOM (LE, consume)");
        CheckConverter_Utf16_Mbto16(ws1, s1_le_bom, cm, L"5.2 - BOM (LE, consume)");
        CheckConverter_Utf16_16toMb(s1_le, ws1, cm, L"6.1 - NoBOM (LE, consume)");
        CheckConverter_Utf16_16toMb(s1_le, ws1_bom, cm, L"6.2 - BOM (LE, consume)");
    }
    {
        locutils::codecvt_mode_utf16 cm(endianess::byte_order::little_endian, locutils::codecvt_headers::generate);
        CheckConverter_Utf16_Mbto16(ws1_bom, s1_le, cm, L"7.1 - NoBOM (LE, generate)");
        CheckConverter_Utf16_Mbto16(ws1_bom, s1_le_bom, cm, L"7.2 - BOM (LE, generate)");
        CheckConverter_Utf16_16toMb(s1_le_bom, ws1, cm, L"8.1 - NoBOM (LE, generate)");
        CheckConverter_Utf16_16toMb(s1_le_bom, ws1_bom, cm, L"8.2 - BOM (LE, generate)");
    }
    string s1_be = locutils::utf16::wchar_to_multibyte(ws1, endianess::byte_order::big_endian);
    string s1_be_bom = s1_be;
    locutils::utf16::add_bom(s1_be_bom, endianess::byte_order::big_endian);
    {
        locutils::codecvt_mode_utf16 cm(endianess::byte_order::big_endian, locutils::codecvt_headers::consume);
        CheckConverter_Utf16_Mbto16(ws1, s1_be, cm, L"9.1 - NoBOM (BE, consume)");
        CheckConverter_Utf16_Mbto16(ws1, s1_be_bom, cm, L"9.2 - BOM (BE, consume)");
        CheckConverter_Utf16_16toMb(s1_be, ws1, cm, L"10.1 - NoBOM (BE, consume)");
        CheckConverter_Utf16_16toMb(s1_be, ws1_bom, cm, L"10.2 - BOM (BE, consume)");
    }
    {
        locutils::codecvt_mode_utf16 cm(endianess::byte_order::big_endian, locutils::codecvt_headers::generate);
        CheckConverter_Utf16_Mbto16(ws1_bom, s1_be, cm, L"11.1 - NoBOM (BE, generate)");
        CheckConverter_Utf16_Mbto16(ws1_bom, s1_be_bom, cm, L"11.2 - BOM (BE, generate)");
        CheckConverter_Utf16_16toMb(s1_be_bom, ws1, cm, L"12.1 - NoBOM (BE, generate)");
        CheckConverter_Utf16_16toMb(s1_be_bom, ws1_bom, cm, L"12.2 - BOM (BE, generate)");
    }
}

TEST_F(LocaleUtilsTest, TestStreamConverter_Utf16_SurrogatePair)
{
    using namespace locutils;
    // 'G clef' symbol (U+1D11E)
    // UTF-16: 0xD834 0xDD1E
    // UTF-16 multybyte LE: 0x 34 D8 1E DD
    wstring ws1 = L"ABC \xD834\xDD1E";
    string s1_le = utf16::wchar_to_multibyte(ws1, endianess::byte_order::little_endian);
    string s1_be = utf16::wchar_to_multibyte(ws1, endianess::byte_order::big_endian);
    {
        codecvt_mode_utf16 cm(endianess::byte_order::little_endian, codecvt_headers::consume);
        CheckConverter_Utf16_Mbto16(ws1, s1_le, cm, L"1.1 - NoBOM (LE, consume)");
        CheckConverter_Utf16_16toMb(s1_le, ws1, cm, L"1.2 - NoBOM (LE, consume)");
    }
    {
        codecvt_mode_utf16 cm(endianess::byte_order::big_endian, codecvt_headers::consume);
        CheckConverter_Utf16_Mbto16(ws1, s1_be, cm, L"2.1 - NoBOM (BE, consume)");
        CheckConverter_Utf16_16toMb(s1_be, ws1, cm, L"2.2 - NoBOM (BE, consume)");
    }
    wstring ws2 = L"\xD834 ABC \xDD1E";
    string s2_le = utf16::wchar_to_multibyte(ws2, endianess::byte_order::little_endian);
    string s2_be = utf16::wchar_to_multibyte(ws2, endianess::byte_order::big_endian);
    {
        codecvt_mode_utf16 cm(endianess::byte_order::little_endian, codecvt_headers::consume);
        CheckConverter_Utf16_Mbto16(ws2, s2_le, cm, L"3.1 - NoBOM (LE, consume)");
        CheckConverter_Utf16_16toMb(s2_le, ws2, cm, L"3.2 - NoBOM (LE, consume)");
    }
    {
        codecvt_mode_utf16 cm(endianess::byte_order::big_endian, codecvt_headers::consume);
        CheckConverter_Utf16_Mbto16(ws2, s2_be, cm, L"4.1 - NoBOM (BE, consume)");
        CheckConverter_Utf16_16toMb(s2_be, ws2, cm, L"4.2 - NoBOM (BE, consume)");
    }
}

TEST_F(LocaleUtilsTest, TestStreamConverter_Utf16_Memory)
{
    string s1 = locutils_test::string_01_utf8;
    string s1_bom = s1;
    locutils::utf8::add_bom(s1_bom);
    wstring ws1 = locutils_test::string_01_utf16;
    testutils::memchecker chk;
    {
        string s2;
        wstring ws2;
        locutils::codecvt_utf16_wchar_t cvt;
        int result = cvt.mb_to_utf16(s1, ws2);
        ASSERT_TRUE(result == cvt.ok);
        result = cvt.mb_to_utf16(s1_bom, ws2);
        ASSERT_TRUE(result == cvt.ok);
        result = cvt.utf16_to_mb(ws1, s2);
        ASSERT_TRUE(result == cvt.ok);
    }
    chk.checkpoint();
    ASSERT_FALSE(chk.has_leaks()) << chk.wreport();
}

TEST_F(LocaleUtilsTest, TestStreamConverter_Ansi)
{
    wstring ws1 = locutils_test::string_01_utf16_cp1252;
    wstring ws1_bom = ws1;
    locutils::utf16::add_bom(ws1_bom);
    wstring ws2 = locutils_test::string_01_utf16_cp1251;
    wstring ws2_bom = ws2;
    locutils::utf16::add_bom(ws2_bom);
    string s1 = locutils_test::string_01_ansi_cp1252;
    string s2 = locutils_test::string_01_ansi_cp1251;
    {
        locutils::codecvt_mode_ansi cm(locutils::ansi_encoding::cp1252, locutils::codecvt_headers::consume);
        CheckConverter_Ansi_Utf16ToAnsi(s1, ws1, cm, L"1.1 - NoBOM (default, consume)");
        CheckConverter_Ansi_Utf16ToAnsi(s1, ws1_bom, cm, L"1.2 - BOM (default, consume)");
        CheckConverter_Ansi_AnsiToUtf16(ws1, s1, cm, L"2.1 - NoBOM (default, consume)");
    }
    {
        locutils::codecvt_mode_ansi cm(locutils::ansi_encoding::cp1251, locutils::codecvt_headers::consume);
        CheckConverter_Ansi_Utf16ToAnsi(s2, ws2, cm, L"3.1 - NoBOM (default, consume)");
        CheckConverter_Ansi_Utf16ToAnsi(s2, ws2_bom, cm, L"3.2 - BOM (default, consume)");
        CheckConverter_Ansi_AnsiToUtf16(ws2, s2, cm, L"3.3 - NoBOM (default, consume)");
    }
    {
        locutils::codecvt_mode_ansi cm(locutils::ansi_encoding::cp1252, locutils::codecvt_headers::generate);
        CheckConverter_Ansi_Utf16ToAnsi(s1, ws1, cm, L"5.1 - NoBOM (default, generate)");
        CheckConverter_Ansi_Utf16ToAnsi(s1, ws1_bom, cm, L"5.2 - BOM (default, generate)");
        CheckConverter_Ansi_AnsiToUtf16(ws1_bom, s1, cm, L"5.3 - NoBOM (default, generate)");
    }
    {
        locutils::codecvt_mode_ansi cm(locutils::ansi_encoding::cp1251, locutils::codecvt_headers::generate);
        CheckConverter_Ansi_Utf16ToAnsi(s2, ws2, cm, L"7.1 - NoBOM (default, generate)");
        CheckConverter_Ansi_Utf16ToAnsi(s2, ws2_bom, cm, L"7.2 - BOM (default, generate)");
        CheckConverter_Ansi_AnsiToUtf16(ws2_bom, s2, cm, L"7.3 - NoBOM (default, generate)");
    }
}

}
}
