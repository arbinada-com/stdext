#include "stdafx.h"
#include "CppUnitTest.h"
#include "csvtools.h"
#include "strutils.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;

TEST_CLASS(CsvToolsTest)
{
public:
    typedef typename std::vector<std::string> csv_row_values_t;
    typedef typename std::vector<csv_row_values_t> csv_values_t;
    typedef typename std::vector<std::wstring> csv_row_wvalues_t;
    typedef typename std::vector<csv_row_wvalues_t> csv_wvalues_t;
private:
    stringstream CreateTestStream(const char separator, const string eol)
    {
        stringstream ss;
        csv_values_t csv = CreateTestStreamValues(eol);
        for (csv_row_values_t r : csv)
        {
            bool first = true;
            for (string s : r)
            {
                if (!first)
                    ss << separator;
                else
                    first = false;
                ss << s;
            }
            ss << eol;
        }
        return ss;
    }
    csv_values_t CreateTestStreamValues(const string eol)
    {
        csv_values_t v;
        v.push_back(csv_row_values_t({ "1", "12.345", "3.3", "\"55,55\"" }));
        v.push_back(csv_row_values_t({ "25", "45", "45.65", "Some text" }));
        v.push_back(csv_row_values_t({ "77", "11.1", "0.5", "\"Hello, CSV\"" }));
        v.push_back(csv_row_values_t({ "8.", ".6", "6.8E01", "\"Comment \"\"line 4\"\"\"" }));
        v.push_back(csv_row_values_t({ "0", ".0", "0.0E01", "\"Multi line text" + eol + "line 2" + eol + "line 3\"" }));;
        return v;
    }
public:
    TEST_METHOD(TestEmptyStream)
    {
        stringstream ss("");
        csv::reader rd(ss);
        csv::row r;
        Assert::IsFalse(rd.next_row(r));
        Assert::IsFalse(rd.has_error());
        Assert::AreEqual((long)0, rd.row_count(), L"Row row");
        Assert::AreEqual((std::size_t)0, r.field_count(), L"Field row");
    }

    TEST_METHOD(TestEmptyWStream)
    {
        wstringstream ss(L"");
        csv::wreader rd(ss);
        csv::wrow r;
        Assert::IsFalse(rd.next_row(r));
        Assert::IsFalse(rd.has_error());
        Assert::AreEqual((long)0, rd.row_count(), L"Row row");
        Assert::AreEqual((std::size_t)0, r.field_count(), L"Field row");
    }

    TEST_METHOD(TestStringStream)
    {
        csv_values_t expected = CreateTestStreamValues("\r\n");
        stringstream ss = CreateTestStream(',', "\r\n");
        csv::reader rd(ss);
        rd.separator(',');
        long row = 0;
        csv::row r;
        while (rd.next_row(r))
        {
            row++;
            Assert::AreEqual(row, rd.row_count(), L"Row row");
            csv_row_values_t expected_row = expected[row - 1];
            Assert::AreEqual(expected_row.size(), r.field_count(), strutils::format(L"Field row. Row: %d", row).c_str());
            for (size_t col = 0; col < r.field_count(); col++)
            {
                string expected_value = strutils::replace_all(expected_row[col], "\"\"", "\"");
                expected_value = strutils::replace(expected_value, "\"", "");
                expected_value = strutils::replace(expected_value, "\"", "", expected_value.length() - 1);
                Assert::AreEqual(expected_value, r[col].value(), strutils::format(L"Uexpected value. Row: %d, field: %d", row, col + 1).c_str());
            }
        }
        Assert::IsTrue(rd.error() == csv::reader_error::none, 
            strutils::format(L"Error occurred: %d", (int)rd.error()).c_str());
        Assert::IsTrue(row > 0, L"No rows processed");
    }
};