#pragma once
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "ioutils.h"
#include <fstream>
#include "testutils.h"
#include "strutils.h"

using namespace std;
using namespace stdext;
using namespace locutils;

namespace ioutils_test
{

const wstring test_file_name = L"current.txt";

class IOUtilsTest : public testing::Test
{
protected:
    void GenerateTestWstring(wstring& ws)
    {
        ws.clear();
        ws.reserve(0xFFFF);
        for (int i = 0x1; i <= 0xFFFF; i++)
        {
            wchar_t c = static_cast<wchar_t>(i);
            if (utf16::is_noncharacter(c) || utf16::is_high_surrogate(c))
                c = utf16::replacement_character;
            ws += c;
        }
    }

    void CheckFileWriteAndRead(const wstring& s, const wstring& expected,
                               const ioutils::text_io_options& options, const wstring title)
    {
        wstring title2 = title + L": ";
        {
            ioutils::text_writer w(test_file_name, options);
            w.write(s);
        }
        ioutils::text_reader r(test_file_name, options);
        wstring s2;
        s2.reserve(s.length());
        r.read_all(s2);
        EXPECT_EQ(expected.length(), s2.length()) << title2 + L"Length";
        EXPECT_EQ(expected, s2) << title2 + L"Content";
    }

};
    TEST_F(IOUtilsTest, TestTextIOOptions)
    {
        ioutils::text_io_options_ansi opt_ansi;
        EXPECT_TRUE(opt_ansi.encoding() == ioutils::text_encoding::ansi) << L"ANSI encoding";
        EXPECT_TRUE(opt_ansi.use_default_encoding()) << L"ANSI locale_default";
        ioutils::text_io_options_ansi opt_ansi2(".1252");
        EXPECT_FALSE(opt_ansi2.use_default_encoding()) << L"ANSI locale_default 2";
        EXPECT_EQ(".1252", opt_ansi2.ansi_encoding_name()) << L"ANSI locale_name 2";
        //
        ioutils::text_io_options_utf8 opt_utf8;
        EXPECT_TRUE(opt_utf8.encoding() == ioutils::text_encoding::utf8) << L"UTF-8 encoding";
        //
        ioutils::text_io_options_utf16 opt_utf16;
        EXPECT_TRUE(opt_utf16.encoding() == ioutils::text_encoding::utf16) << L"UTF-16 encoding";
    }

    TEST_F(IOUtilsTest, TestFileWriteAndRead_ANSI)
    {
        wstring ws1;
        ws1.reserve(255);
        for (int i = 0x01; i <= 0x7F; i++)
        {
            wchar_t c = static_cast<wchar_t>(i);
            ws1 += c;
        }
        CheckFileWriteAndRead(ws1, ws1, ioutils::text_io_options_ansi(), L"ASCII (default)");
        wstring ws2 = /*ws1 +*/ L"éèçàùÈÉÀÇÙïÏ";
        CheckFileWriteAndRead(ws2, ws2, ioutils::text_io_options_ansi(), L"ANSI (default)");
        wstring ws3 = ws1 + L"АаБбВвГгДдЕеЁёЖжЗзИиКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЫыЭэЮюЯя";
        string encoding =
#if defined(__STDEXT_USE_ICONV)
            "CP1251";
#else
            ".1251";
#endif
        CheckFileWriteAndRead(ws3, ws3, ioutils::text_io_options_ansi(encoding), L"ANSI (1251)");
    }

    TEST_F(IOUtilsTest, TestFileWriteAndRead_Utf8)
    {
        wstring ws1;
        GenerateTestWstring(ws1);
        wstring ws1_bom = ws1;
        utf16::add_bom(ws1_bom);
        CheckFileWriteAndRead(
            ws1, ws1, 
            ioutils::text_io_options_utf8(), 
            L"UTF-8 (NoBOM, default)");
        CheckFileWriteAndRead(
            ws1_bom, ws1,
            ioutils::text_io_options_utf8(),
            L"UTF-8 (BOM, default)");
        CheckFileWriteAndRead(
            ws1, ws1, 
            ioutils::text_io_options_utf8(codecvt_mode_base::headers::consume), 
            L"UTF-8 (NoBOM, consume)");
        CheckFileWriteAndRead(
            ws1_bom, ws1,
            ioutils::text_io_options_utf8(locutils::codecvt_mode_utf8(codecvt_mode_base::headers::consume)),
            L"UTF-8 (BOM, consume)");
        CheckFileWriteAndRead(
            ws1, ws1_bom, 
            ioutils::text_io_options_utf8(codecvt_mode_base::headers::generate), 
            L"UTF-8 (NoBOM, generate)");
        CheckFileWriteAndRead(
            ws1_bom, ws1_bom, 
            ioutils::text_io_options_utf8(locutils::codecvt_mode_utf8(codecvt_mode_base::headers::generate)), 
            L"UTF-8 (BOM, generate)");
    }

    TEST_F(IOUtilsTest, TestFileWriteAndRead_Utf16)
    {
        wstring ws1;
        GenerateTestWstring(ws1);
        wstring ws1_bom = ws1;
        utf16::add_bom(ws1_bom);
        CheckFileWriteAndRead(ws1, ws1, ioutils::text_io_options_utf16(), L"UTF-16 (NoBOM, default)");
        CheckFileWriteAndRead(ws1_bom, ws1, ioutils::text_io_options_utf16(), L"UTF-16 (BOM, default)");
        CheckFileWriteAndRead(
            ws1, ws1,
            ioutils::text_io_options_utf16(locutils::codecvt_mode_utf16(codecvt_mode_base::headers::consume)), 
            L"UTF-16 (NoBOM, default, consume)");
        CheckFileWriteAndRead(
            ws1_bom, ws1,
            ioutils::text_io_options_utf16(locutils::codecvt_mode_utf16(codecvt_mode_base::headers::consume)),
            L"UTF-16 (BOM, default, consume)");
        CheckFileWriteAndRead(
            ws1, ws1_bom,
            ioutils::text_io_options_utf16(locutils::codecvt_mode_utf16(codecvt_mode_base::headers::generate)),
            L"UTF-16 (NoBOM, default, generate)");
        CheckFileWriteAndRead(
            ws1_bom, ws1_bom,
            ioutils::text_io_options_utf16(locutils::codecvt_mode_utf16(codecvt_mode_base::headers::generate)),
            L"UTF-16 (BOM, default, generate)");
        CheckFileWriteAndRead(
            ws1, ws1,
            ioutils::text_io_options_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::little_endian, codecvt_mode_base::headers::consume)), 
            L"UTF-16 (NoBOM, LE, consume)");
        CheckFileWriteAndRead(
            ws1_bom, ws1,
            ioutils::text_io_options_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::little_endian, codecvt_mode_base::headers::consume)),
            L"UTF-16 (BOM, LE, consume)");
        CheckFileWriteAndRead(
            ws1, ws1_bom,
            ioutils::text_io_options_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::little_endian, codecvt_mode_base::headers::generate)),
            L"UTF-16 (NoBOM, LE, generate)");
        CheckFileWriteAndRead(
            ws1_bom, ws1_bom,
            ioutils::text_io_options_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::little_endian, codecvt_mode_base::headers::generate)),
            L"UTF-16 (BOM, LE, generate)");
        CheckFileWriteAndRead(
            ws1, ws1,
            ioutils::text_io_options_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::big_endian, codecvt_mode_base::headers::consume)),
            L"UTF-16 (NoBOM, BE, consume)");
        CheckFileWriteAndRead(
            ws1_bom, ws1,
            ioutils::text_io_options_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::big_endian, codecvt_mode_base::headers::consume)),
            L"UTF-16 (BOM, BE, consume)");
        CheckFileWriteAndRead(
            ws1, ws1_bom,
            ioutils::text_io_options_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::big_endian, codecvt_mode_base::headers::generate)),
            L"UTF-16 (NoBOM, BE, generate)");
        CheckFileWriteAndRead(
            ws1_bom, ws1_bom,
            ioutils::text_io_options_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::big_endian, codecvt_mode_base::headers::generate)),
            L"UTF-16 (BOM, BE, generate)");
    }

}
