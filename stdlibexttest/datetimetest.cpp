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
            Assert::AreEqual((datetime::datepart_t)0, dt.hour(), title.c_str());
            Assert::AreEqual((datetime::datepart_t)0, dt.minute(), title.c_str());
            Assert::AreEqual((datetime::datepart_t)0, dt.second(), title.c_str());
            Assert::AreEqual((datetime::datepart_t)0, dt.millisecond(), title.c_str());
        }

        void CheckDateParts(
            datetime& dt,
            datetime::datepart_t year, datetime::datepart_t month, datetime::datepart_t day,
            datetime::datepart_t hour, datetime::datepart_t minute, datetime::datepart_t second, datetime::datepart_t msec,
            wstring title)
        {
            Assert::AreEqual(year, dt.year(), (title + L": year").c_str());
            Assert::AreEqual(month, dt.month(), (title + L": month").c_str());
            Assert::AreEqual(day, dt.day(), (title + L": day").c_str());
            Assert::AreEqual(hour, dt.hour(), (title + L": hour").c_str());
            Assert::AreEqual(minute, dt.minute(), (title + L": minute").c_str());
            Assert::AreEqual(second, dt.second(), (title + L": second").c_str());
        }
        void CheckDateParts(
            datetime& dt,
            datetime::datepart_t year, datetime::datepart_t month, datetime::datepart_t day,
            wstring title)
        {
            CheckDateParts(dt, year, month, day, 0, 0, 0, 0, title);
        }
        void CheckTimeParts(
            datetime& dt,
            datetime::datepart_t hour, datetime::datepart_t minute, datetime::datepart_t second, datetime::datepart_t msec,
            wstring title)
        {
            CheckDateParts(dt, 0, 0, 0, hour, minute, second, msec, title);
        }

        void CompareDatetimeData(const datetime::data_t& expected, const datetime::data_t& value, wstring title)
        {
            Assert::AreEqual(expected.year, value.year, (title + L": year").c_str());
            Assert::AreEqual(expected.month, value.month, (title + L": month").c_str());
            Assert::AreEqual(expected.day, value.day, (title + L": day").c_str());
            Assert::AreEqual(expected.hour, value.hour, (title + L": hour").c_str());
            Assert::AreEqual(expected.min, value.min, (title + L": minute").c_str());
            Assert::AreEqual(expected.sec, value.sec, (title + L": second").c_str());
            Assert::AreEqual(expected.msec, value.msec, (title + L": millisecond").c_str());
        }
	public:
        TEST_METHOD(TestTimePart)
        {
            datetime::data_t dt1(0, 0, 0, 18, 0, 0, 0);
            double hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.25 - hh) < 0.0001, L"HH 1.1");
            datetime::data_t dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 1.2");
            //
            dt1 = datetime::data_t(0, 0, 0, 6, 0, 0, 0);
            hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.75 - hh) < 0.0001, L"HH 2.1");
            dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 2.2");
            //
            dt1 = datetime::data_t(0, 0, 0, 18, 31, 27, 0);
            hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.27184023 - hh) < 1E-7, L"HH 3.1");
            dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 3.2");
            //
            dt1 = datetime::data_t(0, 0, 0, 10, 5, 43, 0);
            hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.92063657 - hh) < 1E-7, L"HH 4.1");
            dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 4.2");
            // Extreme time values
            dt1 = datetime::data_t(0, 0, 0, 0, 0, 0, 0);
            hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.5 - hh) < 0.01, L"HH 5.1");
            dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 5.2");
            dt1 = datetime::data_t(0, 0, 0, 23, 59, 59, 999);
            hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.5 - hh) < 0.01, L"HH 6.1");
            dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 6.2");
            // Noon
            dt1 = datetime::data_t(0, 0, 0, 12, 0, 0, 0);
            hh = datetime::hms_to_hh(dt1);
            Assert::IsTrue(abs(0.0 - hh) < 0.01, L"HH 7.1");
            dt2 = datetime::hh_to_hms(hh);
            CompareDatetimeData(dt1, dt2, L"HH 7.2");
        }

        TEST_METHOD(TestJulianDateCalcs)
        {
            // Epoch date 1 Jan 4713 BC 12:00 (julian) = 24 Nov 4714 BC 12:00 (gregorian)
            // Important note: for NNNN BC years the year parameter is specified as -(NNNN + 1)
            // I.e. 4713 BC => -4712, 1 BC => 0
            Assert::AreEqual((long)0, datetime::julian_to_jdn(-4712, 1, 1), L"Epoch day 1");
            datetime::jd_t jd1 = datetime::julian_to_jd(datetime::data_t(-4712, 1, 1, 12, 0, 0, 0));
            Assert::AreEqual(0.0, jd1, L"Epoch date 1");
            datetime::data_t d1 = datetime::jd_to_calendar(datetime::calendar::julian, 0.0);
            CompareDatetimeData(datetime::data_t(-4712, 1, 1), d1, L"Epoch julian date");
            Assert::AreEqual((long)0, datetime::gregorian_to_jdn(-4713, 11, 24), L"Epoch day 2");
            jd1 = datetime::gregorian_to_jd(datetime::data_t(-4713, 11, 24, 12, 0, 0, 0));
            Assert::AreEqual(0.0, jd1, L"Epoch date 2");
            d1 = datetime::jd_to_calendar(datetime::calendar::gregorian, 0.0);
            CompareDatetimeData(datetime::data_t(-4713, 11, 24), d1, L"Epoch gregorian date");
            // Other dates
            //
            d1 = datetime::data_t(1985, 2, 17, 6, 0, 0, 0);
            jd1 = datetime::gregorian_to_jd(d1);
            Assert::AreEqual(2446113.75, jd1, L"1985-02-17 at 06:00:00 - 1");
            datetime::data_t d2 = datetime::jd_to_calendar(datetime::calendar::gregorian, jd1);
            CompareDatetimeData(d1, d2, L"1985-02-17 at 06:00:00 - 2");
            Assert::AreEqual((long)2446113, datetime::gregorian_to_jdn(1985, 2, 17), L"1985-02-17 1");
            //
            Assert::AreEqual(1355866.5, datetime::gregorian_to_jd(datetime::data_t(-1000, 2, 29, 0, 0, 0, 0)), L"29 Feb 999 BC 1");
            jd1 = datetime::gregorian_to_jdn(-1000, 2, 29);
            Assert::AreEqual(1355866.5, jd1, L"29 Feb 999 BC 2");
            //
            Assert::AreEqual(2451545.0, datetime::gregorian_to_jd(datetime::data_t(2000, 1, 1, 12, 0, 0, 0)));
            jd1 = datetime::gregorian_to_jdn(2000, 1, 1);
            Assert::AreEqual(2451545.0, jd1);
            d1 = datetime::jd_to_calendar(datetime::calendar::gregorian, jd1);
            CompareDatetimeData(datetime::data_t(2000, 1, 1), d1, L"2000-01-01");
            //
            jd1 = datetime::gregorian_to_jd(datetime::data_t(2000, 1, 1, 18, 0, 0, 0));
            Assert::AreEqual(2451545.25, jd1);
            d1 = datetime::jd_to_calendar(datetime::calendar::gregorian, jd1);
            CompareDatetimeData(datetime::data_t(2000, 1, 1, 18, 0, 0, 0), d1, L"2000-01-01 18:00:00");
            //
            jd1 = datetime::gregorian_to_jd(datetime::data_t(1942, 8, 2, 15, 0, 0, 0));
            Assert::AreEqual(2430574.125, jd1);
            d1 = datetime::jd_to_calendar(datetime::calendar::gregorian, jd1);
            CompareDatetimeData(datetime::data_t(1942, 8, 2, 15, 0, 0, 0), d1, L"1942-08-02 15:00:00");
            
        }

        TEST_METHOD(TestConstructors)
        {
            datetime d1;
            CheckDateParts(d1, -4713, 1, 1, L"ctor 1.1");
            CheckTimeIsZero(d1, L"ctor 1.2");
            //
            CheckDateParts(datetime("1999-02-03"), 1999, 02, 03, L"ctor 2.1");
            CheckDateParts(datetime("1900-01-01"), 1900, 01, 01, L"ctor 2.2");
            CheckDateParts(datetime("2000-10-11"), 2000, 10, 11, L"ctor 2.3");
            //
            CheckDateParts(datetime("2019-10-11T01:02:03"), 2019, 10, 11, 1, 2, 3, 0, L"ctor 3.1");
            CheckDateParts(datetime("2019-10-11 11:22:33"), 2019, 10, 11, 11, 22, 33, 0, L"ctor 3.2");
            //
            d1 = datetime::now();
            CheckDateParts(datetime("01:02:03"), d1.year(), d1.month(), d1.day(), 1, 2, 3, 0, L"ctor 4.1");
            CheckDateParts(datetime("11:22:33"), d1.year(), d1.month(), d1.day(), 11, 22, 33, 0, L"ctor 4.2");
            CheckDateParts(datetime("22:33"), d1.year(), d1.month(), d1.day(), 22, 33, 0, 0, L"ctor 4.3");
            //
            // Copy/move constructors
            datetime d21("2019-01-02T03:04:05");
            CheckDateParts(d21, 2019, 1, 2, 3, 4, 5, 0, L"ctor copy 1");
            datetime d22(d21);
            CheckDateParts(d22, 2019, 1, 2, 3, 4, 5, 0, L"ctor copy 2");
            datetime d23 = d21;
            CheckDateParts(d23, 2019, 1, 2, 3, 4, 5, 0, L"ctor copy 3");
            datetime d31(std::move(d21));
            CheckDateParts(d31, 2019, 1, 2, 3, 4, 5, 0, L"ctor move 1");
            datetime d32 = std::move(d22);
            CheckDateParts(d32, 2019, 1, 2, 3, 4, 5, 0, L"ctor move 2");
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
            Assert::AreEqual((datetime::datepart_t)2019, d1.year(), L"year");
            Assert::AreEqual((datetime::datepart_t)10, d1.month(), L"month");
            Assert::AreEqual((datetime::datepart_t)4, d1.quarter(), L"quarter");
            Assert::AreEqual((datetime::datepart_t)20, d1.day(), L"day");
            Assert::AreEqual((datetime::datepart_t)7, d1.day_of_week(), L"day_of_week");
            Assert::AreEqual((datetime::datepart_t)21, d1.hour(), L"hour");
            Assert::AreEqual((datetime::datepart_t)22, d1.minute(), L"minute");
            Assert::AreEqual((datetime::datepart_t)23, d1.second(), L"second");
            datetime d2("1999-01-02 03:04:05");
            Assert::AreEqual((datetime::datepart_t)1999, d2.year(), L"year");
            Assert::AreEqual((datetime::datepart_t)1, d2.month(), L"month");
            Assert::AreEqual((datetime::datepart_t)1, d2.quarter(), L"quarter");
            Assert::AreEqual((datetime::datepart_t)2, d2.day(), L"day");
            Assert::AreEqual((datetime::datepart_t)6, d2.day_of_week(), L"day_of_week");
            Assert::AreEqual((datetime::datepart_t)3, d2.hour(), L"hour");
            Assert::AreEqual((datetime::datepart_t)4, d2.minute(), L"minute");
            Assert::AreEqual((datetime::datepart_t)5, d2.second(), L"second");
        }

	};
}