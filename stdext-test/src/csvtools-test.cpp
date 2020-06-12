﻿#include <gtest/gtest.h>
#include "platforms.h"
#include "csvtools.h"
#include "strutils.h"
#include <fstream>

using namespace std;
using namespace stdext;

namespace csvtools_test
{

class CsvToolsTest : public testing::Test
{
public:
    typedef std::vector<std::wstring> csv_row_values_t;
    typedef std::vector<csv_row_values_t> csv_values_t;
protected:
    wstringstream CreateTestStream(csv_values_t csv, const wchar_t separator, const wstring eol)
    {
        wstringstream ss;
        for (csv_row_values_t r : csv)
        {
            bool first = true;
            for (wstring s : r)
            {
                if (!first)
                    ss << separator;
                else
                    first = false;
                bool contains_dquote = s.find(L"\"") != wstring::npos;
                if (contains_dquote)
                    s = strutils::replace_all(s, L"\"", L"\"\"");
                if (contains_dquote || s.find(separator) != wstring::npos || s.find(eol) != wstring::npos)
                    s = strutils::double_quoted(s);
                ss << s;
            }
            ss << eol;
        }
        return ss;
    }

    csv_values_t CreateTestStreamValues(const wstring eol)
    {
        csv_values_t v;
        v.push_back(csv_row_values_t({ L"1", L"2", L"3" }));
        v.push_back(csv_row_values_t({ L"1.2", L"34.567", L"Text" }));
        v.push_back(csv_row_values_t({ L"-25", L"45,67", L"Some text" }));
        v.push_back(csv_row_values_t({ L"8.", L"11.1", L"Hello, CSV" }));
        v.push_back(csv_row_values_t({ L".6", L"6.8E01", L"Comment \"line 4\"" }));
        v.push_back(csv_row_values_t({ L".0", L"0.0E01", L"Multi line text" + eol + L"line 2" + eol + L"line 3" }));;
        return v;
    }

    void CheckStreamReading(wstring title, wistream& stream, const csv_values_t& expected, const wchar_t separator = L',')
    {
        csv::reader rd(stream);
        rd.separator(separator);
        CheckStreamReading(title, rd, expected);
    }
    void CheckStreamReading(wstring title, csv::reader& rd, const csv_values_t& expected)
    {
        title += L". ";
        long row = 0;
        csv::row r;
        ASSERT_TRUE(parsers::textpos() == rd.pos()) << title + L"Initial pos";
        while (rd.next_row(r))
        {
            if (!rd.eof())
                ASSERT_EQ(1, rd.pos().col()) << title + L"Col after read row";
            row++;
            ASSERT_EQ(row, rd.row_count()) << title + L"Row row";
            csv_row_values_t expected_row = expected[row - 1];
            ASSERT_EQ(expected_row.size(), r.field_count()) << title + strutils::wformat(L"Field row. Row: %d", row);
            for (size_t col = 0; col < r.field_count(); col++)
            {
                ASSERT_EQ(expected_row[col], r[col].value()) << title + strutils::wformat(L"Unexpected value. Row: %d, field: %d", row, col + 1);
            }
        }
        if (rd.has_error())
        {
            csv::reader::message_t* err = rd.errors()[0];
            FAIL() << title + strutils::wformat(L"Unexpected error occurred: %ls", err->to_wstring().c_str());
        }
        ASSERT_TRUE(row > 0) << title + L"No rows processed";
    }
};

TEST_F(CsvToolsTest, TestEmptyStream)
{
    wstringstream ss(L"");
    csv::reader rd(ss);
    csv::row r;
    EXPECT_FALSE(rd.next_row(r));
    EXPECT_FALSE(rd.has_error());
    EXPECT_EQ(0, rd.row_count()) << "Row row";
    EXPECT_EQ(0u, r.field_count()) << "Field row";
}

TEST_F(CsvToolsTest, TestStringStream)
{
    csv_values_t expected = CreateTestStreamValues(L"\r\n");
    wstringstream ss = CreateTestStream(expected, L',', L"\r\n");
    CheckStreamReading(L"String stream CRLF", ss, expected);
    ss = CreateTestStream(expected, L';', L"\r\n");
    CheckStreamReading(L"String stream CRLF separated ;", ss, expected, L';');
    expected = CreateTestStreamValues(L"\n");
    ss = CreateTestStream(expected, L',', L"\n");
    CheckStreamReading(L"String stream LF", ss, expected);
}

TEST_F(CsvToolsTest, TestErrorRowFieldCount)
{
    csv_values_t v;
    v.push_back(csv_row_values_t({ L"Col 1", L"Col 2" }));
    v.push_back(csv_row_values_t({ L"12", L"34", L"56" }));
    wstringstream ss = CreateTestStream(v, L',', L"\r\n");
    csv::reader rd(ss);
    rd.separator(',');
    ASSERT_TRUE(rd.read_header()) << L"Header";
    csv::row r;
    EXPECT_FALSE(rd.next_row(r)) << L"Read row";
    ASSERT_TRUE(rd.has_error()) << L"No error";
    csv::reader::message_t* err = rd.errors()[0];
    EXPECT_TRUE(err->kind() == csv::reader_msg_kind::row_field_count_different_from_header) << L"Wrong error";
    EXPECT_TRUE(parsers::textpos(2, 9) == err->pos()) << L"Error pos: " + err->to_wstring();
}

TEST_F(CsvToolsTest, TestErrorExpectedSeparator)
{
    {
        wstringstream ss;
        ss << L"\"Col 1\"x\n";
        csv::reader rd(ss);
        rd.separator(L',');
        csv::row r;
        EXPECT_FALSE(rd.next_row(r)) << L"Read row 1";
        ASSERT_TRUE(rd.has_error()) << L"No error 1";
        csv::reader::message_t* err = rd.errors()[0];
        EXPECT_TRUE(err->kind() == csv::reader_msg_kind::expected_separator) << L"Wrong error 1";
        EXPECT_TRUE(parsers::textpos(1, 8) == err->pos()) << L"Error pos 1 " + rd.pos().to_wstring();
    }
    {
        wstringstream ss;
        ss << L"\"Col\r\n123\"x";
        csv::reader rd(ss);
        rd.separator(L',');
        csv::row r;
        EXPECT_FALSE(rd.next_row(r)) << L"Read row 2";
        ASSERT_TRUE(rd.has_error()) << L"No error 2";
        csv::reader::message_t* err = rd.errors()[0];
        EXPECT_TRUE(err->kind() == csv::reader_msg_kind::expected_separator) << L"Wrong error 2";
        EXPECT_TRUE(parsers::textpos(2, 5) == err->pos()) << L"Error pos 2 " + rd.pos().to_wstring();
    }
}

TEST_F(CsvToolsTest, TestFiles)
{
    {
        csv_values_t expected; // ANSI 1251
        expected.push_back(csv_row_values_t({ L"1", L"2.345" }));
        expected.push_back(csv_row_values_t({ L"Non-ASCII текст", L"слово" }));
        expected.push_back(csv_row_values_t({ L"67,89", L"Multi line строка\nстрока 2\nстрока 3" }));
        csv::reader rd(L"test01-ansi-1251.csv", ioutils::text_io_options_ansi(locutils::ansi_encoding::cp1251));
        rd.separator(L',');
        CheckStreamReading(L"File ANSI 1251", rd, expected);
    }
    {
        csv_values_t expected; // ANSI 1252
        expected.push_back(csv_row_values_t({ L"1", L"2.345" }));
        expected.push_back(csv_row_values_t({ L"Non-ASCII déjà", L"élève" }));
        expected.push_back(csv_row_values_t({ L"67,89", L"Multi line à côté\ncôté 2\ncôté 3" }));
#if defined(__STDEXT_USE_ICONV)
        string encoding = "CP1252";
#else
        string encoding = ".1252";
#endif
        csv::reader rd(L"test01-ansi-1252.csv", ioutils::text_io_options_ansi(encoding.c_str()));
        rd.separator(L',');
        CheckStreamReading(L"File ANSI 1252", rd, expected);
    }
    csv_values_t expected; // Unicode
    expected.push_back(csv_row_values_t({ L"1", L"2.345" }));
    expected.push_back(csv_row_values_t({ L"Non-ASCII текст éèçà", L"ĀĂ" }));
    expected.push_back(csv_row_values_t({ L"67,89", L"Multi line текст\nстрока 2\ndéjà 3" }));
    {
        // UTF8 without BOM
        csv::reader rd(L"test01-utf8.csv", ioutils::text_io_options_utf8());
        rd.separator(L',');
        CheckStreamReading(L"File UTF8", rd, expected);
    }
    {
        // UTF8 with BOM
        csv::reader rd(L"test01-utf8-bom.csv", ioutils::text_io_options_utf8());
        rd.separator(L',');
        CheckStreamReading(L"File UTF8 BOM", rd, expected);
    }
    {
        // UTF16 Big Endian
        csv::reader rd(L"test01-utf16-be.csv", ioutils::text_io_options_utf16());
        rd.separator(L',');
        CheckStreamReading(L"File UTF16 BE", rd, expected);
    }
    {
        // UTF16 Little Endian
        csv::reader rd(L"test01-utf16-le.csv", ioutils::text_io_options_utf16());
        rd.separator(L',');
        CheckStreamReading(L"File UTF16 LE", rd, expected);
    }
}

}