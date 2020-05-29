#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "locutils.h"
#include <sstream>
#include "testutils.h"
#include "strutils.h"

using namespace std;
using namespace stdext;
using namespace locutils;
using namespace testutils;

namespace locale_utils_test
{

class LocaleUtilsTest : public testing::Test
{
protected:
    void CheckDetectEndianess(const wstring ws, wstring title)
    {
        wstring title2 = title + L": ";
        endianess::byte_order order;
        //
        string test = utf16::bom_str(endianess::platform_value()) +
            utf16::wchar_to_multibyte(ws, endianess::platform_value(), endianess::platform_value());
        ASSERT_TRUE(utf16::try_detect_byte_order(test.c_str(), test.length(), order)) << title2 + L"detect failed 2";
        ASSERT_TRUE(endianess::platform_value() == order) << title2 + L"unexpected (BOM, default)";
        //
        test = utf16::bom_str(endianess::inverse_platform_value()) +
            utf16::wchar_to_multibyte(ws, endianess::platform_value(), endianess::inverse_platform_value());
        ASSERT_TRUE(utf16::try_detect_byte_order(test.c_str(), test.length(), order)) << title2 + L"detect failed 3";
        ASSERT_TRUE(endianess::inverse_platform_value() == order) << title2 + L"unexpected (BOM, inverse)";
        //
        test = utf16::wchar_to_multibyte(ws, endianess::platform_value(), endianess::platform_value());
        ASSERT_TRUE(utf16::try_detect_byte_order(test.c_str(), test.length(), order)) << title2 + L"detect failed 1";
        ASSERT_TRUE(endianess::platform_value() == order) << title2 + L"unexpected (NoBOM, default)";
    }

    void CheckConverter_Utf8_8to16(const wstring expected, const string utf8str, const codecvt_mode_utf8& mode, const wstring title)
    {
        locutils::codecvt_utf8_wchar_t cvt(mode);
        wstring ws;
        int result = cvt.to_utf16(utf8str, ws);
        ASSERT_TRUE(result == cvt.ok) << title + L": result";
        ASSERT_EQ(expected.length(), ws.length()) << title + L": length";
        ASSERT_EQ(expected, ws) << title + L": compare";
    }

    void CheckConverter_Utf8_16to8(const string expected, const wstring utf16str, const codecvt_mode_utf8& mode, const wstring title)
    {
        locutils::codecvt_utf8_wchar_t cvt(mode);
        string s;
        int result = cvt.to_utf8(utf16str, s);
        ASSERT_TRUE(result == cvt.ok) << title + L": result";
        ASSERT_EQ(expected.length(), s.length()) << title + L": length";
        ASSERT_EQ(expected, s) << title + L": compare";
    }

    void CheckConverter_Utf16_Mbto16(const wstring expected, const string mbstr, const codecvt_mode_utf16& mode, const wstring title)
    {
        locutils::codecvt_utf16_wchar_t cvt(mode);
        wstring ws;
        int result = cvt.mb_to_utf16(mbstr, ws);
        ASSERT_TRUE(result == cvt.ok) << title + L": result";
        ASSERT_EQ(expected.length(), ws.length()) << title + L": length";
        ASSERT_EQ(expected, ws) << title + L": compare";
    }

    void CheckConverter_Utf16_16toMb(const string expected, const wstring ws, const codecvt_mode_utf16& mode, const wstring title)
    {
        locutils::codecvt_utf16_wchar_t cvt(mode);
        string mbs;
        int result = cvt.utf16_to_mb(ws, mbs);
        ASSERT_TRUE(result == cvt.ok) << title + L": result";
        ASSERT_EQ(expected.length(), mbs.length()) << title + L": length";
        ASSERT_EQ(expected, mbs) << title + L": compare";
    }

};

TEST_F(LocaleUtilsTest, TestSurrogatePair)
{
    wchar_t high = 0, low = 0;
    char32_t c = 0x10437;
    ASSERT_TRUE(utf16::is_surrogate_pair(c)) << L"1.1";
    ASSERT_TRUE(utf16::to_surrogate_pair(c, high, low)) << L"1.2";
    ASSERT_EQ(0xD801u, high) << L"1.3";
    ASSERT_EQ(0xDC37u, low) << L"1.4";
    c = 0;
    ASSERT_TRUE(utf16::from_surrogate_pair(high, low, c)) << L"2.1";
    ASSERT_EQ(0x10437u, c) << L"2.2";
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
    ASSERT_EQ(2, utf16::bom_be_str().length()) << L"BOM BE length";
    ASSERT_EQ(utf16::bom_be[0], utf16::bom_be_str()[0]) << L"BOM BE [0]";
    ASSERT_EQ(utf16::bom_be[1], utf16::bom_be_str()[1]) << L"BOM BE [1]";
    ASSERT_EQ(2, utf16::bom_le_str().length()) << L"BOM LE length";
    ASSERT_EQ(utf16::bom_le[0], utf16::bom_le_str()[0]) << L"BOM LE [0]";
    ASSERT_EQ(utf16::bom_le[1], utf16::bom_le_str()[1]) << L"BOM LE [1]";
    ASSERT_TRUE(utf16::is_bom(0xFFFEu)) << L"is_bom 1";
    ASSERT_TRUE(utf16::is_bom(0xFEFFu)) << L"is_bom 2";
}

TEST_F(LocaleUtilsTest, TestUtf16_DetectEndianess)
{
    CheckDetectEndianess(L"Test string", L"1");
    CheckDetectEndianess(L"ç'a été déjà detecté", L"2");
    CheckDetectEndianess(L"строка для теста", L"3");
    CheckDetectEndianess(L"Mix déjà строка", L"4");
}

TEST_F(LocaleUtilsTest, TestUtf8_BOM)
{
    ASSERT_EQ(3, utf8::bom_str().length()) << L"BOM length";
    ASSERT_EQ(utf8::bom[0], utf8::bom_str()[0]) << L"BOM char [0]";
    ASSERT_EQ(utf8::bom[1], utf8::bom_str()[1]) << L"BOM char [1]";
    ASSERT_EQ(utf8::bom[2], utf8::bom_str()[2]) << L"BOM char [2]";
    string s11 = u8"ABC";
    string s12 = utf8::bom_str() + s11;
    ASSERT_EQ(s11.length() + 3, s12.length()) << L"BOM length";
    ASSERT_EQ(utf8::bom_str() + s11, s12) << L"BOM default";

}

TEST_F(LocaleUtilsTest, TestSwapByteOrder)
{
    ASSERT_EQ(0x0u, utf16::swap_byte_order16(0x0)) << L"1.1";
    ASSERT_EQ(0xFF00u, utf16::swap_byte_order16(0xFF)) << L"1.2";
    ASSERT_EQ(0xFFu, utf16::swap_byte_order16(0xFF00)) << L"1.3";
    ASSERT_EQ(0x3412u, utf16::swap_byte_order16(0x1234)) << L"1.4";
    //
    ASSERT_EQ(0x0u, utf16::swap_byte_order32(0x0)) << L"2.1";
    ASSERT_EQ(0x00FF0000u, utf16::swap_byte_order32(0xFF)) << L"2.2";
    ASSERT_EQ(0xFF000000u, utf16::swap_byte_order32(0xFF00)) << L"2.3";
    ASSERT_EQ(0xFF00u, utf16::swap_byte_order32(0xFF000000)) << L"2.4";
    ASSERT_EQ(0xFFu, utf16::swap_byte_order32(0x00FF0000)) << L"2.5";
    ASSERT_EQ(0x56781234u, utf16::swap_byte_order32(0x12345678)) << L"2.6";
    //
    wstring s1 = L"ABC déjà строка";
    ASSERT_EQ(s1, utf16::swap_byte_order(utf16::swap_byte_order(s1))) << L"3";
}

TEST_F(LocaleUtilsTest, TestUtf16_WCharToMbytes)
{
    wstring ws1 = L"ABC déjà строка";
    string expected_le("\x41\x00\x42\x00\x43\x00\x20\x00\x64\x00\xe9\x00\x6a\x00\xe0\x00\x20\x00\x41\x04\x42\x04\x40\x04\x3e\x04\x3a\x04\x30\x04", ws1.length() * 2);
    string expected_be("\x00\x41\x00\x42\x00\x43\x00\x20\x00\x64\x00\xe9\x00\x6a\x00\xe0\x00\x20\x04\x41\x04\x42\x04\x40\x04\x3e\x04\x3a\x04\x30", ws1.length() * 2);
    string s1 = utf16::wchar_to_multibyte(ws1, endianess::platform_value(), endianess::platform_value());
    if (endianess::platform_value() == endianess::byte_order::little_endian)
        ASSERT_EQ(expected_le, s1) << L"default LE";
    else
        ASSERT_EQ(expected_be, s1) << L"default BE";
    string s2 = utf16::wchar_to_multibyte(ws1, endianess::platform_value(), endianess::inverse_platform_value());
    wstring ws2 = utf16::swap_byte_order(ws1);
    string s3 = utf16::wchar_to_multibyte(ws2, endianess::inverse_platform_value(), endianess::inverse_platform_value());
    if (endianess::inverse_platform_value() == endianess::byte_order::little_endian)
    {
        ASSERT_EQ(expected_le, s2) << L"default --> inverse LE";
        ASSERT_EQ(expected_le, s3) << L"inverse --> inverse LE";
    }
    else
    {
        ASSERT_EQ(expected_be, s2) << L"default --> inverse BE";
        ASSERT_EQ(expected_be, s3) << L"inverse --> inverse BE";
    }
}


TEST_F(LocaleUtilsTest, TestConverterMode_Utf8)
{
    {
        codecvt_mode_utf8 cm;
        ASSERT_TRUE(cm.consume_header()) << L"Utf8 consume_header 1";
        ASSERT_FALSE(cm.generate_header()) << L"Utf8 generate_header 1";
    }
    {
        codecvt_mode_utf8 cm;
        cm.headers_mode(codecvt_mode_base::headers::consume);
        ASSERT_TRUE(cm.consume_header()) << L"Utf8 consume_header 2";
        ASSERT_FALSE(cm.generate_header()) << L"Utf8 generate_header 2";
    }
    {
        codecvt_mode_utf8 cm;
        cm.headers_mode(codecvt_mode_base::headers::generate);
        ASSERT_FALSE(cm.consume_header()) << L"Utf8 consume_header 3";
        ASSERT_TRUE(cm.generate_header()) << L"Utf8 generate_header 3";
    }
}

TEST_F(LocaleUtilsTest, TestConverterMode_Utf16)
{
    {
        codecvt_mode_utf16 cm1;
        ASSERT_FALSE(cm1.is_byte_order_assigned()) << L"default is_byte_order_assigned";
        ASSERT_TRUE(cm1.byte_order() == endianess::platform_value()) << L"default byte_order";
        ASSERT_TRUE(cm1.headers_mode() == codecvt_mode_base::headers::consume) << L"default headers_mode";
        ASSERT_TRUE(cm1.consume_header()) << L"default consume_header";
        ASSERT_FALSE(cm1.generate_header()) << L"default generate_header";
    }
    endianess::byte_order inverse_order = endianess::byte_order::big_endian;
    if (inverse_order == endianess::platform_value())
        inverse_order = endianess::byte_order::little_endian;
    {
        codecvt_mode_utf16 cm2(inverse_order, codecvt_mode_base::headers::generate);
        ASSERT_TRUE(cm2.is_byte_order_assigned()) << L"is_byte_order_assigned 2";
        ASSERT_TRUE(cm2.byte_order() == inverse_order) << L"byte_order 2";
        ASSERT_FALSE(cm2.consume_header()) << L"consume_header 2";
        ASSERT_TRUE(cm2.generate_header()) << L"generate_header 2";
    }
    {
        codecvt_mode_utf16 cm3;
        cm3.byte_order(inverse_order);
        ASSERT_TRUE(cm3.is_byte_order_assigned()) << L"is_byte_order_assigned 3";
        cm3.headers_mode(codecvt_mode_base::headers::generate);
        ASSERT_TRUE(cm3.byte_order() == inverse_order) << L"byte_order 3";
        ASSERT_FALSE(cm3.consume_header()) << L"consume_header 3";
        ASSERT_TRUE(cm3.generate_header()) << L"generate_header 3";
    }
}

TEST_F(LocaleUtilsTest, TestStreamConverter_Utf8)
{
    wstring s1 = L"ABC déjà строка";
    wstring s1bom = s1;
    utf16::add_bom(s1bom);
    string s2 = u8"ABC déjà строка";
    string s2bom = s2;
    utf8::add_bom(s2bom);
    //string s3 = "ABC d\xC3\xA9j\xC3\xA0 \xD1\x81\xD1\x82\xD1\x80\xD0\xBE\xD0\xBA\xD0\xB0";
    {
        codecvt_mode_utf8 cm(codecvt_mode_utf8::headers::consume);
        CheckConverter_Utf8_8to16(s1, s2, cm, L"1.1 - NoBOM (consume)");
        CheckConverter_Utf8_8to16(s1, s2bom, cm, L"1.2 - BOM (consume)");
        //
        CheckConverter_Utf8_16to8(s2, s1, cm, L"1.3 - NoBOM (consume)");
        CheckConverter_Utf8_16to8(s2, s1bom, cm, L"1.4 - BOM (consume)");
    }
    {
        codecvt_mode_utf8 cm(codecvt_mode_utf8::headers::generate);
        CheckConverter_Utf8_8to16(s1bom, s2, cm, L"2.1 - NoBOM (generate)");
        CheckConverter_Utf8_8to16(s1bom, s2bom, cm, L"2.2 - BOM (generate)");
        //
        CheckConverter_Utf8_16to8(s2bom, s1, cm, L"2.3 - NoBOM (generate)");
        CheckConverter_Utf8_16to8(s2bom, s1bom, cm, L"2.4 - BOM (generate)");
    }
}

TEST_F(LocaleUtilsTest, TestStreamConverter_Utf8_Memory)
{
    string s1 = u8"ABC déjà строка";
    wstring ws1 = L"ABC déjà строка";
    memchecker chk;
    {
        string s2;
        wstring ws2;
        locutils::codecvt_utf8_wchar_t cvt;
        int result = cvt.to_utf16(s1, ws2);
        ASSERT_TRUE(result == cvt.ok);
        result = cvt.to_utf16(utf8::bom_str() + s1, ws2);
        ASSERT_TRUE(result == cvt.ok);
        result = cvt.to_utf8(ws1, s2);
        ASSERT_TRUE(result == cvt.ok);
    }
    chk.checkpoint();
    ASSERT_FALSE(chk.has_leaks()) << chk.wreport();
}


TEST_F(LocaleUtilsTest, TestStreamConverter_Utf16)
{
    wstring ws1 = L"ABC déjà строка";
    wstring ws1_bom = ws1;
    utf16::add_bom(ws1_bom);
    string s1_le = utf16::wchar_to_multibyte(ws1, endianess::platform_value(), endianess::byte_order::little_endian);
    string s1_le_bom = s1_le;
    utf16::add_bom(s1_le_bom, endianess::byte_order::little_endian);
    {
        codecvt_mode_utf16 cm(codecvt_mode_utf16::headers::consume);
        CheckConverter_Utf16_Mbto16(ws1, s1_le, cm, L"1.1 - NoBOM (default, consume)");
        CheckConverter_Utf16_Mbto16(ws1, s1_le_bom, cm, L"1.2 - BOM (default, consume)");
        CheckConverter_Utf16_16toMb(s1_le, ws1, cm, L"2.1 - NoBOM (default, consume)");
        CheckConverter_Utf16_16toMb(s1_le, ws1_bom, cm, L"2.2 - BOM (default, consume)");
    }
    {
        codecvt_mode_utf16 cm(codecvt_mode_utf16::headers::generate);
        CheckConverter_Utf16_Mbto16(ws1_bom, s1_le, cm, L"3.1 - NoBOM (default, generate)");
        CheckConverter_Utf16_Mbto16(ws1_bom, s1_le_bom, cm, L"3.2 - BOM (default, generate)");
        CheckConverter_Utf16_16toMb(s1_le_bom, ws1, cm, L"4.1 - NoBOM (default, generate)");
        CheckConverter_Utf16_16toMb(s1_le_bom, ws1_bom, cm, L"4.2 - BOM (default, generate)");
    }
    {
        codecvt_mode_utf16 cm(endianess::byte_order::little_endian, codecvt_mode_utf16::headers::consume);
        CheckConverter_Utf16_Mbto16(ws1, s1_le, cm, L"5.1 - NoBOM (LE, consume)");
        CheckConverter_Utf16_Mbto16(ws1, s1_le_bom, cm, L"5.2 - BOM (LE, consume)");
        CheckConverter_Utf16_16toMb(s1_le, ws1, cm, L"6.1 - NoBOM (LE, consume)");
        CheckConverter_Utf16_16toMb(s1_le, ws1_bom, cm, L"6.2 - BOM (LE, consume)");
    }
    {
        codecvt_mode_utf16 cm(endianess::byte_order::little_endian, codecvt_mode_utf16::headers::generate);
        CheckConverter_Utf16_Mbto16(ws1_bom, s1_le, cm, L"7.1 - NoBOM (LE, generate)");
        CheckConverter_Utf16_Mbto16(ws1_bom, s1_le_bom, cm, L"7.2 - BOM (LE, generate)");
        CheckConverter_Utf16_16toMb(s1_le_bom, ws1, cm, L"8.1 - NoBOM (LE, generate)");
        CheckConverter_Utf16_16toMb(s1_le_bom, ws1_bom, cm, L"8.2 - BOM (LE, generate)");
    }
    string s1_be = utf16::wchar_to_multibyte(ws1, endianess::platform_value(), endianess::byte_order::big_endian);
    string s1_be_bom = s1_be;
    utf16::add_bom(s1_be_bom, endianess::byte_order::big_endian);
    {
        codecvt_mode_utf16 cm(endianess::byte_order::big_endian, codecvt_mode_utf16::headers::consume);
        CheckConverter_Utf16_Mbto16(ws1, s1_be, cm, L"9.1 - NoBOM (BE, consume)");
        CheckConverter_Utf16_Mbto16(ws1, s1_be_bom, cm, L"9.2 - BOM (BE, consume)");
        CheckConverter_Utf16_16toMb(s1_be, ws1, cm, L"10.1 - NoBOM (BE, consume)");
        CheckConverter_Utf16_16toMb(s1_be, ws1_bom, cm, L"10.2 - BOM (BE, consume)");
    }
    {
        codecvt_mode_utf16 cm(endianess::byte_order::big_endian, codecvt_mode_utf16::headers::generate);
        CheckConverter_Utf16_Mbto16(ws1_bom, s1_be, cm, L"11.1 - NoBOM (BE, generate)");
        CheckConverter_Utf16_Mbto16(ws1_bom, s1_be_bom, cm, L"11.2 - BOM (BE, generate)");
        CheckConverter_Utf16_16toMb(s1_be_bom, ws1, cm, L"12.1 - NoBOM (BE, generate)");
        CheckConverter_Utf16_16toMb(s1_be_bom, ws1_bom, cm, L"12.2 - BOM (BE, generate)");
    }
}

TEST_F(LocaleUtilsTest, TestStreamConverter_Utf16_Memory)
{
    string s1 = u8"ABC déjà строка";
    wstring ws1 = L"ABC déjà строка";
    memchecker chk;
    {
        string s2;
        wstring ws2;
        locutils::codecvt_utf16_wchar_t cvt;
        int result = cvt.mb_to_utf16(s1, ws2);
        ASSERT_TRUE(result == cvt.ok);
        result = cvt.mb_to_utf16(utf8::bom_str() + s1, ws2);
        ASSERT_TRUE(result == cvt.ok);
        result = cvt.utf16_to_mb(ws1, s2);
        ASSERT_TRUE(result == cvt.ok);
    }
    chk.checkpoint();
    ASSERT_FALSE(chk.has_leaks()) << chk.wreport();
}

}
