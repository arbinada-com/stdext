#include "stdafx.h"
#include "CppUnitTest.h"
#include "datetime.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace stdext;

namespace stdexttest
{		
	TEST_CLASS(DatetimeTest)
	{
	public:
		
        TEST_METHOD(TestQuarterOfMonth)
        {
            auto chkQuarter = [](datetime::datepart_t month, datetime::datepart_t quarter)
            {
                char s[] = "9999-99-99\0";
                sprintf_s(s, sizeof(s), "2019-%2d-01", month);
                datetime d(s);
                Assert::AreEqual(quarter, datetime::get_quarter_of_month(month), L"get_quarter_of_month");
            };
            chkQuarter(1, 1);
            chkQuarter(2, 1);
            chkQuarter(3, 1);
            chkQuarter(4, 2);
            chkQuarter(5, 2);
            chkQuarter(6, 2);
            chkQuarter(7, 3);
            chkQuarter(8, 3);
            chkQuarter(9, 3);
            chkQuarter(10, 4);
            chkQuarter(11, 4);
            chkQuarter(12, 4);
        }
        
        TEST_METHOD(TestDateParts)
		{
            datetime d1("2019-10-20 21:22:23");
            Assert::AreEqual(2019, d1.year(), L"year");
            Assert::AreEqual(10, d1.month(), L"month");
            Assert::AreEqual(4, d1.quarter(), L"quarter");
            Assert::AreEqual(20, d1.day(), L"day");
            Assert::AreEqual(7, d1.day_of_week(), L"day_of_week");
            Assert::AreEqual(21, d1.hour(), L"hour");
            Assert::AreEqual(22, d1.minute(), L"minute");
            Assert::AreEqual(23, d1.second(), L"second");
            datetime d2("1999-01-02 03:04:05");
            Assert::AreEqual(1999, d2.year(), L"year");
            Assert::AreEqual(1, d2.month(), L"month");
            Assert::AreEqual(1, d2.quarter(), L"quarter");
            Assert::AreEqual(2, d2.day(), L"day");
            Assert::AreEqual(6, d2.day_of_week(), L"day_of_week");
            Assert::AreEqual(3, d2.hour(), L"hour");
            Assert::AreEqual(4, d2.minute(), L"minute");
            Assert::AreEqual(5, d2.second(), L"second");
        }

	};
}