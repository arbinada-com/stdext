#include "CppUnitTest.h"
#include "csvtools.h"
#include "strutils.h"
#include <fstream>
#include <codecvt>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;

TEST_CLASS(CsvToolsTest)
{
public:
    typedef typename std::vector<std::wstring> csv_row_values_t;
    typedef typename std::vector<csv_row_values_t> csv_values_t;
private:
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
        while (rd.next_row(r))
        {
            row++;
            Assert::AreEqual(row, rd.row_count(), (title + L"Row row").c_str());
            csv_row_values_t expected_row = expected[row - 1];
            Assert::AreEqual(expected_row.size(), r.field_count(), (title + strutils::format(L"Field row. Row: %d", row)).c_str());
            for (size_t col = 0; col < r.field_count(); col++)
            {
                Assert::AreEqual(expected_row[col], r[col].value(), (title + strutils::format(L"Unexpected value. Row: %d, field: %d", row, col + 1)).c_str());
            }
        }
        Assert::IsTrue(rd.error() == csv::reader_error::none,
            (title + strutils::format(L"Unexpected error occurred: %d", (int)rd.error())).c_str());
        Assert::IsTrue(row > 0, (title + L"No rows processed").c_str());
    }
public:
    TEST_METHOD(TestEmptyStream)
    {
        wstringstream ss(L"");
        csv::reader rd(ss);
        csv::row r;
        Assert::IsFalse(rd.next_row(r));
        Assert::IsFalse(rd.has_error());
        Assert::AreEqual((long)0, rd.row_count(), L"Row row");
        Assert::AreEqual((std::size_t)0, r.field_count(), L"Field row");
    }

    TEST_METHOD(TestStringStream)
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

    TEST_METHOD(TestErrorRowFieldCount)
    {
        csv_values_t v;
        v.push_back(csv_row_values_t({ L"Col 1", L"Col 2" }));
        v.push_back(csv_row_values_t({ L"12", L"34", L"56" }));
        wstringstream ss = CreateTestStream(v, L',', L"\r\n");
        csv::reader rd(ss);
        rd.separator(',');
        Assert::IsTrue(rd.read_header(), L"Header");
        csv::row r;
        Assert::IsFalse(rd.next_row(r), L"Read row");
        Assert::IsTrue(rd.has_error(), L"No error");
        Assert::IsTrue(rd.error() == csv::reader_error::rowFieldCountIsDifferentFromHeader, L"Wrong error");
    }

    TEST_METHOD(TestErrorExpectedSeparator)
    {
        wstringstream ss;
        ss << L"\"Col 1\"x" << endl;
        csv::reader rd(ss);
        rd.separator(L',');
        csv::row r;
        Assert::IsFalse(rd.next_row(r), L"Read row");
        Assert::IsTrue(rd.has_error(), L"No error");
        Assert::IsTrue(rd.error() == csv::reader_error::expectedSeparator, L"Wrong error");
    }

    TEST_METHOD(TestFiles)
    {
        {
            csv_values_t expected; // ANSI 1251
            expected.push_back(csv_row_values_t({ L"1", L"2.345" }));
            expected.push_back(csv_row_values_t({ L"Non-ASCII текст", L"слово" }));
            expected.push_back(csv_row_values_t({ L"67,89", L"Multi line строка\nстрока 2\nстрока 3" }));
            csv::reader rd(L"test01-ansi-1251.csv", csv::file_encoding::ansi, ".1251");
            rd.separator(L',');
            CheckStreamReading(L"File ANSI 1251", rd, expected);
        }
        {
            csv_values_t expected; // ANSI 1252
            expected.push_back(csv_row_values_t({ L"1", L"2.345" }));
            expected.push_back(csv_row_values_t({ L"Non-ASCII déjà", L"élève" }));
            expected.push_back(csv_row_values_t({ L"67,89", L"Multi line à côté\ncôté 2\ncôté 3" }));
            csv::reader rd(L"test01-ansi-1252.csv", csv::file_encoding::ansi, ".1252");
            rd.separator(L',');
            CheckStreamReading(L"File ANSI 1252", rd, expected);
        }
        csv_values_t expected; // Unicode
        expected.push_back(csv_row_values_t({ L"1", L"2.345" }));
        expected.push_back(csv_row_values_t({ L"Non-ASCII текст éèçà", L"ĀĂ" }));
        expected.push_back(csv_row_values_t({ L"67,89", L"Multi line текст\nстрока 2\ndéjà 3" }));
        {
            // UTF8 without BOM
            csv::reader rd(L"test01-utf8.csv", csv::file_encoding::utf8);
            rd.separator(L',');
            CheckStreamReading(L"File UTF8", rd, expected);
        }
        {
            // UTF8 with BOM
            csv::reader rd(L"test01-utf8-bom.csv", csv::file_encoding::utf8);
            rd.separator(L',');
            CheckStreamReading(L"File UTF8 BOM", rd, expected);
        }
        {
            // UTF16 Big Endian
            csv::reader rd(L"test01-utf16-be.csv", csv::file_encoding::utf16);
            rd.separator(L',');
            CheckStreamReading(L"File UTF16 BE", rd, expected);
        }
        {
            // UTF16 Little Endian
            csv::reader rd(L"test01-utf16-le.csv", csv::file_encoding::utf16);
            rd.separator(L',');
            CheckStreamReading(L"File UTF16 LE", rd, expected);
        }
    }
};