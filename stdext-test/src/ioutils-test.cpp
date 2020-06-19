#include <gtest/gtest.h>
#include "ioutils.h"
#include <fstream>
#include "testutils.h"
#include "strutils.h"
#include "locutils-test.h"

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
            if (utf16::is_noncharacter(c))
                c = utf16::replacement_character;
            ws += c;
        }
    }

    void GenerateTestWstringAnsi(wstring& ws)
    {
        ws.reserve(127);
        string s1;
        for (int i = 0x01; i <= 0x7F; i++)
            ws += static_cast<wchar_t>(i);
    }

    void GenerateTestStringAnsi(string& s1)
    {
        s1.reserve(127);
        for (int i = 0x01; i <= 0x7F; i++)
            s1 += static_cast<char>(i);
    }

    void CheckFileWriteAndRead(const wstring& ws, const wstring& expected,
                               const ioutils::text_io_policy& policy, const wstring title)
    {
        wstring title2 = title + L" (file I/O): ";
        {
            ioutils::text_writer w(test_file_name, policy);
            w.write(ws);
        }
        ioutils::text_reader r(test_file_name, policy);
        wstring s2;
        s2.reserve(ws.length());
        r.read_all(s2);
        EXPECT_EQ(expected.length(), s2.length()) << title2 + L"Length";
        EXPECT_EQ(expected, s2) << title2 + L"Content";
    }

    void CheckStreamWriteAndRead(const string& bytes, const wstring& expected,
                                 const ioutils::text_io_policy& policy, const wstring title)
    {
        wstringstream wss;
        for (char c : bytes)
            wss << (wchar_t)((unsigned char)c);
        ioutils::text_reader reader(wss, policy);
        wstring ws;
        reader.read_all(ws);
        EXPECT_EQ(ws, expected) << title + L" (stream I/O)";
    }
};

TEST_F(IOUtilsTest, TestTextIOOptions)
{
    {
        ioutils::text_io_policy_ansi opt;
        EXPECT_EQ(opt.cvt_mode().encoding(), locutils::codecvt_mode_ansi::default_encoding()) << L"ANSI encoding 1";
        EXPECT_TRUE(opt.use_default_encoding()) << L"ANSI use default encoding 1";
        EXPECT_EQ("CP1252", opt.cvt_mode().encoding_name_iconv()) << L"ANSI encoding name 1.1";
        EXPECT_EQ(".1252", opt.cvt_mode().encoding_name_windows()) << L"ANSI encoding name 1.2";
    }
    {
        ioutils::text_io_policy_ansi opt(".1251");
        EXPECT_FALSE(opt.use_default_encoding()) << L"ANSI use default encoding 2";
        EXPECT_EQ(opt.cvt_mode().encoding(), locutils::ansi_encoding::by_name) << L"ANSI encoding 2";
        EXPECT_EQ(".1251", opt.cvt_mode().encoding_name_iconv()) << L"ANSI encoding name 2.1";
        EXPECT_EQ(".1251", opt.cvt_mode().encoding_name_windows()) << L"ANSI encoding name 2.2";
    }
    ioutils::text_io_policy_utf8 opt_utf8;
    EXPECT_TRUE(opt_utf8.cvt_mode().consume_header()) << L"UTF-8 encoding";
    //
    ioutils::text_io_policy_utf16 opt_utf16;
    EXPECT_TRUE(opt_utf16.cvt_mode().consume_header()) << L"UTF-16 encoding";
}

TEST_F(IOUtilsTest, TestWriteAndRead_ANSI)
{
    wstring ws1;
    GenerateTestWstringAnsi(ws1);
    string s1;
    GenerateTestStringAnsi(s1);
    {
        ioutils::text_io_policy_ansi policy;
        CheckStreamWriteAndRead(s1, ws1, policy, L"ASCII (default 1252)");
        CheckFileWriteAndRead(ws1, ws1, policy, L"ASCII (default 1252)");
    }
    {
        string s2 = "\xC0\xC7\xC8\xC9\xCF\xD9\xE0\xE7\xE8\xE9\xEF\xF9"; // "ÀÇÈÉÏÙàçèéïù" ANSI cp1252
        wstring ws2 = L"\u00C0\u00C7\u00C8\u00C9\u00CF\u00D9\u00E0\u00E7\u00E8\u00E9\u00EF\u00F9"; // "ÀÇÈÉÏÙàçèéïù" UTF-16
        ioutils::text_io_policy_ansi policy(locutils::ansi_encoding::cp1252);
        CheckStreamWriteAndRead(s2, ws2, policy, L"ANSI (1252)");
        CheckFileWriteAndRead(ws2, ws2, policy, L"ANSI (1252)");
    }
    {
        ioutils::text_io_policy_ansi policy(locutils::ansi_encoding::cp1251);
        string s3 = "\xC0\xC1\xC2\xDE\xDF\xE0\xE1\xE2\xFE\xFF"; // "АБВЮЯабвюя" ANSI cp1251
        wstring ws3 = L"\u0410\u0411\u0412\u042E\u042F\u0430\u0431\u0432\u044E\u044F"; // "АБВЮЯабвюя" UTF-16
        CheckStreamWriteAndRead(s3, ws3, policy, L"ANSI (1251)");
        CheckFileWriteAndRead(ws3, ws3, policy, L"ANSI (1251)");
    }
}

TEST_F(IOUtilsTest, TestWriteAndRead_Utf8)
{
    wstring ws1;
    GenerateTestWstring(ws1);
    wstring ws1_bom = ws1;
    utf16::add_bom(ws1_bom);
    {
        ioutils::text_io_policy_utf8 policy;
        CheckFileWriteAndRead(ws1, ws1, policy, L"UTF-8 (NoBOM, default)");
        CheckFileWriteAndRead(ws1_bom, ws1, policy, L"UTF-8 (BOM, default)");
    }
    {
        ioutils::text_io_policy_utf8 policy((locutils::codecvt_mode_utf8(codecvt_mode_base::headers::consume)));
        // Paranthesis added because of "most vexing parse" https://en.wikipedia.org/wiki/Most_vexing_parse
        CheckFileWriteAndRead(ws1, ws1, policy, L"UTF-8 (NoBOM, consume)");
        CheckFileWriteAndRead(ws1_bom, ws1, policy, L"UTF-8 (BOM, consume)");
    }
    {
        ioutils::text_io_policy_utf8 policy((locutils::codecvt_mode_utf8(codecvt_mode_base::headers::generate)));
        CheckFileWriteAndRead(ws1, ws1_bom, policy, L"UTF-8 (NoBOM, generate)");
        CheckFileWriteAndRead(ws1_bom, ws1_bom, policy, L"UTF-8 (BOM, generate)");
    }
}

TEST_F(IOUtilsTest, TestWriteAndRead_Utf8_SurrogatePair)
{
    // 'G clef' symbol (U+1D11E)
    // UTF-16: 0xD834 0xDD1E
    wstring ws1 = L"ABC \xD834\xDD1E";
    ioutils::text_io_policy_utf8 policy((locutils::codecvt_mode_utf8(codecvt_mode_base::headers::consume)));
    CheckFileWriteAndRead(ws1, ws1, policy, L"Pair (NoBOM, consume)");
    wstring ws2 = L"\xD834 ABC \xDD1E";
    CheckFileWriteAndRead(ws1, ws1, policy, L"Unpaired (NoBOM, consume)");
}

TEST_F(IOUtilsTest, TestWriteAndRead_Utf16)
{
    wstring ws1;
    GenerateTestWstring(ws1);
    wstring ws1_bom = ws1;
    utf16::add_bom(ws1_bom);
    {
        ioutils::text_io_policy_utf16 policy;
        CheckFileWriteAndRead(ws1, ws1, policy, L"UTF-16 (NoBOM, default)");
        CheckFileWriteAndRead(ws1_bom, ws1, policy, L"UTF-16 (BOM, default)");
    }
    {
        ioutils::text_io_policy_utf16 policy((locutils::codecvt_mode_utf16(codecvt_mode_base::headers::consume)));
        CheckFileWriteAndRead(ws1, ws1, policy, L"UTF-16 (NoBOM, default, consume)");
        CheckFileWriteAndRead(ws1_bom, ws1, policy, L"UTF-16 (BOM, default, consume)");
    }
    {
        ioutils::text_io_policy_utf16 policy((locutils::codecvt_mode_utf16(codecvt_mode_base::headers::generate)));
        CheckFileWriteAndRead(ws1, ws1_bom, policy, L"UTF-16 (NoBOM, default, generate)");
        CheckFileWriteAndRead(ws1_bom, ws1_bom, policy, L"UTF-16 (BOM, default, generate)");
    }
    {
        ioutils::text_io_policy_utf16 policy((locutils::codecvt_mode_utf16(endianess::byte_order::little_endian, codecvt_mode_base::headers::consume)));
        CheckFileWriteAndRead(ws1, ws1, policy, L"UTF-16 (NoBOM, LE, consume)");
        CheckFileWriteAndRead(ws1_bom, ws1, policy, L"UTF-16 (BOM, LE, consume)");
    }
    {
        ioutils::text_io_policy_utf16 policy((locutils::codecvt_mode_utf16(endianess::byte_order::little_endian, codecvt_mode_base::headers::generate)));
        CheckFileWriteAndRead(ws1, ws1_bom, policy, L"UTF-16 (NoBOM, LE, generate)");
        CheckFileWriteAndRead(ws1_bom, ws1_bom, policy, L"UTF-16 (BOM, LE, generate)");
    }
    {
        ioutils::text_io_policy_utf16 policy((locutils::codecvt_mode_utf16(endianess::byte_order::big_endian, codecvt_mode_base::headers::consume)));
        CheckFileWriteAndRead(ws1, ws1, policy, L"UTF-16 (NoBOM, BE, consume)");
        CheckFileWriteAndRead(ws1_bom, ws1, policy, L"UTF-16 (BOM, BE, consume)");
    }
    {
        ioutils::text_io_policy_utf16 policy((locutils::codecvt_mode_utf16(endianess::byte_order::big_endian, codecvt_mode_base::headers::generate)));
        CheckFileWriteAndRead(ws1, ws1_bom, policy, L"UTF-16 (NoBOM, BE, generate)");
        CheckFileWriteAndRead(ws1_bom, ws1_bom, policy, L"UTF-16 (BOM, BE, generate)");
    }
}


class TextReaderTest : public testing::Test
{
protected:
    void CheckReadBytes(const string& bytes, const wstring& expected, const ioutils::text_io_policy& policy, string title)
    {
        wstringstream wss;
        for (char c : bytes)
            wss << (wchar_t)((unsigned char)c);
        CheckRead(wss, expected, policy, title);
    };

//    void CheckReadBytes(const wstring& bytes, const wstring& expected, const ioutils::text_io_policy& policy, string title)
//    {
//        wstringstream wss;
//        wss << bytes;
//        CheckRead(wss, expected, policy, title);
//    };

    void CheckRead(wistream& wss, const wstring& expected, const ioutils::text_io_policy& policy, string title)
    {
        ioutils::text_reader r(wss, policy);
        CheckRead(r, expected, title);
    };

    void CheckRead(ioutils::text_reader& reader, const wstring& expected, string title)
    {
        wstring ws;
        reader.read_all(ws);
        EXPECT_EQ(ws, expected) << title;
    }
};

TEST_F(TextReaderTest, TestReadAll)
{
    string bytes = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    wstring expected = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    ioutils::text_io_policy_plain policy;
    policy.max_text_buf_size(expected.length() + 1);
    CheckReadBytes(bytes, expected, policy, "Full length");
    policy.max_text_buf_size(expected.length() / 2);
    CheckReadBytes(bytes, expected, policy, "Partial length");
}

TEST_F(TextReaderTest, TestRead_Ansi)
{
    CheckReadBytes("ABCDEF", L"ABCDEF", ioutils::text_io_policy_ansi(), "ASCII");
    CheckReadBytes("\xC0\xC7\xC8\xC9\xCF\xD9\xE0\xE7\xE8\xE9\xEF\xF9", // "ÀÇÈÉÏÙàçèéïù" ANSI cp1252
                   L"\u00C0\u00C7\u00C8\u00C9\u00CF\u00D9\u00E0\u00E7\u00E8\u00E9\u00EF\u00F9", // "ÀÇÈÉÏÙàçèéïù" UTF-16
                   ioutils::text_io_policy_ansi(locutils::ansi_encoding::cp1252),
                   "ANSI (1252)");
    CheckReadBytes("\xC0\xC1\xC2\xDE\xDF\xE0\xE1\xE2\xFE\xFF", // "АБВЮЯабвюя" ANSI cp1251
                   L"\u0410\u0411\u0412\u042E\u042F\u0430\u0431\u0432\u044E\u044F", // "АБВЮЯабвюя" UTF-16
                   ioutils::text_io_policy_ansi(locutils::ansi_encoding::cp1251),
                   "ANSI (1251)");
}

TEST_F(TextReaderTest, TestRead_Utf8)
{
    // Specific test because of character variable length (in bytes)
    wstring expected = locale_utils_test::string_01_utf16;
    {
        // Test file streams processed as ordinary istream
        // An imbue() converter is not used but reading should produce the same result
        ioutils::text_io_policy_utf8 policy((locutils::codecvt_mode_utf8(codecvt_mode_utf16::headers::consume)));
        {
            wifstream fs(L"text-reader-test-01-utf8-bom.txt", ios::binary);
            CheckRead(fs, expected, policy, "BOM file I/O");
        }
        {
            wifstream fs(L"text-reader-test-01-utf8.txt", ios::binary);
            CheckRead(fs, expected, policy, "NoBOM file I/O");
        }
        policy.max_text_buf_size(expected.length() / 2);
        {
            wifstream fs(L"text-reader-test-01-utf8-bom.txt", ios::binary);
            CheckRead(fs, expected, policy, "BOM file I/O small text buffer");
        }
        {
            wifstream fs(L"text-reader-test-01-utf8.txt", ios::binary);
            CheckRead(fs, expected, policy, "NoBOM file I/O small text buffer");
        }
    }
    {
        string bytes = locale_utils_test::string_01_utf8;
        string bytes_bom = bytes;
        utf8::add_bom(bytes_bom);
        {
            ioutils::text_io_policy_utf8 policy;
            CheckReadBytes(bytes, expected, policy, "1.1");
            CheckReadBytes(bytes_bom, expected, policy, "1.2");
            policy.max_text_buf_size(bytes.length() / 3);
            CheckReadBytes(bytes, expected, policy, "1.3");
            CheckReadBytes(bytes_bom, expected, policy, "1.4");
        }
        wstring expected_bom = expected;
        utf16::add_bom(expected_bom);
        {
            ioutils::text_io_policy_utf8 policy((locutils::codecvt_mode_utf8(codecvt_mode_utf16::headers::generate)));
            CheckReadBytes(bytes, expected_bom, policy, "2.1");
            CheckReadBytes(bytes_bom, expected_bom, policy, "2.2");
            policy.max_text_buf_size(bytes.length() / 3);
            CheckReadBytes(bytes, expected_bom, policy, "2.3");
            CheckReadBytes(bytes_bom, expected_bom, policy, "2.4");
        }
    }
}

TEST_F(TextReaderTest, TestRead_Utf16)
{
    wstring expected = locale_utils_test::string_01_utf16;
    wstring expected_bom = expected;
    utf16::add_bom(expected_bom);
    // Test file streams processed as ordinary istream
    // An imbue() converter is not used but reading should produce the same result
    {
        wifstream fs(L"text-reader-test-01-utf16-le.txt", ios::binary);
        CheckRead(fs, expected,
                  ioutils::text_io_policy_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::little_endian, codecvt_mode_utf16::headers::consume)),
                  "LE BOM file I/O");
    }
    {
        wifstream fs(L"text-reader-test-01-utf16-le.txt", ios::binary);
        ioutils::text_io_policy_utf16 policy(locutils::codecvt_mode_utf16(endianess::byte_order::little_endian, codecvt_mode_utf16::headers::consume));
        policy.max_text_buf_size(expected.length() / 2); // File I/O only
        CheckRead(fs, expected, policy, "LE BOM file I/O small text buffer");
    }
    {
        wifstream fs(L"text-reader-test-01-utf16-be.txt", ios::binary);
        CheckRead(fs, expected,
                  ioutils::text_io_policy_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::big_endian, codecvt_mode_utf16::headers::consume)),
                  "BE BOM file I/O");
    }
    // Like file wide-char stream, other wide-streams (i.e.wstringstream) should contains 1 significant byte per character
    // I.e. the character "A" (ASCII code 0x41) is presented as two wchar_t in wistream:
    // [0x41, 0x00] for LE or [0x00, 0x41] for BE
    {
        // default
        string bytes = utf16::wchar_to_multibyte(expected);
        string bytes_bom = bytes;
        utf16::add_bom(bytes_bom, endianess::platform_value());
        CheckReadBytes(bytes, expected, ioutils::text_io_policy_utf16(), "Default NoBOM consume");
        CheckReadBytes(bytes_bom, expected, ioutils::text_io_policy_utf16(), "Default BOM consume");
    }
    {
        string bytes_le = utf16::wchar_to_multibyte(expected);
        string bytes_le_bom = utf16::wchar_to_multibyte(expected_bom);
        if (endianess::platform_value() != endianess::byte_order::little_endian)
        {
            bytes_le = utf16::wchar_to_multibyte(utf16::swap_byte_order(expected));
            bytes_le_bom = utf16::wchar_to_multibyte(utf16::swap_byte_order(expected_bom));
        }
        ASSERT_EQ(bytes_le.length(), expected.length() * utf16::bytes_per_character);
        ASSERT_EQ(bytes_le_bom.length(), expected_bom.length() * utf16::bytes_per_character);
        CheckReadBytes(bytes_le, expected,
                       ioutils::text_io_policy_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::little_endian, codecvt_mode_utf16::headers::consume)),
                       "LE NoBOM consume");
        CheckReadBytes(bytes_le, expected_bom,
                            ioutils::text_io_policy_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::little_endian, codecvt_mode_utf16::headers::generate)),
                            "LE NoBOM generate");
        CheckReadBytes(bytes_le_bom, expected,
                            ioutils::text_io_policy_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::little_endian, codecvt_mode_utf16::headers::consume)),
                            "LE BOM consume");
        CheckReadBytes(bytes_le_bom, expected_bom,
                            ioutils::text_io_policy_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::little_endian, codecvt_mode_utf16::headers::generate)),
                            "LE BOM generate");
    }
    {
        string bytes_be = utf16::wchar_to_multibyte(expected);
        string bytes_be_bom = utf16::wchar_to_multibyte(expected_bom);
        if (endianess::platform_value() != endianess::byte_order::big_endian)
        {
            bytes_be = utf16::wchar_to_multibyte(utf16::swap_byte_order(expected));
            bytes_be_bom = utf16::wchar_to_multibyte(utf16::swap_byte_order(expected_bom));
        }
        ASSERT_EQ(bytes_be.length(), expected.length() * utf16::bytes_per_character);
        ASSERT_EQ(bytes_be_bom.length(), expected_bom.length() * utf16::bytes_per_character);
        CheckReadBytes(bytes_be, expected,
                            ioutils::text_io_policy_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::big_endian, codecvt_mode_utf16::headers::consume)),
                            "BE NoBOM consume");
        CheckReadBytes(bytes_be, expected_bom,
                            ioutils::text_io_policy_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::big_endian, codecvt_mode_utf16::headers::generate)),
                            "BE NoBOM generate");
        CheckReadBytes(bytes_be_bom, expected,
                            ioutils::text_io_policy_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::big_endian, codecvt_mode_utf16::headers::consume)),
                            "BE BOM consume");
        CheckReadBytes(bytes_be_bom, expected_bom,
                            ioutils::text_io_policy_utf16(locutils::codecvt_mode_utf16(endianess::byte_order::big_endian, codecvt_mode_utf16::headers::generate)),
                            "BE BOM generate");
    }
}

}
