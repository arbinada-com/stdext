#include "stdafx.h"
#include "CppUnitTest.h"
#include "datetime.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace stdext;

namespace stdexttest
{		
	TEST_CLASS(DatetimeTest)
	{
    private:
        void CheckTimeIsZero(datetime& dt, wstring title)
        {
            Assert::AreEqual(0, dt.hour(), title.c_str());
            Assert::AreEqual(0, dt.minute(), title.c_str());
            Assert::AreEqual(0, dt.second(), title.c_str());
        }

        void CheckDateFormat(
            datetime& dt,
            datetime::datepart_t year, datetime::datepart_t month, datetime::datepart_t day,
            datetime::datepart_t hour, datetime::datepart_t minute, datetime::datepart_t second,
            wstring title)
        {
            Assert::AreEqual(year, dt.year(), (title + L": year").c_str());
            Assert::AreEqual(month, dt.month(), (title + L": month").c_str());
            Assert::AreEqual(day, dt.day(), (title + L": day").c_str());
            Assert::AreEqual(hour, dt.hour(), (title + L": hour").c_str());
            Assert::AreEqual(minute, dt.minute(), (title + L": minute").c_str());
            Assert::AreEqual(second, dt.second(), (title + L": second").c_str());
        }
        void CheckDateFormat(
            datetime& dt,
            datetime::datepart_t year, datetime::datepart_t month, datetime::datepart_t day,
            wstring title)
        {
            CheckDateFormat(dt, year, month, day, 0, 0, 0, title);
        }

	public:
        TEST_METHOD(TestConstructors)
        {
            datetime d1;
            CheckDateFormat(d1, datetime::epoch_year, datetime::epoch_month, datetime::epoch_day, L"ctor 1.1");
            CheckTimeIsZero(d1, L"ctor 1.2");
            //
            CheckDateFormat(datetime("1999-02-03"), 1999, 02, 03, L"ctor 2.1");
            CheckDateFormat(datetime("1900-01-01"), 1900, 01, 01, L"ctor 2.2");
            CheckDateFormat(datetime("2000-10-11"), 2000, 10, 11, L"ctor 2.3");
            //
            CheckDateFormat(datetime("2019-10-11T01:02:03"), 2019, 10, 11, 1, 2, 3, L"ctor 3.1");
            CheckDateFormat(datetime("2019-10-11 11:22:33"), 2019, 10, 11, 11, 22, 33, L"ctor 3.2");
            //
            d1 = datetime::now();
            CheckDateFormat(datetime("01:02:03"), d1.year(), d1.month(), d1.day(), 1, 2, 3, L"ctor 4.1");
            CheckDateFormat(datetime("11:22:33"), d1.year(), d1.month(), d1.day(), 11, 22, 33, L"ctor 4.2");
            CheckDateFormat(datetime("22:33"), d1.year(), d1.month(), d1.day(), 22, 33, 0, L"ctor 4.3");
            //
            // Copy/move constructors
            datetime d21("2019-01-02T03:04:05");
            CheckDateFormat(d21, 2019, 1, 2, 3, 4, 5, L"ctor copy 1");
            datetime d22(d21);
            CheckDateFormat(d22, 2019, 1, 2, 3, 4, 5, L"ctor copy 2");
            datetime d23 = d21;
            CheckDateFormat(d23, 2019, 1, 2, 3, 4, 5, L"ctor copy 3");
            datetime d31(std::move(d21));
            CheckDateFormat(d31, 2019, 1, 2, 3, 4, 5, L"ctor move 1");
            datetime d32 = std::move(d22);
            CheckDateFormat(d32, 2019, 1, 2, 3, 4, 5, L"ctor move 2");
        }

        TEST_METHOD(TestQuarterOfMonth)
        {
            auto chkQuarter = [](datetime::datepart_t month, datetime::datepart_t quarter)
            {
                char s[] = "9999-99-99";
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